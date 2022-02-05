#include "framebuffer.h"

/**
 * @description: 解除对屏幕的使用
 * @param {*}
 * @return {*}
 * @author: YURI
 */
framebuffer::~framebuffer()
{
    munmap(fpb, vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8);  /* release the memory */
    close(fbfd);   
}
/**
 * @description: 初始化对屏幕的shiyong
 * @param {string} framedir
 * @return {*}
 * @author: YURI
 */
framebuffer::framebuffer(string framedir)
{
    int x = 0, y = 0;
    long int location = 0;
    fbfd = 0;
    fbfd = open(framedir.c_str(), O_RDWR);
    if (!fbfd) {
                printf("Error: cannot open framebuffer device.\n");
                exit(1);
        }
    printf("The framebuffer device was opened successfully.\n");
    /* Get fixed screen information */
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
            printf("Error reading fixed information.\n");
            exit(2);
    }
    /* Get variable screen information */
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("Error reading variable information.\n");
        exit(3);
    }
    this->width=vinfo.xres;
    this->height=vinfo.yres;
    this->dipalysize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    this->color_bytes=vinfo.bits_per_pixel / 8;
    fpb = (unsigned char *)mmap(0, dipalysize, PROT_READ|PROT_WRITE,MAP_SHARED,fbfd, 0);
    if ((int)fpb == -1)
    {
        printf("Error: failed to map framebuffer device to memory.\n"); exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");
    memset(fpb,0,dipalysize);
}

/**
 * @description: print_info打印屏幕信息
 * @param {*}
 * @return {*}
 * @author: YURI
 */
void framebuffer::print_info()
{
        printf("vinfo.xres_virtual : %d , vinfo.yres_virtual : %d\n",vinfo.xres_virtual, vinfo.yres_virtual);
        printf("vinfo.xres=%d\n",vinfo.xres);
        printf("vinfo.yres=%d\n",vinfo.yres);
        printf("vinfo.bits_per_bits=%d\n",vinfo.bits_per_pixel);
        printf("vinfo.xoffset=%d\n",vinfo.xoffset);
        printf("vinfo.yoffset=%d\n",vinfo.yoffset);
        printf("finfo.line_length=%d\n",finfo.line_length); 
        printf("color bytes=%d\n",color_bytes); 
}
/**
 * @description: 设置画笔颜色
 * @param {unsigned int} color
 * @return {*}
 * @author: YURI
 */
void framebuffer::set_color(unsigned int color)
{
    this->color=color;
}
/**
 * @description: 获取屏幕上的点对应的地址
 * @param {unsigned int} x
 * @param {unsigned int} y
 * @return {*}
 * @author: YURI
 */
unsigned char* framebuffer::at(unsigned int x, unsigned int y)
{
    long int location = 0;
    location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +(y+vinfo.yoffset) * finfo.line_length;
    return fpb + location ;
}
/**
 * @description: 画点
 * @param {unsigned int} x
 * @param {unsigned int} y
 * @param {unsigned int} color
 * @return {*}
 * @author: YURI
 */
void framebuffer::point(unsigned int x, unsigned int y,unsigned int color)
{
    if ((x < width) && (y < height))
	{
		if (color_bytes == 4)
		{
			((unsigned int*)fpb)[y * (width) + x] = color;
		}
		else//2 BYTE
		{
			((unsigned short*)fpb)[y * (width) + x] =
            (((((unsigned int)(color)) & 0xF8) >> 3) | ((((unsigned int)(color)) & 0xFC00) >> 5) | ((((unsigned int)(color)) & 0xF80000) >> 8));
		}
	}
}

/**
 * @description: 画线
 * @param {unsigned short} x1 x1,y1:起点坐标
 * @param {unsigned short} y1
 * @param {unsigned short} x2 x2,y2:终点坐标
 * @param {unsigned short} y2
 * @param {unsigned int} color
 * @return {*}
 * @author: YURI
 */
