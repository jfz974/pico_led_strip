cmake_minimum_required(VERSION 3.13)

# Shared Pico SDK checkout used by the sibling music_box firmware project on
# this machine.
set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/../../Documents/music_box/pico-sdk" CACHE PATH "Path to the Pico SDK")
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pico_led_strip C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_led_strip
	../src/main.cpp
)
target_include_directories(pico_led_strip PRIVATE ../src)
target_link_libraries(pico_led_strip pico_stdlib hardware_pio)
pico_generate_pio_header(pico_led_strip ../src/ws2812.pio)
pico_add_extra_outputs(pico_led_strip)
