cmake_minimum_required(VERSION 3.0)

project(6502 VERSION 0.1.0 LANGUAGES C)

include(TestBigEndian)
test_big_endian(BIG_ENDIAN)


if (${BIG_ENDIAN})
	add_compile_definitions(BIG_ENDIAN)
else()
	add_compile_definitions(LITTLE_ENDIAN)
endif()


add_executable(6502-as main.c as.h as.c map.h map.c hash.c hash.c)

install(TARGETS 6502-as)
