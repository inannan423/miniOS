cmake_minimum_required(VERSION 3.25)
project(miniOS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static")

link_libraries(ws2_32 wsock32)

add_executable(miniOS main.cpp os.h os.cpp fcb.cpp fcb.h user.h)
