cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards)
set(PICO_BOARD waveshare_rp2350b_plus_w)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(rp2350b_plus_w_wifi_sta C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

if(NOT WIFI_SSID OR NOT WIFI_PASSWORD)
    message(FATAL_ERROR "Configure Wi-Fi with -DWIFI_SSID=... -DWIFI_PASSWORD=...")
endif()

set(GAME_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

add_executable(wifi_sta
    wifi_sta.cpp
    ${GAME_ROOT}/src/hardware/display/ili9488/ili9488_driver.cpp
    ${GAME_ROOT}/src/hardware/display/ili9488/ili9488_font.cpp
)

target_include_directories(wifi_sta PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${GAME_ROOT}/include
)

target_compile_definitions(wifi_sta PRIVATE
    "WIFI_SSID=\"${WIFI_SSID}\""
    "WIFI_PASSWORD=\"${WIFI_PASSWORD}\""
)

target_link_libraries(wifi_sta
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_spi
    hardware_pwm
    hardware_gpio
    hardware_dma
)

pico_enable_stdio_usb(wifi_sta 1)
pico_enable_stdio_uart(wifi_sta 0)
pico_add_extra_outputs(wifi_sta)
