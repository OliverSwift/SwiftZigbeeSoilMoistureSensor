SRC=src/main.c src/adc.c include/zb_swift_device.h prj.conf

all: build/zephyr/zephyr.bin

clean:
	west build -t clean
	rm -f $(PKG)


build/zephyr/zephyr.bin: $(SRC)
	west build -b nrf52840dk_nrf52840

flash: build/zephyr/merged.hex
	west flash
