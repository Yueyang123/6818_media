#include "encode_6818.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>		  
#include <stdlib.h>         
#include <math.h>
#include <nx_fourcc.h>
#include <nx_vip.h>			
#include <nx_dsp.h>			
#include <nx_video_api.h>	


/**
 * @description: 将RAW图像数据转化为编码需要的数据格式
 * 将Psrc中的NV12中的数据拷贝到himage，准备编码
 * @param {*}
 * @return {*}
 * @author: YURI
 */
void encode_6818::vip_load_image(unsigned char *pSrc)
{
    int w=width;
    int h=height;
    NX_VID_MEMORY_INFO *pImg=himage;
	int32_t i, j;
	unsigned char *pDst, *pCb, *pCr;
    pDst = (unsigned char*)pImg->luVirAddr;
    memcpy(pDst, pSrc, w*h);//	Copy Y
    if(vip_encode_mode!=NX_JPEG_ENC){//NV12 情况下直接拷贝数据
        pDst = (unsigned char*)pImg->cbVirAddr;
        memcpy(pDst, pSrc+w*h, w*h/2);//COPY CR CB
    }else{ //JPEG 模式下NV12 转MVS0
        pCb = (unsigned char*)pImg->cbVirAddr;
        pCr = (unsigned char*)pImg->crVirAddr;
        pSrc+=w*h;
        for (int i = 0; i < w*h/4; i++)
        {
           *pCb++=*pSrc++;
           *pCr++=*pSrc++;
        }
    }
}
/**
 * @description: encode_6818初始6818编码器
 * @param {int} Width
 * @param {int} Height
 * @param {int} Fps
 * @param {ENCODE_TYPE} Encode_mode
 * @param {int} Encode_qp
 * @param {int} Gop_size
 * @return {*}
 * @author: YURI
 */
encode_6818::encode_6818(int Width,int Height,int Fps,ENCODE_TYPE Encode_mode,int Encode_qp,int Gop_size)
:encode(Width,Height,Fps,Encode_mode,Encode_qp,Gop_size)
{
    int instanceIdx;
    switch (Encode_mode)
    {
    case AVC_ENC:
        vip_encode_mode=NX_AVC_ENC;
        break;
    case MP4_ENC:
        vip_encode_mode=NX_MP4_ENC;
        break;
    case H263_ENC:
        vip_encode_mode=NX_H263_ENC;
        break;
    case JPEG_ENC:
        vip_encode_mode=NX_JPEG_ENC;
        break;  
    case H265_ENC:
        printf("DON'T SUPPORT H265 \r\n");
        vip_encode_mode=NX_AVC_ENC;
        break;          
    default:
        vip_encode_mode=NX_AVC_ENC;
        break;
    }
    seq_head = (unsigned char *)malloc( 4*1024 );
    vip_henc = NX_VidEncOpen(vip_encode_mode, &instanceIdx);   
    memset( &vip_encinitparam, 0, sizeof(vip_encinitparam));
    //基础参数
    vip_encinitparam.width = width;
    vip_encinitparam.height = height;
    vip_encinitparam.fpsNum = fps;          //视频帧率
    vip_encinitparam.fpsDen = 1;
    //速率控制参数
    vip_encinitparam.enableRC = 1;          //开启速率控制
    vip_encinitparam.chromaInterleave = 1;  //开启输入控制
    vip_encinitparam.bitrate = 1000000;     //比特率控制 10Mbps
    vip_encinitparam.gopSize = gop_size;    //多少帧一个关键帧
    vip_encinitparam.disableSkip = 0;       //不跳帧
    vip_encinitparam.maximumQp = 51;        //最大量化参数
    vip_encinitparam.initialQp = 10;        //初始量化参数
    vip_encinitparam.enableAUDelimiter = 1; //插入定界符
	if ( vip_encode_mode == NX_JPEG_ENC )
	{
        vip_encinitparam.chromaInterleave = 0;
        vip_encinitparam.jpgQuality = encode_qp;
    }
    //申请帧空间
    himage = NULL;
    if(vip_encode_mode!=NX_JPEG_ENC){
    himage = NX_VideoAllocateMemory( 4096, width, height, NX_MEM_MAP_LINEAR, FOURCC_NV12);
    }
    else{    
    himage = NX_VideoAllocateMemory( 4096, width, height, NX_MEM_MAP_LINEAR, FOURCC_MVS0);
    }
    vip_encin.timeStamp = 0;
    vip_encin.forcedIFrame = 0;
    vip_encin.forcedSkipFrame = 0;
    vip_encin.quantParam = encode_qp;   //量化参数
    vip_encin.pImage = himage;          //设置数据指针

    if (NX_VidEncInit( vip_henc, &vip_encinitparam ) != VID_ERR_NONE)
    {
        printf("6818 encode init failed \n");
        exit(-1);
    }
    printf("6818 encode init success \n");
}
/**
 * @description: 释放编码器
 * @param {*}
 * @return {*}
 * @author: YURI
 */
encode_6818::~encode_6818()
{
    NX_VidEncClose( vip_henc );
    free(seq_head);
}

unsigned char* encode_6818::encode_get_headinfo(int* length)
{
    if ( vip_encode_mode != NX_JPEG_ENC )
        NX_VidEncGetSeqInfo( vip_henc, seq_head, &encode_seq_length );
    else
        NX_VidEncJpegGetHeader( vip_henc, seq_head, &encode_seq_length );
    *length=encode_seq_length;
    return seq_head;
}

/**
 * @description: encode_enc_frame 编码数据 src nv12
 * @param {unsigned char*} src
 * @param {int} *length
 * @return {*}
 * @author: YURI
 */
unsigned char* encode_6818::encode_enc_frame(unsigned char* src,int *length)
{
    vip_load_image((unsigned char*)src);
    fflush(stdout);
    if(vip_encode_mode != NX_JPEG_ENC){
        NX_VidEncEncodeFrame( vip_henc, &vip_encin, &vip_encout );
    }else{
        NX_VidEncJpegRunFrame( vip_henc, vip_encin.pImage, &vip_encout );
    }
    fflush(stdout);
    encode_data_length=vip_encout.bufSize;
    *length=encode_data_length;
    unsigned char* data=(unsigned char*)vip_encout.outBuf;
    return data;
}


