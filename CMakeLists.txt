# 指定CMake最低版本要求
cmake_minimum_required(VERSION 3.16)

# 设置项目名称、版本和使用的语言
project(AIGomokuGame VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准为C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用Qt的自动化工具
# AUTOMOC - 自动处理Qt的元对象系统
# AUTORCC - 自动处理Qt的资源文件
# AUTOUIC - 自动处理Qt的UI文件
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找并加载Qt6的Widgets模块
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# 添加可执行文件，并指定源文件
add_executable(AIGomokuGame
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/board.cpp
    src/board.h
    src/gamedialog.cpp
    src/gamedialog.h
)

# 链接Qt6::Widgets库
target_link_libraries(AIGomokuGame PRIVATE Qt6::Widgets) 