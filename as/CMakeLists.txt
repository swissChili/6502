cmake_minimum_required(VERSION 3.0)

project(6502 VERSION 0.1.0 LANGUAGES C)

subdirs(libcollect)
include_directories(libcollect/include)

add_executable(6502-as main.c as.h as.c)
target_link_libraries(6502-as collect)
