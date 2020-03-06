# ------------- #
# cmake options #
# ------------- #
cmake_minimum_required(VERSION 3.9.4)

# -------------- #
# Create project #
# -------------- #
project(Orange)

# -------------------- #
# default target setup #
# -------------------- #
set(MY_C_FLAGS
    -Wall
    -Wextra
    -Werror
    -Wno-missing-field-initializers
)

# Link time optimisation
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error)
    if( supported )
        message(STATUS "IPO / LTO enabled")
        if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
            set(CMAKE_AR  "gcc-ar")
            set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
            set(CMAKE_C_ARCHIVE_FINISH   true)
        endif()
    else()
        message(STATUS "IPO / LTO not supported: <${error}>")
    endif()
else()
	set(MY_C_FLAGS ${MY_C_FLAGS} -g -O0)
endif()

# function to setup default options on a new target
function(setup_target target stage)
    string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
    target_compile_definitions(
        ${target} PRIVATE
            SOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}
            BUILD_STAGE=${stage}
            $<$<OR:$<CONFIG:debug>,$<CONFIG:relwithdebinfo>>:DEBUG_BUILD>
    )

    target_link_libraries(${target} m)
    target_include_directories(${target} PRIVATE src)
    target_compile_options(${target} PRIVATE ${MY_C_FLAGS})
    set_property(TARGET ${target} PROPERTY C_STANDARD 11)

    if(supported)
        set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endfunction()

# ------------- #
# STAGE 0 BUILD #
# ------------- #
set(STAGE_0_BUILD
    src/main.c
    src/shared/memory.c
    src/shared/platform.c
    src/shared/table.c
    src/shared/arg.c
    src/shared/graph.c
    src/shared/path.c
    src/shared/log.c
    src/shared/table2.c

    src/microcode/scanner.c
    src/microcode/token.c
    src/microcode/parser.c
    src/microcode/ast.c
    src/microcode/error.c
    src/microcode/test.c
    src/microcode/analyse.c
    src/microcode/analysisTypes.c
    src/microcode/analyseMicrocode.c

    src/emulator/compiletime/template.c
    src/emulator/compiletime/create.c
    src/emulator/compiletime/codegen.c
    src/emulator/compiletime/runCodegen.c
)

add_executable(generator ${STAGE_0_BUILD})
setup_target(generator 0)

# ------------- #
# STAGE 1 BUILD #
# ------------- #
set(STAGE_1_BUILD
    ${STAGE_0_BUILD}
    src/emulator/runtime/emu.c
    "${CMAKE_CURRENT_BINARY_DIR}/switch.c"
)

set(MICROCODE_COMPILE_SOURCES
    src/emulator/microcode.uasm
    src/emulator/types.uasm
    src/emulator/runtime/busToReg.c
    src/emulator/runtime/halt.c
    src/emulator/runtime/instRegSet.c
    src/emulator/runtime/memRead.c
    src/emulator/runtime/memWrite.c
    src/emulator/runtime/regToBus.c
)

add_custom_command(
    OUTPUT switch.c
    COMMAND generator codegen -s stdout -l log.txt "${CMAKE_CURRENT_SOURCE_DIR}/src/emulator/microcode.uasm" "${CMAKE_CURRENT_BINARY_DIR}/switch.c"
    DEPENDS generator ${MICROCODE_COMPILE_SOURCES}
)

add_executable(microasm ${STAGE_1_BUILD})
setup_target(microasm 1)
