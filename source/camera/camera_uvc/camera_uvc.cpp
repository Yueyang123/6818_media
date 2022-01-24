/*
 * @Description: 提供UVC 摄像头访问接口
 * @Autor: YURI
 * @Date: 2022-01-21 01:05:31
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-24 01:39:21
 */
#include "camera_uvc.h"
#include "time.h"
int camera_uvc::camera_get_capability(void)
{
    int ret=0;
    ret=ioctl (video_fd, VIDIOC_QUERYCAP, &cap);
    if(ret<0) printf("failture VIDIOC_QUERYCAP\n");
    return ret;
}
void camera_uvc::camera_show_capability(void)
{
    printf("摄像头基本信息:\n");
    printf(" driver: %s\n", cap.driver);
    printf(" card: %s\n", cap.card);
    printf(" bus_info: %s\n", cap.bus_info);
    printf(" version: %08X\n", cap.version);
    printf(" capabilities: %08X\n", cap.capabilities);       
}
int camera_uvc::camera_get_format(void)
{
    int ret=0;
    int index = 0;
    memset(&fmt1, 0, sizeof(fmt1));
    fmt1.index = 0;
    fmt1.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    while ((ret = ioctl(video_fd, VIDIOC_ENUM_FMT, &fmt1)) == 0)
    {
        fmt1.index++;
        printf("pixelformat = '%c%c%c%c', description = '%s'\n",
        (fmt1.pixelformat >> 0) & 0xFF, 
        (fmt1.pixelformat >> 8) & 0xFF,
        (fmt1.pixelformat >> 16) & 0xFF, 
        (fmt1.pixelformat >> 24) & 0xFF,
        fmt1.description);

        printf("像素格式 = '%c%c%c%c', 格式表述 = '%s'\n",
        (fmt1.pixelformat >> 0) & 0xFF, 
        (fmt1.pixelformat >> 8) & 0xFF,
        (fmt1.pixelformat >> 16) & 0xFF, 
        (fmt1.pixelformat >> 24) & 0xFF,
        fmt1.description);
    }
    return ret;
}
int camera_uvc::camera_set_format(int piexlformat,int width,int height)
{
    int ret=0;
    memset (&(select_fmt), 0, sizeof (select_fmt));
    select_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    select_fmt.fmt.pix.width = width;
    select_fmt.fmt.pix.height = height;
    select_fmt.fmt.pix.pixelformat = piexlformat;//V4L2_PIX_FMT_JPEG;
    select_fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
 
    ret = ioctl (video_fd, VIDIOC_S_FMT, &select_fmt); 
    if(ret<0)
        printf("failture VIDIOC_S_FMT\n");
    return ret;
}
int camera_uvc::camera_alloc_buffer(int count)
{
    int ret=0;
    struct v4l2_buffer buf;
    memset (&(req), 0, sizeof (req));
    frame=(unsigned char *)malloc(width*height*3);
    if(frame==NULL)printf("GET MEMORY ERROR \r\n");
    req.count = count;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    ret = ioctl (video_fd, VIDIOC_REQBUFS, &req); 
    if(ret<0)
        printf("failture VIDIOC_REQBUFS\n");
   if (req.count < 1)
        printf("Insufficient buffer memory\n");
    buffers = (struct buffer*)calloc (req.count, sizeof (*buffers));
    //申请四块空间
    for (int i=0; i < req.count; ++i)
    { 
        memset (&(buf), 0, sizeof (buf)); 
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (-1 == ioctl (video_fd, VIDIOC_QUERYBUF, &buf)) 
                printf ("VIDIOC_QUERYBUF error\n");
        buffers[i].length = buf.length;
        buffers[i].start = mmap (NULL ,buf.length,PROT_READ | PROT_WRITE ,MAP_SHARED ,video_fd, buf.m.offset);
        if (MAP_FAILED == buffers[i].start)
            printf ("mmap failed\n");
     }
    //入队四块空间
    for (int i = 0; i < req.count; ++i)
    {
        memset (&(buf), 0, sizeof (buf)); 
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (-1 == ioctl (video_fd, VIDIOC_QBUF, &buf))
            printf ("VIDIOC_QBUF failed\n");
    }
}
int camera_uvc::camera_open(void)
{
    char buf[20];
    sprintf(buf,"/dev/video%d",videoindex);
    video_fd = open (buf, O_RDWR | O_NONBLOCK, 0);
    //获取摄像头基本信息
    camera_get_capability();
    //显示摄像头基本信息  
    camera_show_capability();
    //获取摄像头格式信息
    camera_get_format();
    //设置像素格式
    camera_set_format(piexlformat,width,height);
    //申请四个队列空间
    camera_alloc_buffer(4);
    //打开视频采集流
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl (video_fd, VIDIOC_STREAMON, &type)) printf ("VIDIOC_STREAMON failed\n");
    return 1;
}

camera_uvc::camera_uvc(int width,int height, int piexlformat,int videoindex):
camera(width,height, piexlformat, videoindex)
{
}

camera_uvc::~camera_uvc()
{
    for (int i = 0; i < 4; ++i)
       if (-1 == munmap (buffers->start, buffers->length))
            printf ("munmap error");
 
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    if (-1 == ioctl(video_fd, VIDIOC_STREAMOFF, &type)) 
        printf("VIDIOC_STREAMOFF");
    close (video_fd);
}


unsigned char* camera_uvc::read_frame (void)
{
    clock_t start,finish;
    start=clock();
    int ret=0;
    struct v4l2_buffer buf;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    fd_set fds;
    struct timeval tv;
    int r;
    tv.tv_sec = 2;
    tv.tv_usec = 0; 
    FD_ZERO (&fds);
    FD_SET (video_fd, &fds);
    r = select (video_fd + 1, &fds, NULL, NULL, &tv);
    if (-1 == r) 
    {       
        if (EINTR == errno)
            printf ("select err\n");
    }
    if (0 == r) 
    {
        fprintf (stderr, "select timeout\n");
        exit (EXIT_FAILURE);
    }
    ret = ioctl (video_fd, VIDIOC_DQBUF, &buf);
    if(ret<0)printf("failture\n"); 
    memcpy(frame,buffers[buf.index].start,buffers[buf.index].length);
    ret=ioctl (video_fd, VIDIOC_QBUF, &buf); 
    if(ret<0) printf("failture VIDIOC_QBUF\n");
    finish=clock();
    fps=1000/(double(finish-start)/CLOCKS_PER_SEC*1000);
    return frame;
}