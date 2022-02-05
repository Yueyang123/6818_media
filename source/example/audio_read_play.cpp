/*
 * @Description: 读取WAV文件并且播放
 * @Autor: YURI
 * @Date: 2022-02-04 06:55:42
 * @LastEditors: YURI
 * @LastEditTime: 2022-02-04 20:51:15
 */
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "audio.h"
#include "audio_wave.h"
#include "signal.h"
#include <string>
using namespace std;
audio_wave *audio_wave1;
unsigned char* buffer;
audio *aud;
int main(int argc,void** argv)
{
    string speaker_path=string(( char*)argv[1]);
    int channel =2;
    unsigned int rate = 48000;			// 采样频率：	48000
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;		// 采样位数：16bit、LE格式
    aud=new audio(AUDIO_SPEAKER,speaker_path,channel,rate,format);
    aud->audio_open_speaker();
    audio_wave1=new audio_wave(WAVE_FILE_READ_MODE,string((char*)argv[2]));
    audio_wave1->audio_read_start();
    
    buffer=(unsigned char*)malloc(4*aud->frame_size);
    while(audio_wave1->audio_read_frame(buffer,4*aud->frame_size) >0){
        aud->audio_write_frame(buffer);
    }
    aud->audio_close_speaker();
    audio_wave1->audio_read_end();
    return 0;
}

