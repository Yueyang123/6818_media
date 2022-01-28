/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-28 00:39:32
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-28 07:40:24
 */
#include "audio_wave.h"

audio_wave::audio_wave(string file_path,int rate,int bit_rate,int channel)
{
    this->sample_rate=rate;
    this->bit_rate=bit_rate;
    this->channel=channel;
    this->file_path=file_path;
    this->data_length=0;

    audio_wave_head.bit_rate=bit_rate;
    audio_wave_head.bits_per_sample=bit_rate;
    audio_wave_head.channel=channel;
    audio_wave_head.sample_rate=sample_rate;
    audio_wave_head.byte_rate=sample_rate*(bit_rate/8)*channel;
    audio_wave_head.block_align=(bit_rate/8)*channel;    
}

//打开文件
void audio_wave::audio_start(){
    file_fd=fopen(file_path.c_str(),"wb");
    fseek(file_fd,44,SEEK_SET);
    data_length=0;
}
//向文件中写入一帧
void audio_wave::audio_write_frame(unsigned char* frame,int size){
    fwrite(frame,size,1,file_fd);
    data_length+=size;
}
//结束wav文件
void audio_wave::audio_end(void)
{
    audio_wave_head.data_length=data_length;
    audio_wave_head.wave_length=data_length+44-8;
    fseek(file_fd,0,SEEK_SET);
    fwrite(&audio_wave_head,sizeof(audio_wave_head),1,file_fd);
    fclose(file_fd);
}