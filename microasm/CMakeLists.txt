cmake_minimum_required(VERSION 3.9.4)

project(microasm)

include_directories(src/include)

add_executable(microasm
    src/main.c
    src/scanner.c
    src/token.c
    src/parser.c
    src/ast.c
    src/memory.c
    src/platform.c
    src/error.c
    src/test.c
    src/analyse.c
    )

set_property(TARGET microasm PROPERTY C_STANDARD 99)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug")
    add_compile_definitions(debug)
else()
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

target_link_libraries(microasm m)

if(MSVC)
    target_compile_options(microasm PRIVATE /W4 /WX)
else()
    target_compile_options(microasm PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
