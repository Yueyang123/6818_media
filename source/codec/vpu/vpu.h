/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-25 06:48:58
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-25 07:33:58
 */
#ifndef VPU_H
#define VPU_H

#include <unistd.h>		//	getopt & optarg
#include <stdlib.h>		//	atoi
#include <stdio.h>		//	printf
#include <string.h>		//	strdup
#include <sys/time.h>		//	gettimeofday
#include <math.h>

#include <nx_fourcc.h>
#include <nx_vip.h>			//	VIP
#include <nx_dsp.h>			//	Display
#include <nx_video_api.h>	//	Video En/Decoder

#include "NX_Queue.h"
#include <Util.h>

class vpu
{
private:
    int32_t opt;
	int32_t mode;
	NX_VID_ENC_HANDLE hEnc;                    // Encoder Handle
    NX_VID_ENC_INIT_PARAM encInitParam;
 	NX_VID_MEMORY_HANDLE hMem;      // Allocate Memory for Encoder Input
	NX_VID_MEMORY_INFO *hInImage;            // Previous Displayed Memor
    uint8_t *pSrcBuf ;
    int32_t inWidth;
    int32_t inHeight;
    FILE *fdOut;
    VID_TYPE_E encode_mode;
public:
    int32_t LoadImage( uint8_t *pSrc, int32_t w, int32_t h, NX_VID_MEMORY_INFO *pImg );
    char*  DecodeNV12_To_H264(void *nv12,int *length);
    NX_VID_ENC_IN encIn;
	NX_VID_ENC_OUT encOut;
    unsigned char *seqBuffer;
    int seqsize;
    vpu(int width,int height,int Fps,VID_TYPE_E encodemode);
    ~vpu();
};

#endif
