#
# Copyright (c) 2024 Olivier DEBON
# SPDX-License-Identifier: AGPL-3.0-or-later
#

cmake_minimum_required(VERSION 3.20.0)

set(DTC_OVERLAY_FILE "app.overlay")
set(BOARD "nrf52840dk_nrf52840")
set(CONF_FILE "prj_power_saving.conf")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project("Zigbee application Swift Soil Moisture Sensor")

target_sources(app PRIVATE
  src/main.c
  src/adc.c
)

target_include_directories(app PRIVATE include)
