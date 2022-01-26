/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-25 06:48:58
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-25 23:21:02
 */
#ifndef VPU_6818_H
#define VPU_6818_H

#include <nx_fourcc.h>
#include <nx_vip.h>			//	VIP
#include <nx_dsp.h>			//	Display
#include <nx_video_api.h>	//	Video En/Decoder
#include "encode.h"

class encode_6818:public encode
{
private:
	NX_VID_ENC_HANDLE vip_henc;                  //编码器句柄
    NX_VID_ENC_INIT_PARAM vip_encinitparam;      //编码参数设置关键变量
 	NX_VID_MEMORY_HANDLE vip_hmem;               //申请转换内存空间
    VID_TYPE_E vip_encode_mode;                  //通用转换形式
    NX_VID_ENC_IN vip_encin;
	NX_VID_ENC_OUT vip_encout;
    NX_VID_MEMORY_INFO *himage;
    void vip_load_image(uint8_t *pSrc);
public:
    virtual unsigned char* encode_get_headinfo(int* length);//获取头部数据
    virtual unsigned char* encode_enc_frame(unsigned char* src,int *length);//获取编码数据
    virtual void encode_change_fps(int Fps){
        vip_encinitparam.fpsNum = Fps;
        this->fps=Fps;
    };
    encode_6818(int Width,int Height,int Fps=30,ENCODE_TYPE Encode_mode=AVC_ENC,int Encode_qp=50,int Gop_size=30);
    ~encode_6818();
};

#endif
