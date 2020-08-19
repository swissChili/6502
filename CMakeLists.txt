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

find_library(readline SDL2 GL GLU GLEW m rt pthread)

add_executable(6502 main.c cpu.c cpu.h dbg.c dbg.h 
	instructions.h gui.h gui.c screen.h screen.c common.h common.c)
target_link_libraries(6502 readline SDL2 GL GLU GLEW m rt pthread)

install(TARGETS 6502)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "swissChili")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-dev, libreadline-dev, libglew-dev")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "6502 emulator, debugger, and assembler")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_FILE_NAME "6502")

include(CPack)
