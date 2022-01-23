/*
 * @Description: 这是一个测试NVP6124摄像头 FFMPEG和opencv库的历程 需要输入一个文件名称 例如 xxx ./frame 
    运行结束后会在当前目录产生一个frame.yuv frame.bmp接口的文件
 * @Autor: YURI
 * @Date: 2022-01-20 00:28:05
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-21 01:04:13
 */
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "camera_6124.h" 
#include "ffmpeg_sws.h"
#include "opencv_mat.h"
using namespace std;
FILE* yuv_file_fd;
camera* capture;
ffmpeg_sws_interface* ffmpeg_sws;
opencv_mat* mat_convert;
int main(int argc ,void ** argv)
{
    int width=1920;
    int height=1080;
    unsigned char* yuvbuf420;
    unsigned char* rgbbuf=(unsigned char*)malloc(width*height*3+1000);
    if(argc!=2)return -1;
    capture=new camera(width,height);
    ffmpeg_sws=new ffmpeg_sws_interface(width,height,AV_PIX_FMT_YUV420P,AV_PIX_FMT_RGB24);
    mat_convert=new opencv_mat(width,height);
    char bmppath[20]={0};
    char yuvpath[20]={0};
    sprintf(bmppath,"%s.bmp",argv[1]);
    sprintf(yuvpath,"%s.yuv",argv[1]);
    yuv_file_fd=fopen((const char*)yuvpath,"wr");
    if(yuv_file_fd<0){
        printf("open file error \r\n");
        return -1;
    }
    yuvbuf420=capture->read_frame();
    ffmpeg_sws->ffmpeg_sws_convert(yuvbuf420,rgbbuf);
    Mat image= mat_convert->opencv_convert(rgbbuf);
    imwrite(bmppath,image);
    fwrite(yuvbuf420,width*height*3/2,1,yuv_file_fd);
    cout<<"END OK "<<endl;
    return 0;
}
