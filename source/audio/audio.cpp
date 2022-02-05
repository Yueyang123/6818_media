#include "audio.h"

void audio::audio_write_frame(unsigned char* data)
{
	int ret;
	if(audio_type!=AUDIO_SPEAKER){
		fprintf(stderr,"IT'S NOT A SPEAKER \r\n");
		return;
	}
	buffer_out=data;
	ret = snd_pcm_writei(capture_handle, buffer_out, frame_size);

    if (ret == -EPIPE) {
      /* EPIPE means underrun */
      fprintf(stderr, "underrun occurred\n");
      snd_pcm_prepare(capture_handle);
    } else if (ret < 0) {
      fprintf(stderr,"error from writei: %s\n",snd_strerror(ret));
    }

}

void audio::audio_close_speaker(void)
{
	if(audio_type!=AUDIO_SPEAKER){
		fprintf(stderr,"IT'S NOT A SPEAKER \r\n");
		return;
	}
	snd_pcm_drain(capture_handle);
  	snd_pcm_close(capture_handle);	
}

void audio::audio_open_speaker(void)
{
	if(audio_type!=AUDIO_SPEAKER){
		fprintf(stderr,"IT'S NOT A SPEAKER \r\n");
		return;
	}
	int ret,dir;
	/* Open PCM device for playback. */
	ret = snd_pcm_open(&capture_handle, audio_path.c_str(),SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		fprintf(stderr,"unable to open pcm device: %s\n",snd_strerror(ret));
		exit(1);
	}
	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&hw_params);
	/* Fill it in with default values. */
	snd_pcm_hw_params_any(capture_handle, hw_params);
	/* Set the desired hardware parameters. */
	/* Interleaved mode */
	snd_pcm_hw_params_set_access(capture_handle, hw_params,SND_PCM_ACCESS_RW_INTERLEAVED);
	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(capture_handle, hw_params,format);
	/* Two channels (stereo) */
	snd_pcm_hw_params_set_channels(capture_handle, hw_params, channel);
	snd_pcm_hw_params_set_rate_near(capture_handle, hw_params,&sample_rate, &dir);
	/* Set period size to 32 frames. */
	ret = snd_pcm_hw_params(capture_handle, hw_params);
	if (ret < 0) {
		fprintf(stderr,"unable to set hw parameters: %s\n",snd_strerror(ret));
		exit(1);
	}	
}

/**
 * @description: 关闭麦克风
 * @param {*}
 * @return {*}
 * @author: YURI
 */
void audio::audio_close_micophone(void){
	if(audio_type!=AUDIO_MICPHONE){
		fprintf(stderr,"IT'S NOT A MICPHONE \r\n");
		return;
	}
	// 释放数据缓冲区
	free(buffer_in);
	fprintf(stdout, "buffer_in freed\n");
	// 关闭音频采集卡硬件
	snd_pcm_close (capture_handle);
	fprintf(stdout, "audio interface closed\n");
}
/**
 * @description: 打开麦克风 并设置参数
 * @param {*}
 * @return {*}
 * @author: YURI
 */
