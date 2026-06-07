cmake_minimum_required(VERSION 3.20)
project(waivm VERSION 2.0.0 LANGUAGES C ASM_NASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    message(FATAL_ERROR "waivm v2 requires x86-64")
endif()

if(NOT UNIX OR APPLE)
    message(FATAL_ERROR "waivm v2 currently targets Linux x86-64")
endif()

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -Wall")

add_library(waicore STATIC
    src/bytecode.c
    src/instruction.c
    src/assembler.c
    src/disassembler.c
    src/debugger.c
    src/vm.c
    src/error.c
    asm/vm_linux_x86_64.asm
)

target_include_directories(waicore PUBLIC include)

target_compile_options(waicore PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Werror>
)

add_executable(waivm
    src/main.c
    src/cli.c
)

target_link_libraries(waivm PRIVATE waicore)
target_compile_options(waivm PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Werror>
)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
