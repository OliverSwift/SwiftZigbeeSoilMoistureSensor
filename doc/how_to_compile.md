# With nRF Connect for Desktop
1. Get [nRF Connect for Desktop](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-desktop)
2. Start it.
3. Install Toolchain Manager.
4. Launch Toolchain Manager (Click the Open button)
5. Install nRF Connect SDK 4.2.1 (the one used here, lates may woek as well)
6. From the dropdown list select Generate Environment Script, save it to ~/ncs for example

~/ncs is where everything stands


# Manually

[Here](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/installation/install_ncs.html)

# Compile

Make sure you source the env.sh script that sets up SDK environment variables

then type:

```
# make
```

# Nordic's page

[SDK Installation ](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/installation.html)

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

With JLink Probe attached to PC and board type:

```
# make flash
```
