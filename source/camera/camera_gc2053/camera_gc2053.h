/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-24 01:26:58
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-24 04:02:03
 */
#ifndef CAMERA_GC2053_H
#define CAMERA_GC2053_H
#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

class camera_gc2053:public camera
{
private:
    int open_flag;
public:
    camera_gc2053(int width,int height, int piexlformat,int videoindex);
    ~camera_gc2053();
    virtual int camera_alloc_buffer(int count);
    virtual unsigned char* read_frame (void);
    virtual int camera_open(void);
};


#endif