void audio::audio_open_micophone(void){
	if(audio_type!=AUDIO_MICPHONE){
		fprintf(stderr,"IT'S NOT A MICPHONE \r\n");
		return;
	}
	int err;
	// 打开音频采集卡硬件，并判断硬件是否打开成功，若打开失败则打印出错误提示
	if ((err = snd_pcm_open (&capture_handle, audio_path.c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0) 
	{
		fprintf (stderr, "cannot open audio device %s (%s)\n",  audio_path.c_str(), snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "audio interface opened\n");

	// 分配一个硬件变量对象，并判断是否分配成功
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) 
	{
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params allocated\n");
	
	// 按照默认设置对硬件对象进行设置，并判断是否设置成功
	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) 
	{
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params initialized\n");
	/*
		设置数据为交叉模式，并判断是否设置成功
		interleaved/non interleaved:交叉/非交叉模式。
		表示在多声道数据传输的过程中是采样交叉的模式还是非交叉的模式。
		对多声道数据，如果采样交叉模式，使用一块buffer_in即可，其中各声道的数据交叉传输；
		如果使用非交叉模式，需要为各声道分别分配一个buffer_in，各声道数据分别传输。
	*/
	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
	{
		fprintf (stderr, "cannot set access type (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params access setted\n");

	// 设置数据编码格式为PCM、有符号、16bit、LE格式，并判断是否设置成功
	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) 
	{
		fprintf (stderr, "cannot set sample format (%s)\n",  snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params format setted\n");

	// 设置采样频率，并判断是否设置成功
	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &sample_rate, 0)) < 0) 
	{
		fprintf (stderr, "cannot set sample rate (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params rate setted\n");

	//  设置为双声道，并判断是否设置成功
	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) 
	{
		fprintf (stderr, "cannot set channel count (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params channels setted\n");
	// 将配置写入驱动程序中，并判断是否配置成功
	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) 
	{
		fprintf (stderr, "cannot set parameters (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params setted\n");
	// 使采集卡处于空闲状态
	snd_pcm_hw_params_free (hw_params);
	fprintf(stdout, "hw_params freed\n");
	// 准备音频接口，并判断是否准备好
	if ((err = snd_pcm_prepare (capture_handle)) < 0) 
	{
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "audio interface prepared\n");
	// 配置一个数据缓冲区用来缓冲数据
	buffer_in = (unsigned char*)malloc(frame_size*snd_pcm_format_width(format) / 8 * channel);
	fprintf(stdout, "buffer_in allocated\n");
}
/**
 * @description: 读一帧数据出来
 * @param {*}
 * @return {*}
 * @author: YURI
 */
unsigned char *audio::audio_read_frame(void)
{
	if(audio_type!=AUDIO_MICPHONE){
		fprintf(stderr,"IT'S NOT A MICPHONE \r\n");
		return NULL;
	}
	int err;
	// 读取
	if ((err = snd_pcm_readi (capture_handle, buffer_in, frame_size)) != frame_size) 
	{
		fprintf (stderr, "read from audio interface failed (%s)\n", err, snd_strerror (err));
		exit (1);
	}
	return buffer_in;
}
/**
 * @description: 分离声道数据
 * @param {unsigned char **} left
 * @return {*}
 * @author: YURI
 */
void audio::audio_channel_split(unsigned char ** data)
{
	if(audio_type!=AUDIO_MICPHONE){
		fprintf(stderr,"IT'S NOT A MICPHONE \r\n");
		return;
	}
    for(int i=0;i<channel;i++){
        for(int j=0;j<frame_size;j++){
            data[i][j*2]=buffer_in[j*4+2*i];
            data[i][j*2+1]=buffer_in[j*4+2*i+1];
        }
    }
}
/**
 * @description: audio 麦克风 初始化
 * @param {string} audio_tag
 * @param {int} channel
 * @param {int} sample_rate
 * @param {snd_pcm_format_t} pcm_format
 * @return {*}
 * @author: YURI
 */
audio::audio(AUDIO_TYPE type,string audio_tag,int channel,int sample_rate,snd_pcm_format_t pcm_format,int frame_size)
{
	this->audio_type=type;
    //设置采集参数
    this->audio_path=audio_tag;
    this->channel=channel;
    this->sample_rate=sample_rate;
    this->format=pcm_format;
    this->frame_size=frame_size;
}
/**
 * @description: 释放空间关闭硬件
 * @param {*}
 * @return {*}
 * @author: YURI
 */
audio::~audio()
{
	if(audio_type=AUDIO_MICPHONE){
		// 释放数据缓冲区
		free(buffer_in);
		fprintf(stdout, "buffer_in freed\n");
	}
	// 关闭音频采集卡硬件
}

