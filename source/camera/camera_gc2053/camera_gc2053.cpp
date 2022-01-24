/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-24 01:26:47
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-24 04:02:14
 */
#include "camera_gc2053.h"
#include "sample_comm.h"
#include <linux/videodev2.h>
/**
 * @description: camera_gc2053初始化
 * @param {int} width
 * @param {int} height
 * @param {int} piexlformat
 * @param {int} videoindex
 * @return {*}
 * @author: YURI
 */
camera_gc2053::camera_gc2053(int width,int height, int piexlformat,int videoindex):
camera(width,height, piexlformat, videoindex)
{
    open_flag=0;
    printf("INIT CAMERA \r\n");
    if(piexlformat!=V4L2_PIX_FMT_NV21){
        open_flag=-1;
        printf("DONT SURPORT THIS FORMAT \r\n");
    }
    if(width!=1920){
        open_flag=-1;
        printf("DONT SURPORT THIS WIDTH \r\n");
    }
    if(height!=1080){
        open_flag=-1;
        printf("DONT SURPORT THIS HEIGHT \r\n");
    }
}
/**
 * @description: 释放空间
 * @param {*}
 * @return {*}
 * @author: YURI
 */
camera_gc2053::~camera_gc2053()
{
    free(frame);
}
/**
 * @description: camera_alloc_buffer申请空间
 * @param {int} count
 * @return {*}
 * @author: YURI
 */
int camera_gc2053::camera_alloc_buffer(int count){
    frame=(unsigned char*)malloc(width*height*3/2);
    if(frame!=NULL)return 1;
    else return -1;
}
/**
 * @description: read_frame读取数据
 * @param {*}
 * @return {*}
 * @author: YURI
 */
unsigned char* camera_gc2053::read_frame (void){
    static VIDEO_FRAME_INFO_S stFrame;
    memset(&stFrame, 0, sizeof(stFrame));
	HI_MPI_VPSS_GetChnFrame(0, VPSS_CHN0, &stFrame, -1);
    VIDEO_FRAME_S *pVBuf= &stFrame.stVFrame;
    HI_CHAR* pVBufVirt=NULL;
    PIXEL_FORMAT_E enPixelFormat = pVBuf->enPixelFormat;
    int u32Size=pVBuf->u32Stride[videoindex] * pVBuf->u32Height * 3 / 2;
    pVBufVirt =(HI_CHAR*)HI_MPI_SYS_Mmap(pVBuf->u64PhyAddr[videoindex], u32Size);
    memcpy(frame,pVBufVirt,u32Size);
    HI_MPI_SYS_Munmap(pVBufVirt, u32Size);
    HI_MPI_VPSS_ReleaseChnFrame(0, VPSS_CHN0, &stFrame);
    return  frame;
}
/**
 * @description: 打开摄像头
 * @param {*}
 * @return {*}
 * @author: YURI
 */
int camera_gc2053 ::camera_open(void){
    if(open_flag==-1)printf("OPEN CAMERA ERROR ,PLEASE CHECK PARAM !!!\r\n");
    else{
        printf("OPENCV CAMERA SUCCESS \r\n");
        camera_alloc_buffer(0);
    } 
}
