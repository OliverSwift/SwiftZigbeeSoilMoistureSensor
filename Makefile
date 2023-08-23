SRC=src/main.c src/adc.c include/zb_swift_device.h app_usb.overlay pm_static.yml prj.conf
PKG=swift.zip

all: build/zephyr/zephyr.bin

clean:
	west build -t clean
	rm $(PKG)


build/zephyr/zephyr.bin: $(SRC)
	west build -b nrf52840dongle_nrf52840

$(PKG): build/zephyr/zephyr.bin
	nrfutil pkg generate --hw-version 52 --sd-req=0x00 --application build/zephyr/zephyr.hex --application-version 1 $(PKG)

flash: $(PKG)
	nrfutil dfu usb-serial -pkg $(PKG) -p /dev/ttyACM0
