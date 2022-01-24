/*
 * @Description: 这是一个测试 一路MIPI（或AHD）摄像头 和UVC 摄像头 同时显示的程序，
 通过opencv将UVC摄像头采集到的 640x480 图像并进行显示
 * @Autor: YURI
 * @Date: 2022-01-24 00:46:05
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-24 03:16:28
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
#include "camera_gc2053/camera_gc2053.h"

#define VIDEO_PATH "/dev/video0"

using namespace std;

typedef struct vip_mutilchannel_app
{
    int width;
    int height;
    unsigned char* rawframe;    //原始数据
    unsigned char* showframe;   //需要显示的数据
    camera* capture;            //UVC摄像节点
    opencv_mat* mat_convert;    //opencv图像转换
    ffmpeg_sws* ff_sws;         //ffmpeg转换接口
};

vip_mutilchannel_app cap_uvc;
vip_mutilchannel_app cap_plat;
framebuffer* p; //绘制图像

static short int BUMap[3][256]={
{0,1,2,3,4,5,6,8,9,10,11,12,13,15,16,17,18,19,20,22,23,24,25,26,27,29,30,31,32,33,34,36,37,38,39,40,41,43,44,45,46,47,48,50,51,52,53,54,55,57,58,59,60,61,62,64,65,66,67,68,69,71,72,73,74,75,76,77,79,80,81,82,83,84,86,87,88,89,90,91,93,94,95,96,97,98,100,101,102,103,104,105,107,108,109,110,111,112,114,115,116,117,118,119,121,122,123,124,125,126,128,129,130,131,132,133,135,136,137,138,139,140,142,143,144,145,146,147,149,150,151,152,153,154,155,157,158,159,160,161,162,164,165,166,167,168,169,171,172,173,174,175,176,178,179,180,181,182,183,185,186,187,188,189,190,192,193,194,195,196,197,199,200,201,202,203,204,206,207,208,209,210,211,213,214,215,216,217,218,220,221,222,223,224,225,226,228,229,230,231,232,233,235,236,237,238,239,240,242,243,244,245,246,247,249,250,251,252,253,254,256,257,258,259,260,261,263,264,265,266,267,268,270,271,272,273,274,275,277,278,279,280,281,282,284,285,286,287,288,289,291,292,293,294,295,296},
{0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,113,115,117,119,121,123,125,127,129,131,133,135,137,139,141,143,145,147,149,151,153,155,157,159,161,163,165,167,169,171,173,175,177,179,181,183,185,187,189,191,193,195,197,199,201,203,205,207,209,211,213,215,217,219,221,224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,254,256,258,260,262,264,266,268,270,272,274,276,278,280,282,284,286,288,290,292,294,296,298,300,302,304,306,308,310,312,314,316,318,320,322,324,326,328,330,332,334,337,339,341,343,345,347,349,351,353,355,357,359,361,363,365,367,369,371,373,375,377,379,381,383,385,387,389,391,393,395,397,399,401,403,405,407,409,411,413,415,417,419,421,423,425,427,429,431,433,435,437,439,441,443,445,448,450,452,454,456,458,460,462,464,466,468,470,472,474,476,478,480,482,484,486,488,490,492,494,496,498,500,502,504,506,508,510,512,514}
};
static short int GUMap[3][256]={
{0,1,2,3,4,5,6,8,9,10,11,12,13,15,16,17,18,19,20,22,23,24,25,26,27,29,30,31,32,33,34,36,37,38,39,40,41,43,44,45,46,47,48,50,51,52,53,54,55,57,58,59,60,61,62,64,65,66,67,68,69,71,72,73,74,75,76,77,79,80,81,82,83,84,86,87,88,89,90,91,93,94,95,96,97,98,100,101,102,103,104,105,107,108,109,110,111,112,114,115,116,117,118,119,121,122,123,124,125,126,128,129,130,131,132,133,135,136,137,138,139,140,142,143,144,145,146,147,149,150,151,152,153,154,155,157,158,159,160,161,162,164,165,166,167,168,169,171,172,173,174,175,176,178,179,180,181,182,183,185,186,187,188,189,190,192,193,194,195,196,197,199,200,201,202,203,204,206,207,208,209,210,211,213,214,215,216,217,218,220,221,222,223,224,225,226,228,229,230,231,232,233,235,236,237,238,239,240,242,243,244,245,246,247,249,250,251,252,253,254,256,257,258,259,260,261,263,264,265,266,267,268,270,271,272,273,274,275,277,278,279,280,281,282,284,285,286,287,288,289,291,292,293,294,295,296},
{0,0,0,1,1,1,2,2,3,3,3,4,4,4,5,5,6,6,6,7,7,7,8,8,9,9,9,10,10,11,11,11,12,12,12,13,13,14,14,14,15,15,15,16,16,17,17,17,18,18,18,19,19,20,20,20,21,21,22,22,22,23,23,23,24,24,25,25,25,26,26,26,27,27,28,28,28,29,29,30,30,30,31,31,31,32,32,33,33,33,34,34,34,35,35,36,36,36,37,37,37,38,38,39,39,39,40,40,41,41,41,42,42,42,43,43,44,44,44,45,45,45,46,46,47,47,47,48,48,49,49,49,50,50,50,51,51,52,52,52,53,53,53,54,54,55,55,55,56,56,56,57,57,58,58,58,59,59,60,60,60,61,61,61,62,62,63,63,63,64,64,64,65,65,66,66,66,67,67,67,68,68,69,69,69,70,70,71,71,71,72,72,72,73,73,74,74,74,75,75,75,76,76,77,77,77,78,78,79,79,79,80,80,80,81,81,82,82,82,83,83,83,84,84,85,85,85,86,86,86,87,87,88,88,88,89,89,90,90,90,91,91,91,92,92,93,93,93,94,94,94,95,95,96,96,96},
{0,0,1,2,3,4,4,5,6,7,8,8,9,10,11,12,13,13,14,15,16,17,17,18,19,20,21,21,22,23,24,25,26,26,27,28,29,30,30,31,32,33,34,34,35,36,37,38,39,39,40,41,42,43,43,44,45,46,47,47,48,49,50,51,52,52,53,54,55,56,56,57,58,59,60,60,61,62,63,64,65,65,66,67,68,69,69,70,71,72,73,73,74,75,76,77,78,78,79,80,81,82,82,83,84,85,86,86,87,88,89,90,91,91,92,93,94,95,95,96,97,98,99,99,100,101,102,103,104,104,105,106,107,108,108,109,110,111,112,113,113,114,115,116,117,117,118,119,120,121,121,122,123,124,125,126,126,127,128,129,130,130,131,132,133,134,134,135,136,137,138,139,139,140,141,142,143,143,144,145,146,147,147,148,149,150,151,152,152,153,154,155,156,156,157,158,159,160,160,161,162,163,164,165,165,166,167,168,169,169,170,171,172,173,173,174,175,176,177,178,178,179,180,181,182,182,183,184,185,186,186,187,188,189,190,191,191,192,193,194,195,195,196,197,198,199,199,200,201,202,203,204,204,205,206,207}
};
static short int RUMap[3][256]={
{0,1,2,3,4,5,6,8,9,10,11,12,13,15,16,17,18,19,20,22,23,24,25,26,27,29,30,31,32,33,34,36,37,38,39,40,41,43,44,45,46,47,48,50,51,52,53,54,55,57,58,59,60,61,62,64,65,66,67,68,69,71,72,73,74,75,76,77,79,80,81,82,83,84,86,87,88,89,90,91,93,94,95,96,97,98,100,101,102,103,104,105,107,108,109,110,111,112,114,115,116,117,118,119,121,122,123,124,125,126,128,129,130,131,132,133,135,136,137,138,139,140,142,143,144,145,146,147,149,150,151,152,153,154,155,157,158,159,160,161,162,164,165,166,167,168,169,171,172,173,174,175,176,178,179,180,181,182,183,185,186,187,188,189,190,192,193,194,195,196,197,199,200,201,202,203,204,206,207,208,209,210,211,213,214,215,216,217,218,220,221,222,223,224,225,226,228,229,230,231,232,233,235,236,237,238,239,240,242,243,244,245,246,247,249,250,251,252,253,254,256,257,258,259,260,261,263,264,265,266,267,268,270,271,272,273,274,275,277,278,279,280,281,282,284,285,286,287,288,289,291,292,293,294,295,296},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,1,2,3,4,5,6,8,9,10,11,12,13,15,16,17,18,19,20,22,23,24,25,26,27,28,30,31,32,33,34,35,37,38,39,40,41,42,44,45,46,47,48,49,50,52,53,54,55,56,57,59,60,61,62,63,64,66,67,68,69,70,71,73,74,75,76,77,78,79,81,82,83,84,85,86,88,89,90,91,92,93,95,96,97,98,99,100,101,103,104,105,106,107,108,110,111,112,113,114,115,117,118,119,120,121,122,124,125,126,127,128,129,130,132,133,134,135,136,137,139,140,141,142,143,144,146,147,148,149,150,151,152,154,155,156,157,158,159,161,162,163,164,165,166,168,169,170,171,172,173,174,176,177,178,179,180,181,183,184,185,186,187,188,190,191,192,193,194,195,197,198,199,200,201,202,203,205,206,207,208,209,210,212,213,214,215,216,217,219,220,221,222,223,224,225,227,228,229,230,231,232,234,235,236,237,238,239,241,242,243,244,245,246,248,249,250,251,252,253,254,256,257,258,259,260,261,263,264,265,266,267,268,270,271,272,273,274,275,276,278,279,280,281,282,283,285,286,287,288,289,290,292,293,294,295}
};
/**
    B= 1.164*(Y-16) + 2.018*(U-128);
    G= 1.164*(Y-16) - 0.380*(U-128) - 0.813*(V-128);
    R= 1.164*(Y-16) + 1.159*(V-128);
*/
int yuv_rgb(int y, int u, int v)
{
    unsigned int pixel32 = 0;
    unsigned char *pixel = (unsigned char *)&pixel32;
    int r, g, b;
    b = BUMap[0][y]+BUMap[1][u]-277;
    g = GUMap[0][y]-GUMap[1][u]-GUMap[2][v]+134;
    r = RUMap[0][y]+RUMap[2][v]-148;
    if(r > 255) r = 255;
    if(g > 255) g = 255;
    if(b > 255) b = 255;
    if(r < 0) r = 0;
    if(g < 0) g = 0;
    if(b < 0) b = 0;
    pixel[2] = b ;
    pixel[1] = g ;
    pixel[0] = r ;
    return pixel32;
}

