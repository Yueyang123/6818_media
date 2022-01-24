#ifndef OPENCV_CONVERT_MAT
#define OPENCV_CONVERT_MAT

#include <opencv2/opencv.hpp>
using namespace cv;
//提供常见Pix格式转化程标准RGB格式的MAT
class opencv_mat
{
private:
    Mat res;
public:
    opencv_mat(int width,int height);
    ~opencv_mat();
    Mat opencv_convert(unsigned char* rgbbuf);
};

#endif