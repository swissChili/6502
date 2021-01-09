cmake_minimum_required(VERSION 3.0)

project(6502 VERSION 0.1.0 LANGUAGES C)

include(TestBigEndian)
test_big_endian(BIG_ENDIAN)

option(VERBOSE OFF "Verbose assembler messages")

if (${BIG_ENDIAN})
	add_compile_definitions(BIG_ENDIAN)
else()
	add_compile_definitions(LITTLE_ENDIAN)
endif()

if(${VERBOSE})
	add_compile_definitions(VERBOSE_ASSEMBLER)
endif()


add_executable(6502-as main.c as.h as.c pp.c map.h map.c hash.c hash.c)

install(TARGETS 6502-as RUNTIME DESTINATION bin)
