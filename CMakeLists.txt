cmake_minimum_required(VERSION 3.13)

# If do't define version number, specify the version number
set(VERSION "1.0.0" CACHE STRING "define project version")

# project 有两种写法，这里建议使用这种写法，补全信息有助于后续使用
project(psdk-cxx
    LANGUAGES CXX
    HOMEPAGE_URL https://github.com/Groveer/psdk-cxx
    DESCRIPTION "C++ Plugin Software Development Kit."
    VERSION ${VERSION})

# 定义GNU标准安装目录，使用此定义可兼容不同发行版之间的安装目录差异
include(GNUInstallDirs)
# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
# 设置为检查C++标准打开，若未设置CMAKE_CXX_STANDARD，则会报错
set(CMAKE_CXX_STANDARD_REQUIRED on)
# 设置包含当前目录，建议头文件的包含写全路径
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# 打开所有编译警告，理论上项目中不允许有编译警告
# 在外部添加编译参数的方法，例：cmake -DCMAKE_CXX_FLAGS="-Wl,--as-needed"
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
# 生成编译命令，用于支持clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 若未指定安装前缀，则指定前缀为/usr
# 注意大部分发行版默认会将前缀设置为/usr/local，所以若需要安装到/usr，还需在编译时指定
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

# 若未指定编译类型，则指定为Release，防止某些发行版不指定编译类型
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose Debug or Release" FORCE)
endif()

# 若当前编译类型为Debug，添加asan以检测内存泄漏
# 对应的还有CMAKE_CXX_FLAGS_RELEASE，Debug模式无需加-g参数，Release模式也无需加-O优化，默认cmake会帮我们加上
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")

# 某些平台可能需要手动指定mieee参数，否则会有浮点数精度问题，如sw_64
# 使用方式：cmake -DENABLE_MIEEE
if (DEFINED ENABLE_MIEEE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif()

# 若目标库安装了.cmake文件，则可以直接使用find_package
#find_package(PkgConfig REQUIRED)
# 若目标库未安装.cmake文件，但是安装了.pc文件，则可以使用pkgconfig
#pkg_search_module(XCB REQUIRED xcb)
find_package(spdlog REQUIRED)

# 建议对不同的模块进行分类，可使用通配符，可指定具名文件
file(GLOB_RECURSE SRCS
    "main.cpp"
)

set(BIN_NAME example)

# 生成可执行文件
add_executable(${BIN_NAME}
    ${SRCS})

# 安全编译选项，二进制需使用‘-fPIE’编译参数和‘-pie’链接参数，库需使用‘-fPIC’编译参数，两者之间有冲突不能同时使用，注意区分
target_compile_options(${BIN_NAME} PRIVATE -fPIE)
target_link_options(${BIN_NAME} PRIVATE -pie)

# 这里针对当前二进制定义了一个宏，保存着版本号，方便程序代码中调用，若程序中需要自定义宏变量并且在代码中使用，可参考此方法。
# 注意CMAKE_PROJECT_XXX系列变量，某些属性是cmake帮助提供，但某些属性是project方法设置的，所以这里需要注意变量是否可用。
# 至于是使用add_compile_xxx还是target_compile_xxx，视情况而定，一个是指定所有对象，一个是指定具体的二进制
target_compile_definitions(${BIN_NAME} PRIVATE
    VERSION="${CMAKE_PROJECT_VERSION}"
)

target_link_libraries(${BIN_NAME} PRIVATE
    spdlog::spdlog
)

# 指定安装目录，一般常用有3个： TARGETS（编译出来的目标二进制）、FILES（指定路径的文件，通常是配置文件或服务文件）、DIRECTORY（一般是配置文件较多时使用）。
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})


set(PLUGIN_NAME plugin)

add_library(${PLUGIN_NAME} MODULE plugin.cpp)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    spdlog::spdlog
)
