/*
 * @Description: opencv转换接口
 * @Autor: YURI
 * @Date: 2022-01-24 06:16:21
 * @LastEditors: YURI
 * @LastEditTime: 2022-02-04 07:05:45
 */
#include "opencv_mat.h"

/**
 * @description: 将RGB颜色空间空间转换成Mat
 * @param {unsigned char*} rgbbuf
 * @return {*}
 * @author: YURI
 */
Mat opencv_mat::opencv_convert(unsigned char* rgbbuf)
{
    memcpy(res.data, rgbbuf, res.cols*res.rows*3);
    return res;
}
/**
 * @description: 初始换参数
 * @param {int} width
 * @param {int} height
 * @return {*}
 * @author: YURI
 */
opencv_mat::opencv_mat(int width,int height)
{
    res =Mat(height, width, CV_8UC3);
}
opencv_mat::~opencv_mat()
{
}