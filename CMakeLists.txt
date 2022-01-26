cmake_minimum_required(VERSION 3.2)
project(MUTIL_MEDIA)
#禁用WARNING
add_definitions(-w)
#设置头文件目录
include_directories (source/camera/camera_uvc)
include_directories (source/camera)
include_directories (source/codec/encode)

include_directories (source/3rd_interface/ffmpeg)
include_directories (source/3rd_interface/opencv)
include_directories (source/framebuffer)
#设置应用程序源码
set (VI_FRAMEBUFFER_APP source/example/vi_uvc_framebuffer.cpp )
set (VI_MUTILCHANNEL_APP source/example/vi_multichannel.cpp )
set (VPSS_ENCODE_APP source/example/vpss_encode.cpp )
#设置程序输出路径
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

aux_source_directory(source/camera/camera_uvc    BASE1_SRC)  
aux_source_directory(source/framebuffer          BASE2_SRC)
aux_source_directory(source/3rd_interface/opencv BASE3_SRC)
aux_source_directory(source/3rd_interface/ffmpeg BASE4_SRC)

option(HI3516 "USE HI3516 LIB" OFF)
option(S5P6818 "USE S5P6818 LIB" ON)
if (HI3516)
    add_definitions(-DHI3516)
endif()
if (S5P6818)
    add_definitions(-DS5P6818)
endif()

if(HI3516)
    #添加环境所需的头文件
    MESSAGE(USE_HI3516_LIB)
    
    #设置两个库所在位置
    set(FFMPEGDIR /home/swann/SDK/HI3516/SDK/ffmpeg)
    set(OPENCVDIR /home/swann/SDK/HI3516/SDK/opencv/INSTALL)
    list(APPEND FFmpeg_LIBS  avformat avdevice avcodec avutil avfilter postproc swscale swresample)
    add_link_options(-L${FFMPEGDIR}/lib -lpthread)
    list(APPEND OpenCV_LIBS     opencv_imgproc opencv_core )
    add_link_options(-L${OPENCVDIR}/lib)
    include_directories (${FFMPEGDIR}/include)
    include_directories (${OPENCVDIR}/include)

    #添加平台摄像头文件夹
    aux_source_directory(source/camera/camera_gc2053   PLATFORM_CAMERA)  
    include_directories (source/camera/camera_gc2053)


    set (MPPDIR /home/swann/SDK/HI3516/MYMMP/mpp)
    set (COMMON_DIR   ${MPPDIR}/sample/common)
    set (HISILIB      ${MPPDIR}/lib)
    aux_source_directory(${COMMON_DIR} COMMON_SRC)
    include_directories (${MPPDIR}/include)
    include_directories (${MPPDIR}/sample/audio/adp)
    include_directories (${MPPDIR}/sample/common)
    include_directories (${MPPDIR}/sample/vio)
    
    list(APPEND HISI_LIBS  isp mpi hdmi sns_gc2053 dnvqe VoiceEngine upvqe securec securec
    sns_imx327 sns_imx327_2l sns_imx307 sns_imx307_2l sns_imx458 sns_mn34220
    sns_os05a sns_os08a10 sns_sc4210 sns_ov12870 sns_os04b10 sns_imx415 
    sns_imx219 sns_imx274 sns_imx335 _hiae _hidehaze _hildci _hiawb _hidrc _hicalcflicker)
    add_link_options(-L${HISILIB})


    add_compile_options(
    -lpthread -lm -ldl -DISP_V2 -lstdc++ -Wall -mcpu=cortex-a7 -mfloat-abi=softfp 
    -mfpu=neon-vfpv4 -fno-aggressive-loop-optimizations -ldl -ffunction-sections -fdata-sections 
    -O2 -fstack-protector-strong -fPIC -Wall  -DHI_RELEASE -Wno-error=implicit-function-declaration 
    -DVER_X=1 -DVER_Y=0 -DVER_Z=0 -DVER_P=0 -DVER_B=10 -DUSER_BIT_32 -DKERNEL_BIT_32 -Wno-date-time	
    -DSENSOR0_TYPE=GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT 
    -DSENSOR1_TYPE=GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT	
    -DHI_ACODEC_TYPE_INNER -DHI_ACODEC_TYPE_HDMI 
    -Wl,--start-group 	
	${HISILIB}/libmpi.a		 	${HISILIB}/libhdmi.a 
	${HISILIB}/lib_hiae.a 	 	${HISILIB}/libisp.a 
	${HISILIB}/lib_hidehaze.a 	${HISILIB}/lib_hidrc.a 
	${HISILIB}/lib_hildci.a 	${HISILIB}/lib_hicalcflicker.a 
	${HISILIB}/lib_hiawb.a 	 	${HISILIB}/libsns_gc2053.a  
	${HISILIB}/libVoiceEngine.a ${HISILIB}/libupvqe.a 
	${HISILIB}/libdnvqe.a		${HISILIB}/libsecurec.a 
	${HISILIB}/libsns_imx327.a  ${HISILIB}/libsns_imx327_2l.a 
	${HISILIB}/libsns_imx307.a  ${HISILIB}/libsns_imx307_2l.a 
	${HISILIB}/libsns_imx458.a  ${HISILIB}/libsns_mn34220.a 
	${HISILIB}/libsns_os05a.a   ${HISILIB}/libsns_os08a10.a 
	${HISILIB}/libsns_sc4210.a  ${HISILIB}/libsns_ov12870.a 
	${HISILIB}/libsns_os04b10.a ${HISILIB}/libsns_imx415.a 
	${HISILIB}/libsns_imx219.a  ${HISILIB}/libsns_imx274.a 
	${HISILIB}/libsns_imx335.a 
	-Wl,--end-group) 
    set(CMAKE_C_COMPILER "arm-himix200-linux-gcc")
    set(CMAKE_CXX_COMPILER "arm-himix200-linux-g++")

    add_executable(vi_multichannel  ${COMMON_SRC} ${PLATFORM_CAMERA} ${BASE1_SRC} ${BASE2_SRC} ${BASE3_SRC} ${BASE4_SRC} ${VI_MUTILCHANNEL_APP})
    target_link_libraries(vi_multichannel ${OpenCV_LIBS})
    target_link_libraries(vi_multichannel ${FFmpeg_LIBS})
    target_link_libraries(vi_multichannel ${HISI_LIBS} dl)

    add_executable(vi_framebuffer  ${BASE1_SRC} ${BASE2_SRC} ${BASE3_SRC} ${BASE4_SRC} ${VI_FRAMEBUFFER_APP})
    target_link_libraries(vi_framebuffer ${OpenCV_LIBS})
    target_link_libraries(vi_framebuffer ${FFmpeg_LIBS})

