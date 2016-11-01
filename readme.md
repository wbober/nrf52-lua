Lua for NRF52
-------------
This project is a [Lua](https://www.lua.org/) based firmware for [Nordic Semiconductor nRF52 SoC](https://www.nordicsemi.com/Products/nRF52-Series-SoC).

The project is done for my personal enjoyment, so it's going to be developed at slow pace and modules will be added as needed. In long run the goal is to make an API which is compatible with [NodeMCU for ESP8266](https://nodemcu.readthedocs.io/en/master/).

If you're after something which is more actively developed with a richer feature set, then take a look at [Espruino for nRF52](https://devzone.nordicsemi.com/blogs/980/espruino-program-nrf5-devices-in-javascript-see-th).

Please remember that this project is not aimed to be a replacement for a crafted and optimised firmware implemented in C but rather a way to quickly prototype a BLE device.

What's done?
------------
At this stage the following features are supported:

* An interactive shell based on [Lua 5.3](https://www.lua.org/).
* A memory manager based on [umm_malloc](https://github.com/dimonomid/umm_malloc).
* A file system in the internal flash based on [spiffs](https://github.com/pellepl/spiffs).
* Lua `ble` module for basic nonconnectable advertising *(API will change)*.

What's in the pipeline?
-----------------------
* `adc` and `pwm` modules.

Getting started
---------------
To run the firmware you'll need to download and flash the SoftDevice (S132-SD-v3) to your DK.  If you'd like to build the firmware you'll need the nRF5 SDK Version 12. nRF5x Command Line Tools are used for flashing.

All of the above packages [here](https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF52-DK).

Running
-------
You'll find a precompiled firmware binary in `bin/shell.hex`. The firmware can be programmed using nrfjprog from nRF5x Command Line tools as follows:

	nrfjprog -f nrf52 --program _build/shell.hex --sectorerase
    nrfjprog -f nrf52 -r

Building
--------
In order to build the firmware define the location of the SDK:

	`export NRF5_SDK=<path_to_sdk>`

and then run:

	`make`

To flash the firmware run:

	`make flash shell`