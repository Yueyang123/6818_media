YUV是视频、图片、相机等应用中使用的一类图像格式，实际上是所有“YUV”像素格式共有的颜色空间的名称。 与RGB格式（红 - 绿 - 蓝）不同，YUV是用一个称为Y（相当于灰度）的“亮度”分量和两个“色度”分量表示，分别称为U（蓝色投影）和V（红色投影），由此得名。

YUV也可以称为YCbCr，虽然这些术语意味着略有不同，但它们往往会混淆并可互换使用。

    Y表示亮度分量：如果只显示Y的话，图像看起来会是一张黑白照。

    U（Cb）表示色度分量：是照片蓝色部分去掉亮度（Y）。

    V（Cr）表示色度分量：是照片红色部分去掉亮度（Y）。

要说清楚YUV，得分别说清楚以下两点：

    YUV的采样格式：即我们在采集图片、视频帧时，是如何获取每个像素的Y、U、V三个分量的。
    YUV的存储格式：即Y、U、V三个分量的值，是以什么方式存储在内存或者文件中的。

U(CB) V(CR)
分量一起存储模式
YUYV(相邻两个像素点共用一对UV)
Y U Y V
Y U Y V
Y U Y V
Y U Y V



分量单独存储模式
NV21 的存储格式是，以4 X 4 图片为例子

Y Y Y Y
Y Y Y Y
Y Y Y Y
Y Y Y Y
V U V U
V U V U

NV12 的存储格式是
Y Y Y Y
Y Y Y Y
Y Y Y Y
Y Y Y Y
U V U V
U V U V

IYUV (I420 YUV420P)的存储格式是
Y Y Y Y
Y Y Y Y
Y Y Y Y
Y Y Y Y
U U U U
V V V V

YV12
Y Y Y Y
Y Y Y Y
Y Y Y Y
Y Y Y Y
V V V V
U U U U
