/*
 * @Description: 这是一个测试 一路MIPI（或AHD）摄像头 和UVC 摄像头 同时显示的程序，
 通过opencv将UVC摄像头采集到的 640x480 图像并进行显示
 * @Autor: YURI
 * @Date: 2022-01-24 00:46:05
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-27 22:40:07
 */
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "ffmpeg_sws.h"
#include "opencv_mat.h"
#include "framebuffer.h"
#include <string.h>
#include "camera.h"
#include "camera_uvc/camera_uvc.h"

#ifdef S5P6818
    #include "camera_6124/camera_6124.h"
    #define VIDEO_INDEX 6  
#else
    #include "camera_gc2053/camera_gc2053.h"
    #define VIDEO_INDEX 0 
#endif
#define FB_PATH     "/dev/fb0"
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
opencv_mat*           mat_convert;    //opencv图像转换
ffmpeg_sws_interface* ff_sws1;         //ffmpeg转换接口
ffmpeg_sws_interface* ff_sws2; 
vip_mutilchannel_app cap_uvc;
vip_mutilchannel_app cap_plat;
framebuffer* p; //绘制图像

int main(int argc ,void ** argv)
{
    cap_uvc.width=640;
    cap_uvc.height=480;
    unsigned char* uvc_yuvbuf422=(unsigned char*)malloc(cap_uvc.width*cap_uvc.height*2+1000);
    unsigned char* uvc_rgbbuf=(unsigned char*)malloc(cap_uvc.width*cap_uvc.height*3+1000);
    cap_uvc.rawframe=uvc_yuvbuf422;
    cap_uvc.showframe=uvc_rgbbuf;
    uvc_capture=new camera_uvc(cap_uvc.width,cap_uvc.height,V4L2_PIX_FMT_YUYV,VIDEO_INDEX);
    ff_sws1=new ffmpeg_sws_interface(cap_uvc.width,cap_uvc.height,AV_PIX_FMT_YUYV422,AV_PIX_FMT_RGB24);
    
    cap_plat.width=1920;
    cap_plat.height=1080;
    mat_convert=new opencv_mat(cap_plat.width,cap_plat.height);
    ff_sws2=new ffmpeg_sws_interface(cap_plat.width,cap_plat.height,AV_PIX_FMT_NV21,AV_PIX_FMT_RGB24);
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

    p=new framebuffer(FB_PATH);
    p->print_info();
    
    Mat image,show;
    uvc_capture->camera_open();
    plat_capture->camera_open();
    while(1){
        uvc_capture->camera_get_fps(0);
        cap_uvc.rawframe=uvc_capture->read_frame();
        cap_plat.rawframe=plat_capture->read_frame();
        ff_sws1->ffmpeg_sws_convert(cap_uvc.rawframe,cap_uvc.showframe);
        ff_sws2->ffmpeg_sws_convert(cap_plat.rawframe,cap_plat.showframe);
        p->show_rgbbuffer(cap_uvc.showframe,0,0,640,480);
    
        image=mat_convert->opencv_convert(cap_plat.showframe);
        resize(image,show,Size(0,0),0.4,0.4);
        p->show_rgbbuffer(show.data,0,481,cap_plat.width*0.4,cap_plat.height*0.4);

        printf("FPS: %lf \r\n" ,uvc_capture->camera_get_fps(1) ) ;
    
    }
    cout<<"END OK "<<endl;
    return 0;
}
