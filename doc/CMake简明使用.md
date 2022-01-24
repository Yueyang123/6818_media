# 同一目录
接下来进入稍微复杂的例子：在同一个目录下有多个源文件。
在之前的目录下添加2个文件，testFunc.c和testFunc.h。

修改CMakeLists.txt，在add_executable的参数里把testFunc.c加进来

cmake_minimum_required (VERSION 2.8)

project (demo)

add_executable(main main.c testFunc.c)

可以类推，如果在同一目录下有多个源文件，那么只要在add_executable里把所有源文件都添加进去就可以了。但是如果有一百个源文件，再这样做就有点坑了，无法体现cmake的优越性，cmake提供了一个命令可以把指定目录下所有的源文件存储在一个变量中，这个命令就是 aux_source_directory(dir var)。
第一个参数dir是指定目录，第二个参数var是用于存放源文件列表的变量

修改CMakeLists.txt，

cmake_minimum_required (VERSION 2.8)
project (demo)
aux_source_directory(. SRC_LIST)
add_executable(main ${SRC_LIST})

aux_source_directory()也存在弊端，它会把指定目录下的所有源文件都加进来，可能会加入一些我们不需要的文件，此时我们可以使用set命令去新建变量来存放需要的源文件，如下，
cmake_minimum_required (VERSION 2.8)

project (demo)

set( SRC_LIST
	 ./main.c
	 ./testFunc1.c
	 ./testFunc.c)

add_executable(main ${SRC_LIST})

# 不同目录

cmake_minimum_required (VERSION 2.8)

project (demo)

include_directories (test_func test_func1)

aux_source_directory (test_func SRC_LIST)
aux_source_directory (test_func1 SRC_LIST1)

add_executable (main main.c ${SRC_LIST} ${SRC_LIST1})


这里出现一个新的命令add_subdirectory()，这个命令可以向当前工程添加存放源文件的子目录，并可以指定中间二进制和目标二进制的存放位置，具体用法可以百度。
这里指定src目录下存放了源文件，当执行cmake时，就会进入src目录下去找src目录下的CMakeLists.txt，所以在src目录下也建立一个CMakeLists.txt，内容如下，

aux_source_directory (. SRC_LIST)

include_directories (../include)

add_executable (main ${SRC_LIST})

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
这里又出现一个新的命令set，是用于定义变量的，EXECUTABLE_OUT_PATH和PROJECT_SOURCE_DIR是CMake自带的预定义变量，其意义如下，

    EXECUTABLE_OUTPUT_PATH ：目标二进制可执行文件的存放位置
    PROJECT_SOURCE_DIR：工程的根目录


# 动态库和静态库的编译控制

有时只需要编译出动态库和静态库，然后等着让其它程序去使用。让我们看下这种情况该如何使用cmake。首先按照如下重新组织文件，只留下testFunc.h和TestFunc.c，

我们会在build目录下运行cmake，并把生成的库文件存放到lib目录下。
CMakeLists.txt内容如下，

cmake_minimum_required (VERSION 3.5)

project (demo)

set (SRC_LIST ${PROJECT_SOURCE_DIR}/testFunc/testFunc.c)

add_library (testFunc_shared SHARED ${SRC_LIST})
add_library (testFunc_static STATIC ${SRC_LIST})

set_target_properties (testFunc_shared PROPERTIES OUTPUT_NAME "testFunc")
set_target_properties (testFunc_static PROPERTIES OUTPUT_NAME "testFunc")

set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

这里又出现了新的命令和预定义变量，

    add_library: 生成动态库或静态库(第1个参数指定库的名字；第2个参数决定是动态还是静态，如果没有就默认静态；第3个参数指定生成库的源文件)
    set_target_properties: 设置最终生成的库的名称，还有其它功能，如设置库的版本号等等
    LIBRARY_OUTPUT_PATH: 库文件的默认输出路径，这里设置为工程目录下的lib目录

# 对库进行链接

既然我们已经生成了库，那么就进行链接测试下。重新建一个工程目录，然后把上节生成的库拷贝过来，然后在在工程目录下新建src目录和bin目录，在src目录下添加一个main.c，整体结构如下，

cmake_minimum_required (VERSION 3.5)

project (demo)


set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set (SRC_LIST ${PROJECT_SOURCE_DIR}/src/main.c)

# find testFunc.h
include_directories (${PROJECT_SOURCE_DIR}/testFunc/inc)

find_library(TESTFUNC_LIB testFunc HINTS ${PROJECT_SOURCE_DIR}/testFunc/lib)

add_executable (main ${SRC_LIST})

target_link_libraries (main ${TESTFUNC_LIB})
这里出现2个新的命令，

    find_library: 在指定目录下查找指定库，并把库的绝对路径存放到变量里，其第一个参数是变量名称，第二个参数是库名称，第三个参数是HINTS，第4个参数是路径，其它用法可以参考cmake文档
    target_link_libraries: 把目标文件与库文件进行链接

# 添加编译选项

有时编译程序时想添加一些编译选项，如-Wall，-std=c++11等，就可以使用add_compile_options来进行操作。

cmake_minimum_required (VERSION 2.8)

project (demo)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_compile_options(-std=c++11 -Wall) 

add_executable(main main.cpp)

# 添加控制选项

有时希望在编译代码时只编译一些指定的源码，可以使用cmake的option命令，主要遇到的情况分为2种：

    本来要生成多个bin或库文件，现在只想生成部分指定的bin或库文件
    对于同一个bin文件，只想编译其中部分代码（使用宏来控制）
cmake_minimum_required(VERSION 3.5)

project(demo)

option(MYDEBUG "enable debug compilation" OFF)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src)

cmake_minimum_required (VERSION 3.5)
add_executable(main1 main1.c)
if (MYDEBUG)
    add_executable(main2 main2.c)
else()
    message(STATUS "Currently is not in debug mode")    
endif()
注意，这里使用了if-else来根据option来决定是否编译main2.c

假设我们有个main.c，其内容如下，

#include <stdio.h>

int main(void)
{
#ifdef WWW1
    printf("hello world1\n");
#endif    

#ifdef WWW2     
    printf("hello world2\n");
#endif

    return 0;
}
可以通过定义宏来控制打印的信息，我们CMakeLists.txt内容如下，

cmake_minimum_required(VERSION 3.5)

project(demo)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

option(WWW1 "print one message" OFF)
option(WWW2 "print another message" OFF)

if (WWW1)
    add_definitions(-DWWW1)
endif()

if (WWW2)
    add_definitions(-DWWW2)
endif()

add_executable(main main.c)
