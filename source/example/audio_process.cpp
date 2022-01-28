/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-27 07:33:35
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-28 07:42:40
 */
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "audio.h"
#include "audio_wave.h"
#include "signal.h"
#include <string>
using namespace std;
audio_wave *audio_wave1,*audio_wave2;
void record_stop(int signo)
{
	printf("end \r\n");
	audio_wave1->audio_end();
	audio_wave2->audio_end();
}
int main (int argc, char *argv[])
{
	int i;
	int err;
	string mico_path=string(argv[1]);
	string save_path=string(argv[2]);
	int channel =2;
	unsigned int rate = 48000;			// 采样频率：	44100Hz
	snd_pcm_t *capture_handle;			// 一个指向PCM设备的句柄
	snd_pcm_hw_params_t *hw_params;		// 此结构包含有关硬件的信息，可用于指定PCM流的配置
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;		// 采样位数：16bit、LE格式
	signal(SIGINT, record_stop); 
	audio* aud=new audio(mico_path,channel,rate,format);
	aud->audio_open_micophone();
	unsigned char * buffer;
	unsigned char * data[2];
	data[0]=(unsigned char*)malloc(2*aud->frame_size) ;
	data[1]=(unsigned char*)malloc(2*aud->frame_size) ;
	char file[20];
	sprintf(file,"%s_0.wav",save_path.c_str());
	audio_wave1=new audio_wave(string(file),48000,16,1);
	audio_wave1->audio_start();
	sprintf(file,"%s_1.wav",save_path.c_str());
	audio_wave2=new audio_wave(string(file),48000,16,1);
	audio_wave2->audio_start();
	
	// 开始采集音频pcm数据
	for(int i=0;i<300;i++)
	{
		buffer=aud->audio_read_frame();
		aud->audio_channel_split(data);
		audio_wave1->audio_write_frame(data[0],2*aud->frame_size);
		audio_wave2->audio_write_frame(data[1],2*aud->frame_size);
	}
	audio_wave1->audio_end();
	audio_wave2->audio_end();
	return 0;
}