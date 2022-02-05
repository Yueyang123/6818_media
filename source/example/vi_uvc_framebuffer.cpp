/*
 * @Description: 这是一个测试framebuf 和UVC 摄像头的程序，通过opencv将UVC摄像头采集到的
 640x480的图像转化成1920x1080并进行显示
 * @Autor: YURI
 * @Date: 2022-01-20 00:28:05
 * @LastEditors: YURI
 * @LastEditTime: 2022-02-04 22:13:50
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
using namespace std;
FILE* yuv_file_fd;
camera* capture;
opencv_mat* mat_convert;
framebuffer* p;
ffmpeg_sws_interface* ff_sws;
//是否适应屏幕大小
#define FIX_SCREEN  1


int main(int argc ,void ** argv)
{
    int width=640;
    int height=480;
    unsigned char* yuvbuf422=(unsigned char*)malloc(width*height*2+1000);
    unsigned char* rgbbuf=(unsigned char*)malloc(width*height*3+1000);
    if(argc!=2)return -1;
    capture=new camera_uvc(width,height,V4L2_PIX_FMT_YUYV,atoi((char*)argv[1]));
    mat_convert=new opencv_mat(width,height);
    ff_sws=new ffmpeg_sws_interface(width,height,AV_PIX_FMT_YUYV422,AV_PIX_FMT_RGB24);

    p=new framebuffer("/dev/fb0");
    p->print_info();
    p->set_color(0xff0000);
    Mat image,show;
    capture->camera_open();
    while(1){
        capture->camera_get_fps(0);
        yuvbuf422=capture->read_frame();
        ff_sws->ffmpeg_sws_convert(yuvbuf422,rgbbuf);
        //Li_Arr_yuyv_bgr(yuvbuf422,rgbbuf,width,height);
        if(FIX_SCREEN){
            image= mat_convert->opencv_convert(rgbbuf);
            resize(image,show,Size(1920,1080));
            p->show_rgbbuffer(show.data ,0,0,1920,1080);
        }else{
            p->show_rgbbuffer(rgbbuf,0,0,640,480);
        }
        printf("FPS: %lf \r\n" ,capture->camera_get_fps(1) ) ;
    }
    cout<<"END OK "<<endl;
    return 0;
}
