########################################################################
# Get Linux Build Enviornment:
ARCHNAME   	:= s5p6818
CROSSNAME	:= arm-cortex_a9-linux-gnueabi-
#换成KERNEL目录
KERNDIR := /home/swann/SDK/EXYNOS6818/NEWSOURCE/linux/linux/kernel/kernel-3.4.39
#换成NEXELL库目录
LIBSDIR	:= /home/swann/SDK/EXYNOS6818/NEWSOURCE/linux/linux/platform/s5p6818/library
#换成OPENCV库目录
OPENCVDIR:=/home/swann/SDK/EXYNOS6818/Driver/MEDIA/opencv
#换成FFMPEG库目录
FFMPEGDIR:=/home/swann/SDK/EXYNOS6818/Driver/MEDIA/ffmpeg
#########################################################################
#	Toolchain.
#########################################################################
CROSS 	 	:= $(CROSSNAME)
CC 		 	:= $(CROSS)gcc
CPP		 	:= $(CROSS)g++
AR 		 	:= $(CROSS)ar
AS			:= $(CROSS)as
LD 		 	:= $(CROSS)ld
NM 		 	:= $(CROSS)nm
RANLIB 	 	:= $(CROSS)ranlib
OBJCOPY	 	:= $(CROSS)objcopy
STRIP	 	:= $(CROSS)strip
#########################################################################
#	Library & Header macro
#########################################################################
INCLUDE :=-I$(KERNDIR)/arch/arm/mach-s5p6818/include/
INCLUDE += -I$(KERNDIR)/include
INCLUDE += -I$(LIBSDIR)/src/libion
INCLUDE += -I$(LIBSDIR)/src/libnxv4l2
INCLUDE += -I$(LIBSDIR)/include/theoraparser
INCLUDE += -I$(LIBSDIR)/include
INCLUDE += -I$(FFMPEGDIR)/include
INCLUDE += -I$(OPENCVDIR)/include
INCLUDE += -I./camera_6124
INCLUDE += -I./camera_uvc
INCLUDE += -I./ffmpeg/interface
INCLUDE += -I./opencv/interface
INCLUDE += -I./framebuffer

LIBRARY	+= -L$(LIBSDIR)/lib -L$(LIBSDIR)/lib/ratecontrol 
LIBRARY	+= -L$(LIBSDIR)/src/libion
LIBRARY	+= -L$(LIBSDIR)/src/libnxv4l2
LIBRARY += -lion -lv4l2-nexell
LIBRARY	+= -lm -lstdc++
LIBRARY	+= -lnxvpu -lnxdsp -lnxvip -lnxv4l2 -lnxvmem -lnxvidrc
LIBRARY	+= -ltheoraparser

LIBRARY += -L$(FFMPEGDIR)/lib  \
		-lavformat\
        -lavdevice\
        -lavcodec \
        -lavutil \
		-lavfilter	\
		-lpostproc\
		-lswscale \
        -lswresample\
		-lz -lm -lrt


LIBRARY += -L$(OPENCVDIR)/lib  -lopencv_imgcodecs -lopencv_dnn -lopencv_imgproc -lopencv_core 

#########################################################################
# 	Build Options
#########################################################################
OPTS		:= -Wall -O2 -Wextra -Wcast-align -Wno-unused-parameter -Wshadow -Wwrite-strings -Wcast-qual -fno-strict-aliasing -fstrict-overflow -fsigned-char -fno-omit-frame-pointer -fno-optimize-sibling-calls
COPTS 		:= $(OPTS)
CPPOPTS 	:= $(OPTS) -Wnon-virtual-dtor
CFLAGS 	 	:= $(COPTS)
CPPFLAGS 	:= $(CPPOPTS)
#########################################################################
# 	Generic Rules
#########################################################################
#底层文件
CPPSRCS +=./camera_6124/camera_6124.cpp
CPPSRCS +=./camera_uvc/camera_uvc.cpp
CPPSRCS +=./ffmpeg/interface/ffmpeg_sws.cpp
CPPSRCS +=./opencv/interface/opencv_mat.cpp
CPPSRCS +=./framebuffer/framebuffer.cpp
CCSRCS  :=
COBJS	:= $(CCSRCS:%.c=%.o)
CPPOBJS	:=  $(CPPSRCS:%.cpp=%.o)
OBJS    :=$(COBJS) $(CPPOBJS)
######################################################################
# 例程文件
CFLAGS	+= -g
FFMPEGAPP  := ./bin/vi_ffmpeg_opencv
FFMPEG_SRC :=./example/vi_ffmpeg_opencv.cpp 
FFMPEG_OBJ :=$(FFMPEG_SRC:%.cpp=%.o)
UVCAPP  := ./bin/vi_uvc
UVC_SRC :=./example/vi_uvc.cpp 
UVC_OBJ:=$(UVC_SRC:%.cpp=%.o)
FRAMEBUFFERAPP  := ./bin/framebuffer_uvc
FRAMEBUFFER_SRC :=./example/vi_framebuffer.cpp 
FRAMEBUFFER_OBJ:=$(FRAMEBUFFER_SRC:%.cpp=%.o)
######################################################################
# Build
all: ffmpeg_app uvc_app framebuffer_app
ffmpeg_app: $(FFMPEGAPP) 
uvc_app: 	$(UVCAPP)
framebuffer_app:$(FRAMEBUFFERAPP)
$(FFMPEGAPP): $(OBJS) $(FFMPEG_OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@ $(LIBRARY)
$(UVCAPP): $(OBJS) $(UVC_OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@ $(LIBRARY)
$(FRAMEBUFFERAPP): $(OBJS) $(FRAMEBUFFER_OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@ $(LIBRARY)
%.o	:  %.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDE) -c -o $@ $<  $(LIBRARY)
%.o	:  %.c
	$(CC) $(CPPFLAGS) $(INCLUDE) -c -o $@ $<  $(LIBRARY)
clean:
	rm  $(CAMAPP)  *.o  camera_6124/*.o  ffmpeg/interface/*.o opencv/interface/*.o
