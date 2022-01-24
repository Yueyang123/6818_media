/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-23 01:57:31
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-24 05:55:56
 */
#ifndef FFMPEG_SWS_INTERFACE
#define FFMPEG_SWS_INTERFACE
extern "C"{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
class ffmpeg_sws_interface
{
private:
    int width;
    int height;
    AVPixelFormat src_format;
    AVPixelFormat dst_format;
    AVFrame  *frmsrc ;
    AVFrame  *frmdst ;
    struct SwsContext *sws;
public:
    ffmpeg_sws_interface(int iwidth,int iheight,AVPixelFormat isrc_format,AVPixelFormat idst_format); 
    int ffmpeg_sws_convert(uint8_t *src,uint8_t *dst);
    ~ ffmpeg_sws_interface();
};


#endif