cmake_minimum_required(VERSION 3.28.1)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/stm32_toolchain.cmake)

project(Dragonfly C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_SOURCE_DIR}/linker/stm32f401.ld")
set(FIREFLY_DIR ${CMAKE_SOURCE_DIR}/firefly)

add_subdirectory(${FIREFLY_DIR}/src/hal/stm32f401
${CMAKE_BINARY_DIR}/firefly/hal) 
add_subdirectory(${FIREFLY_DIR}/src/kernel
${CMAKE_BINARY_DIR}/firefly/kernel)
add_subdirectory(${FIREFLY_DIR}/src/port/cortex_m4 
${CMAKE_BINARY_DIR}/firefly/cortex_m4)

add_executable(dragonfly 
    src/comms/src/crsf.c
    src/comms/src/protocol.c
    src/controls/src/attitude.c
    src/controls/src/motor_mix.c
    src/controls/src/pid.c
    src/drivers/src/esc.c
    src/drivers/src/mpu.c
    src/hal/src/i2c.c
    src/hal/src/pwm.c
    src/hal/src/uart.c
    src/sensors/src/imu.c
    src/tasks/src/comms_task.c
    src/tasks/src/control_task.c
    src/tasks/src/health_task.c
    src/main.c
    startup/stm32f401.c
)

target_include_directories(dragonfly PRIVATE
    src/comms/include
    src/controls/include
    src/drivers/include
    src/hal/include
    src/sensors/include
    src/tasks/include
)

target_link_libraries(dragonfly
    hal_stm32f401
    kernel
    port_cortex_m4
)