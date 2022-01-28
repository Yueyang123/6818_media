/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-26 18:08:08
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-27 18:48:12
*/
#ifndef AUDIO_H
#define AUDIO_H
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#define FRAME_INIT           1024 //划分帧 
#define MAX_CHANNEL          4

using namespace std;
class audio
{
private:
    string audio_path;          //ALSA音频设备
    
	int channel;                //采集设备的通道数
    unsigned int sample_rate; 
    snd_pcm_t *capture_handle;			// 一个指向PCM设备的句柄
    unsigned char *buffer_in;  
	snd_pcm_hw_params_t *hw_params;		// 此结构包含有关硬件的信息，可用于指定PCM流的配置
	snd_pcm_format_t format ;		    // 采样位数：16bit、LE格式
    


public:
	int frame_size;
    audio(string audio_tag="default",int channel=2,int sample_rate=48000,snd_pcm_format_t pcm_format=SND_PCM_FORMAT_S16_LE);
    ~audio();
    void audio_open_micophone(void);
    unsigned char* audio_read_frame();//从麦克风读一帧出来
	void audio_close_micophone(void);
	void audio_channel_split(unsigned char ** data);//分离声道数据，两个数据空间需要在外部申请

};

#endif