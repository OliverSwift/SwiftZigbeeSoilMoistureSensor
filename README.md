# Swift Zigbee Soil Moisture Sensor

This hardware project is about creating a Zigbee adapter to the existing Capacitive Soil Moisture Sensor 1.2 widely available.

![Hw illustration](doc/Hardware3D.png "Hardware")

![Capacitive Sensor](doc/capacitive_sensor_v1.2.jpg)

## How to use it

[Here is](doc/Swift%20Zigbee%20Soil%20Moisture%20Sensor.pdf) the small user manual.

It works well with a Conbee2 zigbee concentrator used with Home Assistant.

CR2032 coin battery should allow approximately two years of autonomy.

## Technical background

After some trials on STM32WB55 which permanently failed to connect to a zigbee concentrator (Conbee2) I finally chose the nrf52840 SOC from Nordic Semicondudtors. It is very well known in the BLE world and decided to test its Zigbee capabilities and it was successful.

The solution is based on ZephyrOS and NRF Connect SDK version 2.6.1 that embeds ZBoss proprietary Zigbee stack. One of the challenging aspect of this project is the power consumption. The nrf52840 is able to enter deep sleep and only consumes 2.5µA approximately. Now I had to face that the Capacitive Sensor is based on NE555. Surprisingly it is not the CMOS version and is supposed to be power supplied between 5 and 15V. But it can be with only 3.3V. Any attempt to power supply below this value resulted in unstable sensor. So I decided to use 3.3V for the whole device (MCU+sensor). This led to choosing a boost converter so a coin battery can be used, ideally CR2032.

The average current measurement is about 10µA, which should allow 2 year autonomy. Probe measurement is done every 30 minutes. When it occurs, the tiny LED lits.

The project was developed thanks to [nRF52840 DK board](doc/nRF52840_DK_User_Guide_v1.2.pdf). I used the OB JLink capability to flash the target board.

The nRF52840 module (E73-2G4M08S1C) comes from EBYTE. It doesn't have the external 32kHz crystal, but this one isn't useful for Zigbee applications.

Documentation folder includes:

- [Electronic schematics](doc/schematics-2.0.pdf)
- Main components datasheet (nrf52840 and tps61097a)
- How to build and flash

CAD folder includes:

- [KiCAD](cad/KiCAD) project files
- [FreeCAD](cad/FreeCAD) projet, a desparate attempt to designing a [box](cad/FreeCAD/box.stl)
