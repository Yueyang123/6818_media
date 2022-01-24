/*
 * @Description: 提供ffmpeg raw转换接口
 * @Autor: YURI
 * @Date: 2022-01-21 01:05:31
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-24 05:55:10
 */
#include "ffmpeg_sws.h"


 ffmpeg_sws_interface:: ffmpeg_sws_interface(int iwidth,int iheight,AVPixelFormat isrc_format,AVPixelFormat idst_format)
:width(iwidth),height(iheight),src_format(isrc_format),dst_format(idst_format)
{
    frmsrc = av_frame_alloc();
    frmdst = av_frame_alloc();
    sws = sws_getContext(width, height, src_format, width,height, dst_format,
                                            SWS_BILINEAR, NULL, NULL, NULL);

}

 ffmpeg_sws_interface::~ ffmpeg_sws_interface()
{
    av_frame_free(&frmsrc);
    av_frame_free(&frmdst);
    sws_freeContext(sws);
}


int ffmpeg_sws_interface::ffmpeg_sws_convert(uint8_t *src,uint8_t *dst)
{
    //绑定数据缓冲区
    avpicture_fill((AVPicture *)frmsrc, src, src_format, width, height);
    avpicture_fill((AVPicture *)frmdst, dst, dst_format, width, height);
    int ret = sws_scale(sws, frmsrc->data, frmsrc->linesize, 0, height, frmdst->data, frmdst->linesize);
    return  (ret == height) ? 0 : -1;
}
