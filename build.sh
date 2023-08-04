#!/bin/bash
west build -b nrf52840dongle_nrf52840
if [ "$1" = "-f" ]; then 
    nrfutil pkg generate --hw-version 52 --sd-req=0x00 --application build/zephyr/zephyr.hex --application-version 1 template.zip
    nrfutil dfu usb-serial -pkg template.zip -p /dev/ttyACM0
fi
