cmake_minimum_required(VERSION 3.9.4)

# create project
project(microasm)
include_directories(src)

# debug mode preprocessor defenition
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug")
    add_compile_definitions(debug)
endif()

# list of source files
set(MICROCODE_FILES
    src/microcode/scanner.c
    src/microcode/token.c
    src/microcode/parser.c
    src/microcode/ast.c
    src/microcode/error.c
    src/microcode/test.c
    src/microcode/analyse.c
    src/microcode/codegen.c
)

set(SHARED_FILES
    src/shared/memory.c
    src/shared/platform.c
    src/shared/table.c
    src/shared/arg.c
    src/shared/graph.c
)

set(EMULATOR_FILES
    src/emulator/emu.c
    src/emulator/register.c
    src/emulator/vmcore.c
    src/emulator/memory64k.c
    src/emulator/instructionRegister.c
    src/emulator/emulator.c
)

add_executable(generator
    src/microcode/prebuild.c
    ${SHARED_FILES}
    ${MICROCODE_FILES}
)
target_link_libraries(generator m)
target_compile_options(generator PRIVATE -Wall -Wextra -pedantic -Werror)
set_property(TARGET generator PROPERTY C_STANDARD 99)

add_custom_command(
    OUTPUT switch.c
    COMMAND generator "${CMAKE_CURRENT_SOURCE_DIR}/test/sample.uasm" "${CMAKE_CURRENT_BINARY_DIR}/switch.c"
    DEPENDS generator "${CMAKE_CURRENT_SOURCE_DIR}/test/sample.uasm"
)

add_executable(microasm
    src/main.c
    switch.c
    ${SHARED_FILES}
    ${MICROCODE_FILES}
    ${EMULATOR_FILES}
)
target_link_libraries(microasm m)
target_compile_options(microasm PRIVATE -Wall -Wextra -pedantic -Werror)
set_property(TARGET microasm PROPERTY C_STANDARD 99)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error)
    if( supported )
        message(STATUS "IPO / LTO enabled")
        set_property(TARGET microasm PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        SET(CMAKE_AR  "gcc-ar")
        SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
        SET(CMAKE_C_ARCHIVE_FINISH   true)
    else()
        message(STATUS "IPO / LTO not supported: <${error}>")
    endif()
endif()