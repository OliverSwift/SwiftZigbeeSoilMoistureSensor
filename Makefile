SRC=src/main.c src/adc.c include/zb_swift_device.h app_usb.overlay pm_static.yml prj.conf

all: build/zephyr/zephyr.bin

clean:
	west build -t clean


build/zephyr/zephyr.bin: $(SRC)
	west build -b nrf52840dongle_nrf52840

template.zip: build/zephyr/zephyr.bin
	nrfutil pkg generate --hw-version 52 --sd-req=0x00 --application build/zephyr/zephyr.hex --application-version 1 template.zip

flash: template.zip
	nrfutil dfu usb-serial -pkg template.zip -p /dev/ttyACM0
