SRC=src/main.c src/adc.c include/zb_swift_device.h app.overlay prj.conf
BIN=build/zephyr/zephyr.bin

all: $(BIN)

clean:
	rm -rf ${HOME}/.cache/ccache
	rm -rf ${HOME}/.cache/zephyr
	rm -rf build
	rm -rf prod

$(BIN): $(SRC)
	west build -- -DCONF_FILE=prj.conf

flash: $(BIN)
	west flash

sed:
	(west build -p 2>&1; md5sum build/zephyr/merged.hex) | tee -p /tmp/output-`date +%Y%m%d%H%M%S`.log

prod:
	mkdir -p prod
	cmake -B prod -S .
	cmake --build prod -j
	md5sum prod/zephyr/merged.hex

flash_prod: prod/zephyr/merged.hex
	nrfjprog -f NRF52 --program prod/zephyr/merged.hex --sectoranduicrerase --verify --reset
