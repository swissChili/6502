cmake_minimum_required(VERSION 3.0)

project(6502 VERSION 0.1.0 LANGUAGES C)

option(GEN_INSTRUCTIONS_HEADER ON)
option(BUILD_ASSEMBLER ON)

include_directories(nuklear)

include(TestBigEndian)
test_big_endian(BIG_ENDIAN)

if (${BIG_ENDIAN})
	add_compile_definitions(BIG_ENDIAN)
else()
	add_compile_definitions(LITTLE_ENDIAN)
endif()

subdirs(as)

if (${GEN_INSTRUCTIONS_HEADER})
	add_custom_command(
		OUTPUT instructions.h
		DEPENDS csv2h.awk 6502.csv
		COMMAND awk -f csv2h.awk 6502.csv > instructions.h
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

add_executable(6502 main.c cpu.c cpu.h dbg.c dbg.h 
	instructions.h gui.h gui.c screen.h screen.c common.h common.c)
target_link_libraries(6502 readline SDL2 GL GLU GLEW m rt pthread)
