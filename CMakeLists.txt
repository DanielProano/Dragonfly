cmake_minimum_required(VERSION 3.28.1)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/stm32_toolchain.cmake)

project(Dragonfly C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_SOURCE_DIR}/linker/stm32f401.ld")
set(FIREFLY_DIR ${CMAKE_SOURCE_DIR}/firefly)

add_subdirectory(firefly)

file(GLOB_RECURSE DRAGONFLY_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.c)

add_executable(dragonfly 
    ${DRAGONFLY_SOURCES}
    Dragonfly_Protocol/crc.c
    Dragonfly_Protocol/protocol_codec.c
    startup/stm32f401.c
)

file(GLOB_RECURSE DRAGONFLY_HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.h)

set(DRAGONFLY_INCLUDES "")
foreach(header ${DRAGONFLY_HEADERS})
    get_filename_component(header_dir ${header} DIRECTORY)
    list(APPEND DRAGONFLY_INCLUDES ${header_dir})
endforeach()
list(REMOVE_DUPLICATES DRAGONFLY_INCLUDES)

target_include_directories(dragonfly PRIVATE
    ${DRAGONFLY_INCLUDES}

    # Submodules
    firefly
    Dragonfly_Protocol
)

target_link_libraries(dragonfly
    hal_stm32f401
    kernel
    port_cortex_m4
    m
)

find_program(OPENOCD openocd)

if (OPENOCD) 
    add_custom_target(flash COMMAND ${OPENOCD} -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $<TARGET_FILE:dragonfly> verify reset exit" DEPENDS dragonfly USES_TERMINAL)
endif()
