# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky)

target_include_directories(app PRIVATE include)
target_sources(app PRIVATE src/custom_utils.c)
target_sources(app PRIVATE src/custom_led.c)
target_sources(app PRIVATE src/lcd_driver.c)
target_sources(app PRIVATE src/main.c)
