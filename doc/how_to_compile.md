# With nRF Connect for Desktop
1. Get [nRF Connect for Desktop](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-desktop)
2. Start it.
3. Install Toolchain Manager.
4. Launch Toolchain Manager (Click the Open button)
5. Install nRF Connect SDK 2.6.1 (only valid version for low power)
6. From the dropdown list select Generate Environment Script, save it to ~/ncs for example

~/ncs is where everything stands

Tools for flashing are required, see below nrf Command line Tools.

## Installing SDK Manually

Not recommanded but used SDK may not be available from nRF Connect for Desktop later on.

[Here](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/installation/install_ncs.html)

## Compile the project

Make sure you source the env.sh script that sets up SDK environment variables

then type:

```
# make
```

## Compile for production

```
# make clean
# make prod
```

## Nordic's page

Source pages about installations:  [SDK Installation ](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/installation.html)


# Installing nrfjprog for flashing

## Install nrf-Command-Line-Tools

Download [here](https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools)

```
# dpkg -i nrf-command-line-tools_10.24.0_amd64.deb
```

Last output lines propose to install JLink that comes with the package. This is the preferred way.

## SEGGER JLink installation [optional]

Install above should propose to install JLink which is recommended.

However, it may also be installed manually:

We need to install SEGGER JLink took first. Visit [SEGGER's download page](https://www.segger.com/downloads/jlink)
And install Linux version.

```
# dpkg -i JLink_Linux_V794i_x86_64.deb
```

# Flashing the device

The board J3 connector pins have Vtref, SWDIO, SWDCLK and GND. The three should be properly wired to any SWD compatible
programming device (JLink, OB-Jlink, etc).

Cell coin battery MUST NOT be inserted during flashing operation.

## Using nrf52840 DK board for flashing

The PCA10056 comes with SEGGER J-Link OB with a debug out option.
Here is how to wire the DK to board for flashing:

![Flashing](Flashing.png)

## Flash for dev

With JLink Probe attached to PC and board type:

```
# make flash
```

## Flash for production

Warning: AP protection is applied here. No possible debug or memory readback possible after this.

With JLink Probe attached to PC and board type:

```
# make flash_prod
```
