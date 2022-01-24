这个软件目前适用于6818开发板 HI3516开发板
文件系中需要准备的库有QT OPENCV FFMPEG  文件系统已经打包，

该程序目的是屏蔽不同平台在视频输入，视频处理，图像显示等方面的
平台差异提供方便移植的统一接口

# VI摄像接口

```cpp
class camera
{
public:
    //返回的图像数据
    unsigned char* frame;
    double fps;
    int width,height;
    //相机编号 
    int videoindex;
    //像素格式引用FFMPEG对图象的定义方式
    int piexlformat;
    clock_t start,finish;
public:
    camera(int width,int height, int piexlformat,int videoindex)
    {
        this->width=width;
        this->height=height;
        this->piexlformat=piexlformat;
        this->videoindex=videoindex;
    }
    //虚类需要虚析构函数，防止内存泄漏
    virtual ~camera(){};

    virtual int camera_alloc_buffer(int count)= 0;
    virtual unsigned char* read_frame (void)= 0;
    virtual int camera_open(void)= 0;
    /**
     * @description: 计算帧率使用的函数
     * 在做所有任务之前运行一次（参数0），所有任务完成之后
     * 再运行一次（参数1）
     * @param {int} index
     * @return {*}
     * @author: YURI
     */    
    double camera_get_fps(int index){
        if(index==0){
            start=clock();
            return 0;
        }
        else{
            finish=clock();
            fps=1000/((double)(finish-start)/CLOCKS_PER_SEC*1000);
            return fps;
        }
    }
};
```
摄像接口如上 
调用顺序
camera -》camera_open-》read_frame


# VO Framebuffer接口
```cpp
class framebuffer 
{
private:
    fb_fix_screeninfo finfo;
    fb_var_screeninfo vinfo;
    unsigned char* fpb;     //显存映射到用户空间
    int fbfd;               //打开屏幕的描述符
    int color_bytes;        //一个像素点几个字节
    int dipalysize;         //显存大小
public:
    unsigned int color,width,height;
    framebuffer(string framedir);
    ~framebuffer();
    void print_info();
    void set_color(unsigned int color);
    void point(unsigned int x, unsigned int y,unsigned int color=0);
    void cchar(unsigned short x,unsigned short y,unsigned char num,unsigned char size,unsigned int color=0);
    void line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2,unsigned int color=0);
    void circle(unsigned short x0,unsigned short y0,unsigned char r,unsigned int color=0);
    void sstring(unsigned short x,unsigned short y,unsigned short width,unsigned short height,unsigned char size,char* str,unsigned int color=0);
    unsigned char* at(unsigned int x, unsigned int y);
    void show_rgbbuffer(unsigned char * rgbbuf,int startx,int starty, int pwidth,int pheight);

};
```

framebuffer初始化后就可以直接绘图
支持双字节，和四字节的fb设备

