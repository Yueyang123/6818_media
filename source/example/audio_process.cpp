/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-27 07:33:35
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-27 18:50:20
 */
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "audio.h"
#include <string>
using namespace std;

FILE *out1,*out2;		// 一个指向文件的指针，文件输出流


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

	audio* aud=new audio(mico_path,channel,rate,format);
	aud->audio_open_micophone();
	unsigned char * buffer;
	unsigned char * data[2];
	data[0]=(unsigned char*)malloc(2*aud->frame_size) ;
	data[1]=(unsigned char*)malloc(2*aud->frame_size) ;
	char file[20];
	sprintf(file,"%s_0.pcm",save_path.c_str());
	out1= fopen(file, "wb");
	sprintf(file,"%s_1.pcm",save_path.c_str());
	out2= fopen(file, "wb");
	// 开始采集音频pcm数据
	while (1) 
	{
		buffer=aud->audio_read_frame();
		aud->audio_channel_split(data);
		fwrite( data[0],2*aud->frame_size, 1,out1);
		fwrite( data[1],2*aud->frame_size, 1,out2);
	}

	// 关闭文件流
	fclose(out1);
	fclose(out2);
	exit (0);
}