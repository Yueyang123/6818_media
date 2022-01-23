/*
 * @Description: 提供UVC 摄像头访问接口
 * @Autor: YURI
 * @Date: 2022-01-21 01:05:31
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-22 22:04:03
 */
#ifndef CAMERA_UVC
#define CAMERS_UVC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h> 
#include <linux/videodev2.h>
//V4L2的 DQBUF得到的数据格式，一个指针，一个长度
struct buffer {
    void * start;
    size_t length;
};

class camera_uvc
{
private:
    //V4L2能力
    struct v4l2_capability cap;
    //V4L2选择格式
    struct v4l2_fmtdesc fmt1;
    //当前中的V4L2格式
    struct v4l2_format select_fmt;
    //v4l2申请空间
    struct v4l2_requestbuffers req;
    //打开摄像头的描述符
    int video_fd;
    //申请出来的摄像头空间
    struct buffer* buffers;
    //返回的图像数据
    unsigned char* frame;
    double fps;
public:
    camera_uvc(int videoindex, int piexlformat, int width,int height);
    ~camera_uvc();
    int camera_get_capability(void);
    void camera_show_capability(void);
    int camera_get_format(void);
    int camera_set_format(int piexlformat,int width,int height);
    int camera_alloc_buffer(int count);
    unsigned char* read_frame (void);
    double camera_get_fps(){return fps;}
};


#endif