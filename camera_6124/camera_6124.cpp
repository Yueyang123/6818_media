#include "camera_6124.h"
#include <time.h>
void camera::CHECK_COMMAND(int command) 
{
     do { 
            int ret = command; 
            if (ret < 0) { 
                fprintf(stderr, "line %d error!!!\n", __LINE__); 
                return;
            } 
        } while (0);
}

//YUV420m 获取所需内存空间大小
unsigned int camera::yuv420m_get_size(int format, int num, int width, int height)
{
    int size;
    if (num == 0) {
        size = YUV_YSTRIDE(width) * YUV_VSTRIDE(height);
    } else {
        size = YUV_STRIDE(width/2) * YUV_VSTRIDE(height/2);
    }
    return size;
}
//YUV空间是分开的
int camera::yuv420m_alloc_buffers(int ion_fd, int count, struct nxp_vid_buffer *bufs, int width, int height, int format)
{
    int ret;
    int i, j;
    struct nxp_vid_buffer *buffer;
    int plane_num=3;
    for (i = 0; i < count; i++) {
        buffer = &bufs[i];
        //printf("[Buffer %d] --->\n", i);
        for (j = 0; j < plane_num; j++) {
            buffer->sizes[j] = yuv420m_get_size(format, j, width, height);
            ret = ion_alloc_fd(ion_fd, buffer->sizes[j], 0, ION_HEAP_NXP_CONTIG_MASK, 0, &buffer->fds[j]);
            if (ret < 0) {
                fprintf(stderr, "failed to ion_alloc_fd()\n");
                return ret;
            }
            buffer->virt[j] = (char *)mmap(NULL, buffer->sizes[j], PROT_READ | PROT_WRITE, MAP_SHARED, buffer->fds[j], 0);
            if (!buffer->virt[j]) {
                fprintf(stderr, "failed to mmap\n");
                return ret;
            }
            ret = ion_get_phys(ion_fd, buffer->fds[j], &buffer->phys[j]);
            if (ret < 0) {
                fprintf(stderr, "failed to get phys\n");
                return ret;
            }
            buffer->plane_num = plane_num;
            //printf("\tplane %d: fd(%d), size(%d), phys(0x%x), virt(0x%x)\n",
                    //j, buffer->fds[j], buffer->sizes[j], buffer->phys[j], buffer->virt[j]);
        }
    }
    return 0;
}

camera::camera(int width,int height)
{
    this->ion_fd = ion_open();
    this->clipper_id = nxp_v4l2_clipper0; //通道号
    this->sensor_id = nxp_v4l2_sensor0;   //传感器标号
    this->video_id = nxp_v4l2_mlc0_video; //屏幕标号
    this->width=width;
    this->height=height;
    this->format = V4L2_PIX_FMT_YUV420M;
    this->yuv420m_buf=(unsigned char*)malloc(width*height*2);
    if (ion_fd < 0) {
        fprintf(stderr, "can't open ion!!!\n");
        return ;
    }
    struct V4l2UsageScheme s;
    memset(&s, 0, sizeof(s));
    s.useClipper0 = true;
    s.useMlc0Video = true;
    CHECK_COMMAND(v4l2_init(&s));
    CHECK_COMMAND(v4l2_set_format(clipper_id, width, height, format));
    CHECK_COMMAND(v4l2_set_crop(clipper_id, 0, 0, width, height));
    //CHECK_COMMAND(v4l2_set_format(sensor_id, width+32, height+24, V4L2_MBUS_FMT_YUYV8_2X8));
    CHECK_COMMAND(v4l2_set_format(sensor_id, width, height, V4L2_MBUS_FMT_YUYV8_2X8));
    CHECK_COMMAND(v4l2_set_format(video_id, width, height, format));
	// setting destination position
    CHECK_COMMAND(v4l2_set_crop(video_id, 0, 0, width, height));
    // setting source crop
    CHECK_COMMAND(v4l2_set_crop_with_pad(video_id, 2, 0, 0, width, height)); //psw 20150331
    CHECK_COMMAND(v4l2_set_ctrl(video_id, V4L2_CID_MLC_VID_PRIORITY, 0));
    CHECK_COMMAND(v4l2_set_ctrl(video_id, V4L2_CID_MLC_VID_COLORKEY, 0x0));
    CHECK_COMMAND(v4l2_reqbuf(clipper_id, MAX_BUFFER_COUNT));
    CHECK_COMMAND(v4l2_reqbuf(video_id, MAX_BUFFER_COUNT));
    CHECK_COMMAND(yuv420m_alloc_buffers(ion_fd, MAX_BUFFER_COUNT, bufs, width, height, format));
    printf("vid_buf: %p, %p, %p, %p\n", bufs[0].virt[0], bufs[1].virt[0], bufs[2].virt[0], bufs[3].virt[0]);
    for (int i = 0; i < MAX_BUFFER_COUNT; i++) {
        struct nxp_vid_buffer *buf = &bufs[i];
        printf("buf plane num: %d\n", buf->plane_num);
        CHECK_COMMAND(v4l2_qbuf(clipper_id, buf->plane_num, i, buf, -1, NULL));
    }
    CHECK_COMMAND(v4l2_streamon(clipper_id));			
}


unsigned char*  camera::read_frame()
{
    clock_t start,finish;
    start=clock();
    static int capture_index = 0;
    struct nxp_vid_buffer *buf = &bufs[capture_index];
    CHECK_COMMAND(v4l2_dqbuf(clipper_id, buf->plane_num, &capture_index, NULL));
    unsigned char *p=yuv420m_buf;
    memcpy(p,bufs[capture_index].virt[0] , bufs[capture_index].sizes[0]);
    p+=bufs[capture_index].sizes[0];
    memcpy(p,bufs[capture_index].virt[1] , bufs[capture_index].sizes[1]);
    p+=bufs[capture_index].sizes[1];
    memcpy(p,bufs[capture_index].virt[2] , bufs[capture_index].sizes[2]);
    p+=bufs[capture_index].sizes[2];
    CHECK_COMMAND(v4l2_qbuf(clipper_id, buf->plane_num, capture_index, buf, -1, NULL));
    finish=clock();
    fps=1000/(double(finish-start)/CLOCKS_PER_SEC*1000);
    return yuv420m_buf;
}

void  camera::waitquit(string str)
{
    //wait for command
    fd_set fds;
    struct timeval tv;
    char cmd[10];
    int channel = 0;
    tv.tv_sec=0;
    tv.tv_usec=0;
    while (1)
    {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO,&fds);
        select(STDIN_FILENO+1,&fds,NULL,NULL,&tv);
        if(FD_ISSET(STDIN_FILENO,&fds)){
            memset(cmd,0,sizeof(cmd));
            read(STDIN_FILENO,cmd,256);
        if(strncmp(cmd,str.c_str(),str.length())==0){
                break;
            }
        }
    }
    

}

camera::~camera()
{
    CHECK_COMMAND(v4l2_streamoff(video_id));
    CHECK_COMMAND(v4l2_streamoff(clipper_id));
    free(yuv420m_buf);
    v4l2_exit();
    close(ion_fd);
}
