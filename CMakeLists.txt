cmake_minimum_required(VERSION 3.25)
project(miniOS)

set(CMAKE_CXX_STANDARD 17)

add_executable(miniOS main.cpp os.h os.cpp DiskDrive.txt fcb.cpp fcb.h user.h)
