/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-28 00:39:32
 * @LastEditors: YURI
 * @LastEditTime: 2022-02-04 08:03:03
 */
#include "audio_wave.h"

/**
 * @description: 保存wav文件时使用的初始化
 * @param {string} filepath
 * @param {int} rate
 * @param {int} bit_rate
 * @param {int} channel
 * @return {*}
 * @author: YURI
 */
audio_wave::audio_wave(WAVE_FILE_MODE mode,string filepath,int rate,int bit_rate,int channel)
{
    if(mode!=WAVE_FILE_WRITE_MODE){
        fprintf(stderr,"DON'T SUPPORT THIS MODE \r\n");
        return;
    }
    this->sample_rate=rate;
    this->bit_rate=bit_rate;
    this->channel=channel;
    this->file_path=filepath;
    this->data_length=0;

    audio_wave_head.bit_rate=bit_rate;
    audio_wave_head.bits_per_sample=bit_rate;
    audio_wave_head.channel=channel;
    audio_wave_head.sample_rate=sample_rate;
    audio_wave_head.byte_rate=sample_rate*(bit_rate/8)*channel;
    audio_wave_head.block_align=(bit_rate/8)*channel;    
}
/**
 * @description: 打开文件
 * @param {*}
 * @return {*}
 * @author: YURI
 */
void audio_wave::audio_write_start(){
    file_fd=fopen(file_path.c_str(),"wb");
    fseek(file_fd,44,SEEK_SET);
    data_length=0;
}

/**
 * @description: 向文件中写入一帧
 * @param {unsigned char*} frame
 * @param {int} size
 * @return {*}
 * @author: YURI
 */
void audio_wave::audio_write_frame(unsigned char* frame,int size){
    fwrite(frame,size,1,file_fd);
    data_length+=size;
}

/**
 * @description: 结束wav文件
 * @param {*}
 * @return {*}
 * @author: YURI
 */
void audio_wave::audio_write_end(void)
{
    audio_wave_head.data_length=data_length;
    audio_wave_head.wave_length=data_length+44-8;
    fseek(file_fd,0,SEEK_SET);
    fwrite(&audio_wave_head,sizeof(audio_wave_head),1,file_fd);
    fclose(file_fd);
}
/**
 * @description: 读取文件情况下的初始化函数
 * @param {string} filepath
 * @return {*}
 * @author: YURI
 */
audio_wave::audio_wave(WAVE_FILE_MODE mode,string filepath)
{
    if(mode!=WAVE_FILE_READ_MODE){
        fprintf(stderr,"DON'T SUPPORT THIS MODE \r\n");
        return;
    }
    this->file_path=filepath;
    file_fd =fopen(file_path.c_str(),"rb");
}
/**
 * @description: 开始读取，读取文件头
 * @param {*}
 * @return {*}
 * @author: YURI
 */
void audio_wave::audio_read_start(void)
{
    fread((void*)&audio_wave_head,44,1,file_fd);
    if(audio_wave_head.wave_header[0]!='R' ){
        fprintf(stderr,"THE FILE IS NOT WAV \r\n");
        return;
    }
    printf("CHANNEL %d \r\n",audio_wave_head.channel);
    printf("SAMPLE_RATE %d \r\n",audio_wave_head.sample_rate);
}
/**
 * @description: 从文件读一帧出来
 * @param {unsigned char*} frame
 * @param {int} size
 * @return {*}
 * @author: YURI
 */
int audio_wave::audio_read_frame(unsigned char* frame,int size)
{
    int ret= fread((void*)frame,size,1,file_fd);
    return ret;
}
/**
 * @description: 结束读取关闭文件
 * @param {*}
 * @return {*}
 * @author: YURI
 */
void audio_wave::audio_read_end(void){
    fclose(file_fd);
}