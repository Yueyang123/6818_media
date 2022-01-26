########################################################################
# Get Linux Build Enviornment:
include /home/swann/SDK/EXYNOS6818/NEWSOURCE/linux/linux/platform/s5p6818/build.env

######################################################################
# Build options
INCLUDE += -I./
INCLUDE += -I$(LIBSDIR)/include -I$(LIBSDIR)/include/theoraparser
LIBRARY	+= -L$(LIBSDIR)/lib -L$(LIBSDIR)/lib/ratecontrol 
LIBRARY += -lnxvpu -lnxdsp -lnxvip -lnxv4l2 -lnxvmem -lnxvidrc
LIBRARY += -ltheoraparser
LIBRARY	+=  -L/home/swann/SDK/EXYNOS6818/SDK/ffmpeg/lib -lavformat -lavdevice -lavcodec -lavutil -lavfilter -lpostproc -lswscale -lswresample -lstdc++ -lm

######################################################################
# Target
CPPOBJS	:= \
	MediaExtractor.o	\
	CodecInfo.o			\
	NX_Queue.o			\
	NX_Semaphore.o		\
	Util.o				\
	VpuDecTest.o		\
	VpuEncTest.o		\
	VpuJpgTest.o		\
	vpu.o		\
	main.o

CFLAGS	+= -g

TARGET:= codec_tests

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all:	\
	$(THEORAPARSER) \
	$(TARGET)

$(TARGET):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $@ $(LIBRARY)

clean:
	rm -f $(OBJS) $(TARGET) .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
