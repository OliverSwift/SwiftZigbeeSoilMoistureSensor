SRC=src/main.c src/adc.c include/zb_swift_device.h app.overlay prj.conf
BIN=build/zephyr/zephyr.bin

all: $(BIN)

clean:
	west build -t clean

$(BIN): $(SRC)
	west build -b nrf52840dk_nrf52840

flash: $(BIN)
	west flash

sed:
	(west build -b nrf52840dk_nrf52840 -p -- -DCONF_FILE=prj_power_saving.conf 2>&1; md5sum build/zephyr/merged.hex) | tee -p /tmp/output-`date +%Y%m%d%H%M%S`.log
