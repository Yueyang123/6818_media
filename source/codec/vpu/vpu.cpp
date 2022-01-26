#include "vpu.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>		    //	getopt & optarg
#include <stdlib.h>         //	atoi
#include <sys/time.h>		//	gettimeofday
#include <math.h>

#include <nx_fourcc.h>
#include <nx_vip.h>			//	VIP
#include <nx_dsp.h>			//	Display
#include <nx_video_api.h>	//	Video En/Decoder

#include "NX_Queue.h"
#include "Util.h"


#define	MAX_SEQ_BUF_SIZE		(4*1024)
#define	MAX_ENC_BUFFER			8
#define	ENABLE_NV12				1

enum
{
	MODE_NONE,
	DECODER_MODE,
	ENCODER_MODE,
	JPEG_MODE,
	MODE_MAX
};

//将RAW图像数据转化为编码需要的数据格式
//PSRC NV12
int32_t vpu:: LoadImage( uint8_t *pSrc, int32_t w, int32_t h, NX_VID_MEMORY_INFO *pImg )
{
	int32_t i, j;
	uint8_t *pDst, *pCb, *pCr;
    pDst = (uint8_t*)pImg->luVirAddr;
    memcpy(pDst, pSrc, w*h);//	Copy Y
    if(encode_mode!=NX_JPEG_ENC){//NV12 情况下直接拷贝数据
        pDst = (uint8_t*)pImg->cbVirAddr;
        memcpy(pDst, pSrc+w*h, w*h/2);//COPY CR CB
    }else{ //JPEG 模式下NV12 转MVS0
        pCb = (uint8_t*)pImg->cbVirAddr;
        pCr = (uint8_t*)pImg->crVirAddr;
        pSrc+=w*h;
        for (int i = 0; i < w*h/4; i++)
        {
           *pCb++=*pSrc++;
           *pCr++=*pSrc++;
        }
    }

	return 0;
}



vpu::vpu(int width,int height,int Fps,VID_TYPE_E encodemode)
{
    inWidth=width;
    inHeight=height;
    encode_mode=encodemode;

    seqBuffer = (unsigned char *)malloc( MAX_SEQ_BUF_SIZE );
    long long totalSize = 0;
    long long vipTimeStamp;
    int instanceIdx;
    hEnc = NX_VidEncOpen(encode_mode, &instanceIdx);   
    memset( &encInitParam, 0, sizeof(encInitParam));
    //基础参数
    encInitParam.width = inWidth;
    encInitParam.height = inHeight;
    encInitParam.fpsNum = Fps;          //视频帧率
    encInitParam.fpsDen = 1;
    //速率控制参数
    encInitParam.enableRC = 1;          //开启速率控制
    encInitParam.chromaInterleave = 1;  //开启输入控制
    encInitParam.bitrate = 1000000;     //比特率控制 10Mbps
    encInitParam.gopSize = 30/2;        //15帧一个关键帧
    encInitParam.disableSkip = 0;       //不跳帧
    encInitParam.maximumQp = 51;        //最大量化参数
    encInitParam.initialQp = 10;        //初始量化参数
    encInitParam.enableAUDelimiter = 1; //插入定界符
	if ( encode_mode == NX_JPEG_ENC )
	{
        encInitParam.chromaInterleave = 0;
        encInitParam.jpgQuality = 90;
    }
    //申请帧空间
    hInImage = NULL;
    if(encode_mode!=NX_JPEG_ENC)
    hInImage = NX_VideoAllocateMemory( 4096, inWidth, inHeight, NX_MEM_MAP_LINEAR, FOURCC_NV12);
    else    
    hInImage = NX_VideoAllocateMemory( 4096, inWidth, inHeight, NX_MEM_MAP_LINEAR, FOURCC_MVS0);

    encIn.timeStamp = 0;
    encIn.forcedIFrame = 0;
    encIn.forcedSkipFrame = 0;
    encIn.quantParam = 23;
    encIn.pImage = hInImage;      
        
    if (NX_VidEncInit( hEnc, &encInitParam ) != VID_ERR_NONE)
    {
        printf("NX_VidEncInit() failed \n");
        exit(-1);
    }

    printf("NX_VidEncInit() success \n");
    if ( encode_mode != NX_JPEG_ENC )
        NX_VidEncGetSeqInfo( hEnc, seqBuffer, &seqsize );
    else
        NX_VidEncJpegGetHeader( hEnc, seqBuffer, &seqsize );
}

char*  vpu::DecodeNV12_To_H264(void *yuv420p,int *length)
{
    LoadImage((uint8_t*)yuv420p,inWidth , inHeight, encIn.pImage);
    if(encode_mode != NX_JPEG_ENC){
        NX_VidEncEncodeFrame( hEnc, &encIn, &encOut );
    }else{
        NX_VidEncJpegRunFrame( hEnc, encIn.pImage, &encOut );
    }
    *length=encOut.bufSize;
    char* data=(char*)encOut.outBuf;
    return data;
}


vpu::~vpu()
{
    NX_VidEncClose( hEnc );
    free(seqBuffer);
}