int Li_Arr_yuyv_bgr(unsigned char *yuv, unsigned char* rgb, int width, int height)
{
  unsigned int in, out = 0;
  unsigned int pixel_16;
  unsigned char pixel_24[3];
  unsigned int pixel32;
  int y0, u, y1, v;
  for(in = 0; in < width * height * 2; in += 4) {
    pixel_16 =
    ((unsigned char*)yuv)[in + 3] << 24 |
    ((unsigned char*)yuv)[in + 2] << 16 |
    ((unsigned char*)yuv)[in + 1] <<  8 |
    ((unsigned char*)yuv)[in + 0];
    y0 = (pixel_16 & 0x000000ff);
    u  = (pixel_16 & 0x0000ff00) >>  8;
    y1 = (pixel_16 & 0x00ff0000) >> 16;
    v  = (pixel_16 & 0xff000000) >> 24;
    pixel32 = yuv_rgb(y0, u, v);
    pixel_24[0] = (pixel32 & 0x000000ff);
    pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
    pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
    ((unsigned char*)rgb)[out++] = pixel_24[0];
    ((unsigned char*)rgb)[out++] = pixel_24[1];
    ((unsigned char*)rgb)[out++] = pixel_24[2];
    pixel32 = yuv_rgb(y1, u, v);
    pixel_24[0] = (pixel32 & 0x000000ff);
    pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
    pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
    ((unsigned char*)rgb)[out++] = pixel_24[0];
    ((unsigned char*)rgb)[out++] = pixel_24[1];
    ((unsigned char*)rgb)[out++] = pixel_24[2];
 }
 return 0;
}

int main(int argc ,void ** argv)
{
    cap_uvc.width=640
    cap_uvc.height=480;
    cap_uvc.ff_sws=new ffmpeg_sws
    cap_uvc.mat_convert=new opencv_mat(cap_uvc.width,cap_uvc.height);
    unsigned char* uvc_yuvbuf422=(unsigned char*)malloc(cap_uvc.width*cap_uvc.height*2+1000);
    unsigned char* uvc_rgbbuf=(unsigned char*)malloc(cap_uvc.width*cap_uvc.height*3+1000);
    unsigned char* plat_yuvbuf420=(unsigned char*)malloc(640*480*2+1000);
    unsigned char* plat_rgbbuf=(unsigned char*)malloc(640*480*3+1000);
    


    mat_convert=new opencv_mat(width,height);
    p=new framebuffer("/dev/fb0");
    p->print_info();
    Mat image,show;
    capture->camera_open();
    while(1){
        capture->camera_get_fps(0);
        yuvbuf422=capture->read_frame();
        Li_Arr_yuyv_bgr(yuvbuf422,rgbbuf,width,height);
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
