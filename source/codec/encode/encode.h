/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-25 19:06:08
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-25 23:42:08
 */
#ifndef ENCODE_H
#define ENCODE_H
#include <unistd.h>		//	getopt & optarg
#include <stdlib.h>		//	atoi
#include <stdio.h>		//	printf
#include <string.h>		//	strdup
#include <sys/time.h>   //	gettimeofday
#include <math.h>
#include <time.h>
typedef enum {
	AVC_ENC      = 0x00,
	MP4_ENC      = 0x01,
	H263_ENC     = 0x02,
	JPEG_ENC     = 0x03,
    H265_ENC     = 0x04,
} ENCODE_TYPE;

class encode
{
public:
    int width;
    int height;
    int fps;
    int encode_qp;          //编码的量化参数，0-100
    int gop_size;           //关键帧间距
    unsigned char *encode_data;   //编码后的数据
    int     encode_data_length;//编码得到的数据长度
    unsigned char *seq_head;      //编码需要放在头部的数据
    int     encode_seq_length;//文件头长度
    ENCODE_TYPE encode_mode;//选择编码类型
    double real_fps;        //在运行中实际的帧率
    clock_t start,finish;
public:
    encode(int Width,int Height,int Fps=30,ENCODE_TYPE Encode_mode=AVC_ENC,int Encode_qp=50,int Gop_size=30)
    {
        width=Width;
        height=Height;
        fps=Fps;
        encode_mode=Encode_mode;
        encode_qp=Encode_qp;
        gop_size=Gop_size;
    }
    virtual ~encode(){};
    virtual unsigned char* encode_get_headinfo(int* length)=0;//获取头部数据
    virtual unsigned char* encode_enc_frame(unsigned char* src,int *length)=0;//获取编码数据
    virtual void encode_change_fps(int Fps)=0;
    /**
     * @description: 在 encode_get_frame
     * 前面运行encode_get_fps(0)
     * 后面运行encode_get_fps(1)
     * @param {int} index
     * @return {*}
     * @author: YURI
     */    
    double encode_get_fps(int index){
        if(index==0){
            start=clock();
            return 0;
        }
        else{
            finish=clock();
            real_fps=(1000/((double)(finish-start)/CLOCKS_PER_SEC*1000))/2;
            fps=(int)real_fps;
            return real_fps;
        }
    }
    
};




#endif