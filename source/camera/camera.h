/*
 * @Description: 摄像头程序的统一接口
 * @Autor: YURI
 * @Date: 2022-01-23 17:54:52
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-23 23:48:07
 */
#ifndef CAMERA_H
#define CAMERA_H
#include "ffmpeg_sws.h"
#include <time.h>
/**
 * @description: 
 * 这是一个虚类，不能实例化 
 * 只能作为指针指向具体的摄像头
 * 例如 camera * cap=new camera_xxx()
 */
class camera
{
public:
    //返回的图像数据
    unsigned char* frame;
    double fps;
    int width,height;
    //相机编号 
    int videoindex;
    //像素格式引用FFMPEG对图象的定义方式
    int piexlformat;
    clock_t start,finish;
public:
    camera(int width,int height, int piexlformat,int videoindex)
    {
        this->width=width;
        this->height=height;
        this->piexlformat=piexlformat;
        this->videoindex=videoindex;
    }
    ~camera(){};
    virtual int camera_get_capability(void)= 0;
    virtual void camera_show_capability(void)= 0;
    virtual int camera_get_format(void)= 0;
    virtual int camera_set_format(int piexlformat,int width,int height)= 0;
    virtual int camera_alloc_buffer(int count)= 0;
    virtual unsigned char* read_frame (void)= 0;
    virtual int camera_open(void)= 0;
    /**
     * @description: 计算帧率使用的函数
     * 在做所有任务之前运行一次（参数0），所有任务完成之后
     * 再运行一次（参数1）
     * @param {int} index
     * @return {*}
     * @author: YURI
     */    
    double camera_get_fps(int index){
        if(index==0){
            start=clock();
            return 0;
        }
        else{
            finish=clock();
            fps=1000/((double)(finish-start)/CLOCKS_PER_SEC*1000);
            return fps;
        }
    }
};

#endif