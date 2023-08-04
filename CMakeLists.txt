#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

set(CONF_FILE "prj_usb.conf")

# Include app_usb.overlay when sample is built with prj_usb.conf
set(DTC_OVERLAY_FILE "app_usb.overlay")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project("Zigbee application template")

# NORDIC SDK APP START
target_sources(app PRIVATE
  src/main.c
)

target_include_directories(app PRIVATE include)
# NORDIC SDK APP END
