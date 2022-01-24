/*
 * @Description: 提供UVC 摄像头访问接口
 * @Autor: YURI
 * @Date: 2022-01-21 01:05:31
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-24 04:04:36
 */
#ifndef CAMERA_UVC
#define CAMERA_UVC
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
#include "camera.h"
//V4L2的 DQBUF得到的数据格式，一个指针，一个长度
struct buffer {
    void * start;
    size_t length;
};

class camera_uvc:public camera
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

public:
    camera_uvc(int width,int height, int piexlformat,int videoindex);
    ~camera_uvc();
     int camera_get_capability(void);
     void camera_show_capability(void);
     int camera_get_format(void);
     int camera_set_format(int piexlformat,int width,int height);
    virtual int camera_alloc_buffer(int count);
    virtual unsigned char* read_frame (void);
    virtual int camera_open(void);
};


#endif