/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-26 18:08:08
 * @LastEditors: YURI
 * @LastEditTime: 2022-02-04 20:06:39
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

typedef enum {AUDIO_MICPHONE=0,AUDIO_SPEAKER=1}AUDIO_TYPE;
class audio
{
private:
    string audio_path;          //ALSA音频设备
    
	int channel;                //采集设备的通道数
    unsigned int sample_rate; 
    snd_pcm_t *capture_handle;			// 一个指向PCM设备的句柄
    unsigned char *buffer_in;           //麦克风采集用的数据
    unsigned char *buffer_out;          //话筒播放的数据
	snd_pcm_hw_params_t *hw_params;		// 此结构包含有关硬件的信息，可用于指定PCM流的配置
	snd_pcm_format_t format ;		    // 采样位数：16bit、LE格式
    AUDIO_TYPE audio_type;
public:
	int frame_size;
    audio(AUDIO_TYPE type,string audio_tag="default",int channel=2,int sample_rate=48000,snd_pcm_format_t pcm_format=SND_PCM_FORMAT_S16_LE,int frame_size=FRAME_INIT);
    ~audio();
    void audio_open_micophone(void);
    unsigned char* audio_read_frame(void);//从麦克风读一帧出来
	void audio_close_micophone(void);
	void audio_channel_split(unsigned char ** data);//分离声道数据，两个数据空间需要在外部申请
    void audio_open_speaker(void);//打开麦克风
    void audio_close_speaker(void);//关闭麦克风
    void audio_write_frame(unsigned char* data);//写一帧数据
};

#endif