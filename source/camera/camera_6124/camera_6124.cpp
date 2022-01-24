/*
 * @Description: 提供NVP6124 摄像头访问接口
 * @Autor: YURI
 * @Date: 2022-01-21 01:05:31
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-24 08:19:23
 */
#include "camera_6124.h"
#include <time.h>

void camera_6124::CHECK_COMMAND(int command) 
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
unsigned int camera_6124::yuv420m_get_size(int num, int iwidth, int iheight)
{
    int size;
    if (num == 0) {
        size = YUV_YSTRIDE(iwidth) * YUV_VSTRIDE(iheight);
    } else {
        size = YUV_STRIDE(iwidth/2) * YUV_VSTRIDE(iheight/2);
    }
    return size;
}
//YUV空间是分开的
int camera_6124::camera_alloc_buffer(int count)
{
    int ret;
    int i, j;
    struct nxp_vid_buffer *buffer;
    int plane_num=3;
    frame=(unsigned char*)malloc(width*height*2);
    for (i = 0; i < count; i++) {
        buffer = &bufs[i];
        //printf("[Buffer %d] --->\n", i);
        for (j = 0; j < plane_num; j++) {
            buffer->sizes[j] = yuv420m_get_size(j, width, height);
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
        }
    }
    return 0;
}

camera_6124::camera_6124(int width,int height, int piexlformat,int videoindex):
camera(width,height, piexlformat, videoindex)
{
    open_flag=0;
    printf("INIT CAMERA \r\n");
    if(piexlformat!=V4L2_PIX_FMT_YUV420M){
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
int camera_6124::camera_open(void){
    this->ion_fd = ion_open();
    this->clipper_id = nxp_v4l2_clipper0; //通道号
    this->sensor_id = nxp_v4l2_sensor0;   //传感器标号
    this->video_id = nxp_v4l2_mlc0_video; //屏幕标号
    this->piexlformat = V4L2_PIX_FMT_YUV420M;
    
    if (ion_fd < 0) {
        fprintf(stderr, "can't open ion!!!\n");
        return -1;
    }
    struct V4l2UsageScheme s;
    memset(&s, 0, sizeof(s));
    s.useClipper0 = true;
    s.useMlc0Video = true;
    CHECK_COMMAND(v4l2_init(&s));
    CHECK_COMMAND(v4l2_set_format(clipper_id, width, height, piexlformat));
    CHECK_COMMAND(v4l2_set_crop(clipper_id, 0, 0, width, height));
    //CHECK_COMMAND(v4l2_set_format(sensor_id, width+32, height+24, V4L2_MBUS_FMT_YUYV8_2X8));
    CHECK_COMMAND(v4l2_set_format(sensor_id, width, height, V4L2_MBUS_FMT_YUYV8_2X8));
    CHECK_COMMAND(v4l2_set_format(video_id, width, height, piexlformat));
	// setting destination position
    CHECK_COMMAND(v4l2_set_crop(video_id, 0, 0, width, height));
    // setting source crop
    CHECK_COMMAND(v4l2_set_crop_with_pad(video_id, 2, 0, 0, width, height)); //psw 20150331
    CHECK_COMMAND(v4l2_set_ctrl(video_id, V4L2_CID_MLC_VID_PRIORITY, 0));
    CHECK_COMMAND(v4l2_set_ctrl(video_id, V4L2_CID_MLC_VID_COLORKEY, 0x0));
    CHECK_COMMAND(v4l2_reqbuf(clipper_id, MAX_BUFFER_COUNT));
    CHECK_COMMAND(v4l2_reqbuf(video_id, MAX_BUFFER_COUNT));
    CHECK_COMMAND(camera_alloc_buffer(MAX_BUFFER_COUNT));
    printf("vid_buf: %p, %p, %p, %p\n", bufs[0].virt[0], bufs[1].virt[0], bufs[2].virt[0], bufs[3].virt[0]);
    for (int i = 0; i < MAX_BUFFER_COUNT; i++) {
        struct nxp_vid_buffer *buf = &bufs[i];
        printf("buf plane num: %d\n", buf->plane_num);
        CHECK_COMMAND(v4l2_qbuf(clipper_id, buf->plane_num, i, buf, -1, NULL));
    }
    CHECK_COMMAND(v4l2_streamon(clipper_id));	
}

unsigned char* camera_6124::read_frame()
{
    static int capture_index = 0;
    struct nxp_vid_buffer *buf = &bufs[capture_index];
    CHECK_COMMAND(v4l2_dqbuf(clipper_id, buf->plane_num, &capture_index, NULL));
    unsigned char *p=frame;
    memcpy(p,bufs[capture_index].virt[0] , bufs[capture_index].sizes[0]);
    p+=bufs[capture_index].sizes[0];
    memcpy(p,bufs[capture_index].virt[1] , bufs[capture_index].sizes[1]);
    p+=bufs[capture_index].sizes[1];
    memcpy(p,bufs[capture_index].virt[2] , bufs[capture_index].sizes[2]);
    p+=bufs[capture_index].sizes[2];
    CHECK_COMMAND(v4l2_qbuf(clipper_id, buf->plane_num, capture_index, buf, -1, NULL));
    return frame;
}

camera_6124::~camera_6124()
{
    CHECK_COMMAND(v4l2_streamoff(video_id));
    CHECK_COMMAND(v4l2_streamoff(clipper_id));
    free(frame);
    v4l2_exit();
    close(ion_fd);
}
