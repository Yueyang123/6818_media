/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-27 17:50:45
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-27 17:50:46
 */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/types.h>

#define ALSA_PCM_NEW_HW_PARAMS_API
#define SAMPLE_RATE         			(48000)
#define FRAMES_INIT         			1024
#define CHANNEL 	 	  			(2)
#define FRAMES_SIZE  	  			((16/8) *CHANNEL)
#define PCM_STREAM_CAPTURE_DEVICE	"hw:0,0"
//#define PCM_STREAM_CAPTURE_DEVICE	"default"

typedef struct{
    int dir;
    int size;
    unsigned int val;
    snd_pcm_t *handle;
    snd_pcm_uframes_t frames;
    snd_pcm_hw_params_t *params;
}rec_config_t;

static rec_config_t *s_index = NULL;
	

typedef struct _pcm_header_t {
    char    pcm_header[4];
    uint32_t  pcm_length;
    char    format[8];
    int     bit_rate;
    short   pcm;
    short   channel;
    int     sample_rate;
    int     byte_rate;
    short   block_align;
    short   bits_per_sample;
    char    fix_data[4];
    uint32_t  data_length;
} pcm_header_t;

static pcm_header_t s_pcm_header = {
    {'R', 'I', 'F', 'F'},
    ( uint32_t)-1,
    {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '},
    0x10,
    0x01,
    0x01,
    SAMPLE_RATE,
    SAMPLE_RATE*2,
    0x02,
    0x10,
    {'d', 'a', 't', 'a'},
    (uint32_t)-1
};


typedef struct _REC_FILE{
	FILE    *_file;
	pcm_header_t _hdr;
}REC_FILE;

REC_FILE  *s_rec_files[CHANNEL+1];

REC_FILE * duer_store_voice_start(int channel_id)
{
	REC_FILE *file=NULL;
	 
       printf("start");
       file = (REC_FILE*)malloc(sizeof(REC_FILE));
       if(file==NULL){
	      return NULL;	
	}
	
    char _name[64];
    snprintf(_name, sizeof(_name), "./channel-%d.wav", channel_id);
    file->_file = fopen(_name, "wb");
    if (!file->_file ) {
        printf("can't open file %s", _name);
        return  NULL;
    } else {
        printf("begin write to file:%s", _name);
    }
    
    memcpy(&file->_hdr,&s_pcm_header,sizeof(s_pcm_header));
    fwrite(&s_pcm_header, 1, sizeof(s_pcm_header), file->_file);
    file->_hdr.data_length = 0;
    file->_hdr.pcm_length = sizeof(s_pcm_header) - 8;

    return  file;
}

int duer_store_voice_write(REC_FILE *file,const void *data, uint32_t size)
{
    if (file&&file->_file) {
        fwrite(data, 1, size, file->_file);
        file->_hdr.data_length += size;
    }
    
    return 0;
}

int duer_store_voice_end(REC_FILE *file)
{
    if (file&&file->_file) {
        file->_hdr.pcm_length += s_pcm_header.data_length;
        fseek(file->_file, 0, SEEK_SET);
        fwrite(&file->_hdr, 1, sizeof(file->_hdr), file->_file);
        fclose(file->_file);
        file->_file = NULL;
    }
    return 0;
}

int read_pcm_mono_data(int16_t *in,int ilen,int16_t *out,int channel_cnt)
{
	int i=0;
	if((ilen%channel_cnt)){
			   printf("invalid pcm data lenght!\n");
		       return -1;	
	 }
	 		
	 for(i=0;i<ilen/channel_cnt;i++){
			uint16_t mono_data = 0;
			int j=0;
			for(j=0;j<channel_cnt;j++){
				  out[i] += in[channel_cnt*i+j];
			}
			out[i] /= channel_cnt;
	}
    	
	  return ilen/channel_cnt;
}


int read_pcm_channel_data(int16_t *in,int ilen,int16_t *out,int channel_id,int channel_cnt)
{
        int i=0;
        
	    if((ilen%channel_cnt)){
		     printf("invalid pcm data lenght!\n");
		     return -1;	
		}
		
		if(channel_id<0||channel_id>=channel_cnt){
		        printf("please input invalid channel id \n");
		        return -1;	
		}
		
		for(i=0;i<ilen/channel_cnt;i++){
			out[i] = in[i*channel_cnt+channel_id];
		}
		
		return  ilen/channel_cnt;
}

static void   recording_pcm_data()
{
    int16_t *buffer = NULL;
    int16_t *mono_buffer = NULL;
    int mono_data_size = 0;
		
    snd_pcm_hw_params_get_period_size(s_index->params, &(s_index->frames), &(s_index->dir));
    if (s_index->frames < 0) {
        printf("Get period size failed!");
        return;
    }
    
    printf("frames %d dir %d\n",s_index->frames,s_index->dir);
    s_index->size = s_index->frames * FRAMES_SIZE;

    if (buffer) {
        free(buffer);
        buffer = NULL;
    }
	
    buffer = (int16_t *)malloc(s_index->size);
    if (!buffer) {
        printf("malloc buffer failed!\n");
    } else {
        memset(buffer, 0, s_index->size);
    }

    mono_buffer = (int16_t *)malloc(s_index->size);
    if (!mono_buffer) {
        printf("malloc buffer failed!\n");
    } else {
        memset(mono_buffer, 0, s_index->size);
    }
	
    while (1)
    {
	 int i=0;
        int ret = snd_pcm_readi(s_index->handle, buffer, s_index->frames);
         
        if (ret == -EPIPE) {
            printf("an overrun occurred!\n");
            snd_pcm_prepare(s_index->handle);
	        continue;
        } else if (ret < 0) {
            printf("read: %s\n", snd_strerror(ret));
	    continue;
        } else if (ret != (int)s_index->frames) {
            printf("read %d frames!\n", ret);
	    continue;
        } else {
	       printf("ret=%d %d\n",ret,s_index->size);
        }
	
	for(i=0;i<CHANNEL;i++){
	      mono_data_size = read_pcm_channel_data(buffer,s_index->size>>1,mono_buffer,i,CHANNEL);
		    duer_store_voice_write(s_rec_files[i],mono_buffer,mono_data_size<<1);
	}
    }
    
    if (buffer) {
        free(buffer);
        buffer = NULL;
    }
    
    if(mono_buffer){
         free(mono_buffer);
	     mono_buffer=NULL;	
    }
	
    snd_pcm_drain(s_index->handle);
    snd_pcm_close(s_index->handle);
	
    if(s_index) {
        free(s_index);
        s_index = NULL;
    }
    	
	return;
}

static int duer_open_alsa_pcm()
{
    int ret = 0;
    
     s_index = (rec_config_t *)malloc(sizeof(rec_config_t));
    if (!s_index) {
	printf("malloc fail\n");
        return -1;
    }
    
    memset(s_index, 0, sizeof(rec_config_t));
    s_index->frames = FRAMES_INIT;
    s_index->val = SAMPLE_RATE; // pcm sample rate
    
    int result = (snd_pcm_open(&(s_index->handle), PCM_STREAM_CAPTURE_DEVICE, SND_PCM_STREAM_CAPTURE, 0));
    if (result < 0){
        printf("\n\n****unable to open pcm device: %s*********\n\n", snd_strerror(ret));
        ret = -1;
    }
    
    return ret;
}

static int duer_set_pcm_params()
{
    int ret = 0;
    
    snd_pcm_hw_params_alloca(&(s_index->params));
    snd_pcm_hw_params_any(s_index->handle, s_index->params);
    snd_pcm_hw_params_set_access(s_index->handle, s_index->params,
                                 SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(s_index->handle, s_index->params,
                                 SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(s_index->handle, s_index->params,
                                   CHANNEL);
    snd_pcm_hw_params_set_rate_near(s_index->handle, s_index->params,
                                    &(s_index->val), &(s_index->dir));
    snd_pcm_hw_params_set_period_size_near(s_index->handle, s_index->params,
                                           &(s_index->frames), &(s_index->dir));

    int result = snd_pcm_hw_params(s_index->handle, s_index->params);
    if (result < 0)    {
        printf("unable to set hw parameters: %s\n", snd_strerror(result));
        ret = -1;
    }
    
    return ret;
}

void record_stop(int signo) 
{
     int i=0;
     
     printf("oops! stop!!!\n");
     for(i=0;i<CHANNEL+1;i++){
	     if(s_rec_files[i] != NULL){
		       duer_store_voice_end(s_rec_files[i]);
		 } 
	}
	
     _exit(0);
}

int main(int argc, char *argv[])
{
        int ret=0;
	int i;
        
	for(i=0;i<CHANNEL;i++){
	     s_rec_files[i] = NULL;  
	      s_rec_files[i] =  duer_store_voice_start(i);
	}
	
	signal(SIGINT, record_stop); 	
	printf("%s %d\n",__FUNCTION__,__LINE__);    
	ret = duer_open_alsa_pcm();
        if (ret != 0) {
	        printf("open pcm failed\n");
	        return -1;
	}
	
	printf("%s %d\n",__FUNCTION__,__LINE__);    	
        ret = duer_set_pcm_params();
        if (ret != 0) {
	        printf("duer_set_pcm_params failed\n");
		return -1;
	 }
	 
	 printf("%s %d\n",__FUNCTION__,__LINE__); 
        recording_pcm_data();
	    
       return 0;
}


// #include <alsa/asoundlib.h>

// snd_pcm_t *handle;
// snd_pcm_hw_params_t *params;
// snd_pcm_uframes_t frames = 1024;//一个周期多少帧
// snd_pcm_access_t access_mode = SND_PCM_ACCESS_RW_INTERLEAVED;//访问模式:交错访问
// snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;//采样位数:16位,小端存储
// int channel = 2;
// unsigned int simple_rate = 48000;//采样率
// //int dir;//设备采样率与输入采样的偏差

// char *buffer = NULL;//缓冲区数据
// int size;//缓冲区大小
// FILE *file_fd;
// int main()
// {
//     file_fd=fopen("./test.wav","r");
//     fseek(file_fd,44,SEEK_SET);

//     /* 1. 打开pcm设备 */
//     int rc = snd_pcm_open(&handle, "hw:0,0",SND_PCM_STREAM_PLAYBACK, 0);
//     if (rc < 0) {
//         printf("open device failed\n");
//         return 0;
//     }

//     /* 分配一个硬件参数对象 */
//     snd_pcm_hw_params_alloca(&params);
//     snd_pcm_hw_params_any(handle, params);/* 使用默认值填充参数对象. */
//     /* 设置硬件参数 */
//     snd_pcm_hw_params_set_access(handle, params,access_mode);/* 交错模式 Interleaved mode */
//     snd_pcm_hw_params_set_format(handle, params,format);/* 采样位数 Signed 16-bit little-endian format */
//     snd_pcm_hw_params_set_channels(handle, params, channel);/* 通道数 Two channels (stereo) */
//     snd_pcm_hw_params_set_rate_near(handle, params,&simple_rate, NULL);/* 采样率 44100 bits/second sampling rate (CD quality) */
//     snd_pcm_hw_params_set_period_size_near(handle,params, &frames, NULL);//设置一个周期的多少帧
//     printf("FRAME SIZE %d \r\n",frames);
//     size= 4*(frames) ;
//     buffer=(char*)malloc(size);

//     rc = snd_pcm_hw_params(handle, params);/* 将设置好的参数写入驱动 */
//     if (rc < 0) {
//         printf("unable to set hw parameters: %s\n",snd_strerror(rc));
//         return 0;
//     }
//     int read=0;
//     do{
//         read= fread(buffer,size,1,file_fd);
//         rc = snd_pcm_writei(handle, buffer, frames);
//         if( rc>0 && rc<frames ){
//             printf("short write\n");
//         }else if(rc == -EPIPE){
//             printf("underrun occurred\n");
//             snd_pcm_prepare(handle);
//         }else if (rc < 0){
//             printf("error from writei: %s\n",snd_strerror(rc));
//             break;
//         }
//     }while(read>0);
    
//     snd_pcm_drain(handle);
//     snd_pcm_close(handle);
// }
