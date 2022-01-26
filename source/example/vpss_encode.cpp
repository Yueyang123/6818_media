/*
 * @Description: 程序实现对两路摄像头的分别编码保存
 * 运行方式 ./xxx 0 20  第一个参数选择摄像头，第二个参数设置帧率
 * @Autor: YURI
 * @Date: 2022-01-25 19:06:08
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-25 23:42:08
 */
#include "encode.h"
#include "encode_6818/encode_6818.h"
#include "ffmpeg_sws.h"
#include "opencv_mat.h"
#include "framebuffer.h"
#include "camera.h"
#include "camera_uvc/camera_uvc.h"
#include "camera_gc2053/camera_gc2053.h"
#include "camera_6124/camera_6124.h"
#ifdef S5P6818
    #define VIDEO_INDEX 6  
#else
    #define VIDEO_INDEX 0 
#endif


using namespace std;
using namespace cv;
typedef struct 
{
    int width;
    int height;
    unsigned char* rawframe;    //原始数据
    unsigned char* showframe;   //需要显示的数据
}vip_mutilchannel_app;

camera*               uvc_capture;            //UVC摄像节点
camera*               plat_capture; 
ffmpeg_sws_interface* ff_sws1;         //ffmpeg转换接口
ffmpeg_sws_interface* ff_sws2; 
vip_mutilchannel_app cap_uvc;
vip_mutilchannel_app cap_plat;
encode* enc1,*enc2;
FILE* file_fd_out1;
FILE* file_fd_out2;
int main(int argc,void **argv)
{
    unsigned char *out1,*out2,*seq;
	int length;
	int index=0;
	ENCODE_TYPE encode_type =AVC_ENC;
    int mode=atoi((const char*)argv[1]);
    int fps=atoi((const char*)argv[2]);
    printf("YOU CHOOSE MODE :%d \r\n",mode);

    if(mode==0||mode==2){
        cap_uvc.width=640;
        cap_uvc.height=480;
        unsigned char* uvc_yuvbuf422=(unsigned char*)malloc(cap_uvc.width*cap_uvc.height*2+1000);
        unsigned char* uvc_rgbbuf=(unsigned char*)malloc(cap_uvc.width*cap_uvc.height*3+1000);
        cap_uvc.rawframe=uvc_yuvbuf422;
        cap_uvc.showframe=uvc_rgbbuf;
        uvc_capture=new camera_uvc(cap_uvc.width,cap_uvc.height,V4L2_PIX_FMT_YUYV,VIDEO_INDEX);
        ff_sws1=new ffmpeg_sws_interface(cap_uvc.width,cap_uvc.height,AV_PIX_FMT_YUYV422,AV_PIX_FMT_NV12);
        uvc_capture->camera_open();
        file_fd_out1=fopen("./encode1.out","wr");
        enc1=new encode_6818(cap_uvc.width,cap_uvc.height,fps,encode_type,70,15);
        seq=enc1->encode_get_headinfo(&length);
        if(encode_type!=JPEG_ENC)fwrite(seq,length,1,file_fd_out1);
        // printf("WRITE HEAD \r\n");
        // fflush(stdout);
    }
    if(mode==1||mode==2){
        cap_plat.width=1920;
        cap_plat.height=1080;
        ff_sws2=new ffmpeg_sws_interface(cap_plat.width,cap_plat.height,AV_PIX_FMT_NV21,AV_PIX_FMT_NV12);
        unsigned char* plat_yuvbuf420=(unsigned char*)malloc(cap_plat.width*cap_plat.height*3/2+1000);
        unsigned char* plat_rgbbuf=(unsigned char*)malloc(cap_plat.width*cap_plat.height*3+1000);    
        cap_plat.rawframe=plat_yuvbuf420;
        cap_plat.showframe=plat_rgbbuf;
        
    #ifdef HI3516   
        plat_capture=new camera_gc2053(cap_plat.width,cap_plat.height,V4L2_PIX_FMT_NV21,VIDEO_INDEX);
    #endif
    #ifdef S5P6818   
        plat_capture=new camera_6124(cap_plat.width,cap_plat.height,V4L2_PIX_FMT_YUV420M,VIDEO_INDEX);
    #endif
        plat_capture->camera_open();
        file_fd_out2=fopen("./encode2.out","wr");
        enc2=new encode_6818(cap_plat.width,cap_plat.height,fps,encode_type,70,15);
        seq=enc2->encode_get_headinfo(&length);
        if(encode_type!=JPEG_ENC)fwrite(seq,length,1,file_fd_out2);
    }

   	while(1){  
        if(mode==1||mode==2){   
            enc2->encode_get_fps(0);
        }
        if(mode==0||mode==2){   
            enc1->encode_get_fps(0);
        }
    if(mode==0||mode==2){
        cap_uvc.rawframe=uvc_capture->read_frame();
        ff_sws1->ffmpeg_sws_convert(cap_uvc.rawframe,cap_uvc.showframe);
        out1=enc1->encode_enc_frame(cap_uvc.showframe,&length);
		if(encode_type==JPEG_ENC)fwrite(enc1->seq_head ,enc1->encode_seq_length,1,file_fd_out1);
		fwrite(out1,length,1,file_fd_out1);
    }
    if(mode==1||mode==2){
        cap_plat.rawframe=plat_capture->read_frame();
        ff_sws2->ffmpeg_sws_convert(cap_plat.rawframe,cap_plat.showframe);
        out2=enc2->encode_enc_frame(cap_plat.showframe,&length);
		if(encode_type==JPEG_ENC)fwrite(enc2->seq_head ,enc2->encode_seq_length,1,file_fd_out2);
		fwrite(out2,length,1,file_fd_out2);
    }        
		index++;
		printf("LENGTH : %d INDEX : %d \r\n",length,index);
        if(mode==0||mode==2){
        printf("FPS0: %lf \r\n" ,enc1->encode_get_fps(1)) ;
        enc1->encode_change_fps((int)enc1->fps);
        }
        if(mode==1||mode==2){
        printf("FPS1: %lf \r\n" ,enc2->encode_get_fps(1)) ;
        enc2->encode_change_fps((int)enc2->fps);
        }
	} 
    printf("END OK \r\n");
    return 0;
}