elseif(S5P6818)
    #添加环境所需的头文件
    MESSAGE(USE_S5P6818_LIB)
    set(FFMPEGDIR /home/swann/SDK/EXYNOS6818/SDK/ffmpeg)
    set(OPENCVDIR /home/swann/SDK/EXYNOS6818/SDK/opencv/install)
    
    list(APPEND FFmpeg_LIBS  avformat avdevice avcodec avutil avfilter postproc swscale swresample)
    add_link_options(-L${FFMPEGDIR}/lib -lpthread)
    list(APPEND OpenCV_LIBS   opencv_imgcodecs opencv_dnn opencv_imgproc opencv_core )
    add_link_options(-L${OPENCVDIR}/lib)
    include_directories (${FFMPEGDIR}/include)
    include_directories (${OPENCVDIR}/include)

    #添加平台摄像头文件夹
    aux_source_directory(source/camera/camera_6124   PLATFORM_CAMERA)  
    include_directories (source/camera/camera_6124)

    #添加平台编解码器文件夹
    aux_source_directory(source/codec/encode/encode_6818   PLATFROM_ENCODE)  
    include_directories (source/codec/encode/encode_6818)

    #写入platform文件夹中nexell提供的库
    set (NXPLIBSDIR   /home/swann/SDK/EXYNOS6818/NEWSOURCE/linux/linux/platform/s5p6818/library)
    #写入内核所在位置
    set (KERNDIR      /home/swann/SDK/EXYNOS6818/NEWSOURCE/linux/linux/kernel/kernel-3.4.39)
    include_directories (${KERNDIR}/arch/arm/mach-s5p6818/include)
    include_directories (${KERNDIR}/include)
    include_directories (${NXPLIBSDIR}/src/libion)
    include_directories (${NXPLIBSDIR}/src/libnxv4l2)
    include_directories (${NXPLIBSDIR}/include/theoraparser)
    include_directories (${NXPLIBSDIR}/include)
    add_compile_options(-lz -lm -lrt -Wall -O2 -Wextra -Wcast-align -Wno-unused-parameter -Wshadow -Wwrite-strings -Wcast-qual -fno-strict-aliasing -fstrict-overflow -fsigned-char -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wnon-virtual-dtor)
    set(CMAKE_C_COMPILER "arm-cortex_a9-linux-gnueabi-gcc")
    set(CMAKE_CXX_COMPILER "arm-cortex_a9-linux-gnueabi-g++")
    
    list(APPEND NXP_LIBS  ion v4l2-nexell m stdc++ nxvpu
    nxdsp nxvip nxv4l2 nxvmem nxvidrc theoraparser
    )
    add_link_options(-L${NXPLIBSDIR}/lib 
    -L${NXPLIBSDIR}/lib/ratecontrol
    -L${NXPLIBSDIR}/src/libion
    -L${NXPLIBSDIR}/src/libnxv4l2
    )
    
    add_executable(vpss_encode ${PLATFORM_CAMERA} ${PLATFROM_ENCODE} ${BASE1_SRC} ${BASE2_SRC} ${BASE3_SRC} ${BASE4_SRC} ${VPSS_ENCODE_APP})
    target_link_libraries(vpss_encode ${OpenCV_LIBS})
    target_link_libraries(vpss_encode ${FFmpeg_LIBS})
    target_link_libraries(vpss_encode ${NXP_LIBS})

    add_executable(vi_multichannel ${PLATFORM_CAMERA} ${BASE1_SRC} ${BASE2_SRC} ${BASE3_SRC} ${BASE4_SRC} ${VI_MUTILCHANNEL_APP})
    target_link_libraries(vi_multichannel ${OpenCV_LIBS})
    target_link_libraries(vi_multichannel ${FFmpeg_LIBS})
    target_link_libraries(vi_multichannel ${NXP_LIBS} dl)

    add_executable(vi_framebuffer  ${BASE1_SRC} ${BASE2_SRC} ${BASE3_SRC} ${BASE4_SRC} ${VI_FRAMEBUFFER_APP})
    target_link_libraries(vi_framebuffer ${OpenCV_LIBS})
    target_link_libraries(vi_framebuffer ${FFmpeg_LIBS})

endif()






