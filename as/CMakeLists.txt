cmake_minimum_required(VERSION 3.0)

project(6502 VERSION 0.1.0 LANGUAGES C)

add_executable(6502-as main.c as.h as.c map.h map.c hash.c hash.c)