void framebuffer::line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2,unsigned int color)
{
    unsigned short t;
    int xerr=0,yerr=0,delta_x,delta_y,distance;
    int incx,incy,uRow,uCol;
    delta_x=x2-x1; //计算坐标增量
    delta_y=y2-y1;
    uRow=x1;
    uCol=y1;
    if(delta_x>0)incx=1; //设置单步方向
    else if(delta_x==0)incx=0;//垂直线
    else {incx=-1;delta_x=-delta_x;}
    if(delta_y>0)incy=1;
    else if(delta_y==0)incy=0;//水平线
    else{incy=-1;delta_y=-delta_y;}
    if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴
    else distance=delta_y;
    for(t=0;t<=distance+1;t++ )//画线输出
    {
        point(uRow, uCol,color);
        xerr+=delta_x ;
        yerr+=delta_y ;
        if(xerr>distance)
        {
            xerr-=distance;
            uRow+=incx;
        }
        if(yerr>distance)
        {
            yerr-=distance;
            uCol+=incy;
        }
    }
}
/**
 * @description: 画字符
 * @param {unsigned short} x
 * @param {unsigned short} y
 * @param {unsigned char} num
 * @param {unsigned char} size
 * @param {unsigned int} color
 * @return {*}
 * @author: YURI
 */
void framebuffer::cchar(unsigned short x,unsigned short y,unsigned char num,unsigned char size,unsigned int color)
{
    unsigned char temp,t1,t;
    unsigned short y0=y;
    unsigned char csize=(size/8+((size%8)?1:0))*(size/2);		//得到字体一个字符对应点阵集所占的字节数
    num=num-' ';//得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库）
    for(t=0;t<csize;t++)
    {
        if(size==12)temp=asc2_1206[num][t]; 	 	//调用1206字体
        else if(size==16)temp=asc2_1608[num][t];	//调用1608字体
        else if(size==24)temp=asc2_2412[num][t];	//调用2412字体
        else if(size==32)temp=asc2_3216[num][t];	//调用3216字体
        else return;								//没有的字库
        for(t1=0;t1<8;t1++)
        {
            if(temp&0x80)point(x,y,color);
            temp<<=1;
            y++;
            if(y>=height)return;		//超区域了
            if((y-y0)==size)
            {
                y=y0;
                x++;
                if(x>=width)return;	//超区域了
                break;
            }
        }
    }
}

/**
 * @description: 画⚪
 * @param {unsigned short} x0
 * @param {unsigned short} y0
 * @param {unsigned char} r
 * @param {unsigned int} color
 * @return {*}
 * @author: YURI
 */
void framebuffer::circle(unsigned short x0,unsigned short y0,unsigned char r,unsigned int color)
{
    int a,b;
    int di;
    a=0;b=r;
    di=3-(r<<1);             //判断下个点位置的标志
    while(a<=b)
    {
        point(x0+a, y0-b,color);
        point(x0+b, y0-a,color);
        point(x0+b, y0+a,color);
        point(x0+a, y0+b,color);
        point(x0-a, y0+b,color);
        point(x0-b, y0+a,color);
        point(x0-a, y0-b,color);
        point(x0-b, y0-a,color);
        a++;
        //使用Bresenham算法画圆
        if(di<0)di +=4*a+6;
        else
        {
            di+=10+4*(a-b);
            b--;
        }
    }
}

/**
 * @description: 画字符串
 * @param {unsigned short} x
 * @param {unsigned short} y
 * @param {unsigned short} width
 * @param {unsigned short} height
 * @param {unsigned char} size
 * @param {char*} p
 * @param {unsigned int} color
 * @return {*}
 * @author: YURI
 */
void framebuffer::sstring(unsigned short x,unsigned short y,unsigned short width,unsigned short height,unsigned char size,char* p,unsigned int color)
{
    unsigned char x0=x;
    width+=x;
    height+=y;
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//退出
        cchar(x, y, (unsigned char)*p,size,color);
        x+=size/2;
        p++;
    }
}
/**
 * @description: 在屏幕中显示RGB数据
 * @param {unsigned char *} rgbbuf
 * @param {int} pwidth
 * @param {int} pheight
 * @return {*}
 * @author: YURI
 */
void framebuffer::show_rgbbuffer(unsigned char * rgbbuf,int startx,int starty, int pwidth,int pheight)
{
    unsigned char r,g,b;
    unsigned char* q= rgbbuf;
    for(int y=starty;y<starty+pheight;y++)
        for(int x=startx;x<startx+pwidth;x++)
        {
            r=*q;
            g=*(q+1);
            b=*(q+2);
            ((unsigned int*)fpb)[y * (width) + x] = ((0xFF << 24) | (((unsigned int)(r)) << 16) | (((unsigned int)(g)) << 8) | ((unsigned int)(b)));
            q+=3;
        }
}