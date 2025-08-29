cmake_minimum_required(VERSION 3.31)
project(cfox C)

set(CMAKE_C_STANDARD 23)

add_executable(cfox
        chunk.h
        chunk.c
        debug.h
        debug.c
        memory.h
        memory.c
		value.h
		value.c
		main.c
		vm.h
		vm.c
		common.h
		common.c
		compiler.h
		compiler.c
		scanner.h
		scanner.c
		object.h
		object.c
)
