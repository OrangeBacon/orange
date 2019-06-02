cmake_minimum_required(VERSION 3.9)

project(microasm)

# include_directories(src/include)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(debug)
elseif()
    add_compile_definitions(release)
endif()

add_executable(microasm
    src/main.c )
set_property(TARGET microasm PROPERTY C_STANDARD 99)