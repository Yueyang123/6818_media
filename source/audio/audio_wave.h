/*
 * @Description: 提供音频保存为wav文件的接口
 * @Autor: YURI
 * @Date: 2022-01-28 00:39:43
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-28 07:10:14
 */
#ifndef AUDIO_WAVE_H
#define AUDIO_WAVE_H
#include "stdio.h"
#include <string>
using namespace std;
typedef struct _wave_header_t {
    char    wave_header[4];     //WAVE 头1
    int     wave_length;        //音频数据的长度 +44 -8
    char    format[8];          //WAVE+fmt
    int     bit_rate;           //一个采样点占几个bit
    short   pcm;                //音频数据编码方式             
    short   channel;            //通道数
    int     sample_rate;        //采样率
    int     byte_rate;          //采样率×每次采样大小
    short   block_align;        //每次采样的大小 = 采样精度*声道数/8(单位是字节) 譬如 16bit 立体声在这里的值是 4 字节
    short   bits_per_sample;    //每个声道的采样精度; 譬如 16bit 在这里的值就是16
    char    fix_data[4];        //"data"
    int     data_length;        //音频数据的长度
} wave_t;

class audio_wave
{
private:
    FILE* file_fd;
    string file_path;
    int channel;
    int data_length;
    int sample_rate;
    int bit_rate;
    wave_t audio_wave_head={
        {'R', 'I', 'F', 'F'},
        ( int)-1,
        {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '},
        bit_rate,
        0x01,
        channel,
        sample_rate,
        sample_rate*(bit_rate/8)*channel,
        (bit_rate/8)*channel,
        bit_rate,
        {'d', 'a', 't', 'a'},
        (int)-1          
    };
public:
    audio_wave(string file_path,int rate=48000,int bit_rate=16,int channel=2);
    ~audio_wave(){};
    //打开文件
    void audio_start();
    //向文件中写入一帧
    void audio_write_frame(unsigned char* frame,int size);
    //结束wav文件
    void audio_end(void);
};



#endif
