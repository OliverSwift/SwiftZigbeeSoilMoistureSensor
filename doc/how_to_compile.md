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
# make flash
```

With JLink Probe attached to PC and board
