SRC=src/main.c src/adc.c include/zb_swift_device.h app_usb.overlay pm_static.yml prj.conf
BIN=build/zephyr/zephyr.bin

all: $(BIN)

clean:
	west build -t clean

$(BIN): $(SRC)
	west build -b nrf52840dk_nrf52840

flash: $(BIN)
	west flash

sed:
	west build -b nrf52840dongle_nrf52840 -p -- -DCONF_FILE=prj_power_saving.conf
