cmake_minimum_required(VERSION 3.0)

project(6502 VERSION 0.1.0 LANGUAGES C)

option(GEN_INSTRUCTIONS_HEADER ON)

if (${GEN_INSTRUCTIONS_HEADER})
	add_custom_command(
		OUTPUT instructions.h
		DEPENDS csv2h.awk 6502.csv
		COMMAND awk -f csv2h.awk 6502.csv > instructions.h
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

add_executable(emu-6502 main.c cpu.c cpu.h dbg.c dbg.h instructions.h)
