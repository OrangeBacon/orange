cmake_minimum_required(VERSION 3.7)

project(microasm)

include_directories(src/include)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(debug)
elseif()
    add_compile_definitions(release)
endif()

add_executable(microasm
    src/main.c
    src/scanner.c
    src/token.c
    src/parser.c
    src/ast.c
    src/memory.c
    src/platform.c
    src/error.c
    )

set_property(TARGET microasm PROPERTY C_STANDARD 99)

target_link_libraries(microasm m)

if(MSVC)
  target_compile_options(microasm PRIVATE /W4 /WX)
else()
  target_compile_options(microasm PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
