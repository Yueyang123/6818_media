#include "opencv_mat.h"
Mat opencv_mat::opencv_convert(unsigned char* rgbbuf)
{
    memcpy(res.data, rgbbuf, res.cols*res.rows*3);
    return res;
}
opencv_mat::opencv_mat(int width,int height)
{
    res =Mat(height, width, CV_8UC3);
}
opencv_mat::~opencv_mat()
{
}