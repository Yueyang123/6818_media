/*
 * @Description: 提供NVP6124 摄像头访问接口
 * @Autor: YURI
 * @Date: 2022-01-21 01:05:31
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-21 01:05:32
 */
#ifndef CAMERA_H
#define CAMERA_H

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/media.h>
#include <linux/v4l2-subdev.h>
#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>
#include <ion.h>
#include <linux/ion.h>
#include <linux/nxp_ion.h>
#include <linux/videodev2_nxp_media.h>
#include <nxp-v4l2.h>
#include <string>

using namespace std;

#define MAX_BUFFER_COUNT 4
#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))


#define YUV_STRIDE_ALIGN_FACTOR     64
#define YUV_VSTRIDE_ALIGN_FACTOR    16

#define YUV_STRIDE(w)    ALIGN(w, YUV_STRIDE_ALIGN_FACTOR)
#define YUV_YSTRIDE(w)   (ALIGN(w/2, YUV_STRIDE_ALIGN_FACTOR) * 2)
#define YUV_VSTRIDE(h)   ALIGN(h, YUV_VSTRIDE_ALIGN_FACTOR)

class camera
{
private:
    int clipper_id ;
    int sensor_id ;
    int video_id ;
    int format ;
    int width;
    int height;
    int ion_fd;
    float fps;
    struct nxp_vid_buffer bufs[MAX_BUFFER_COUNT];
    unsigned char* yuv420m_buf;//width*height*3/2的连续空间
    void CHECK_COMMAND(int command) ;
    unsigned int yuv420m_get_size(int format, int num, int width, int height);
    int yuv420m_alloc_buffers(int ion_fd, int count, struct nxp_vid_buffer *bufs, int width, int height, int format);

public:
    camera(int width,int height);//初始化摄像节点
    ~camera();
    void waitquit(string str);
    unsigned char* read_frame();
    float get_fps(){return fps;}
};

#endif