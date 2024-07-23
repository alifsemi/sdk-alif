# sdk-alif
Alif semiconductor's Zephyr SDK

This repository contains Alif semiconductor's base code having some Alif device specific samples.
It also contains the west.yml needed to fetch all the needed modules and submodules for the SDK.'

# Getting started
If you are already familiar with Zephyr and 'west' tool you need to simply install it and run following command in a (clean) folder of your choice
```
west init -m org-115832732@github.com:AlifSemiDev/sdk-alif; west update
```

Otherwise you should familiarize yourself with Zephyr:
See the [Introduction to Zephyr](http://docs.zephyrproject.org/latest/introduction/index.html) for a high-level overview, and the documentation's [Getting Started Guide](http://docs.zephyrproject.org/latest/develop/getting_started/index.html) to start developing.

# Matter SDK

This chapter introduces Matter SDK and how to take it into use.
See the [Introduce to Matter SDK project](https://project-chip.github.io/connectedhomeip-doc/index.html) documentation about Matter SDK Open source project and [Open Thread Border Router](https://openthread.io/guides/border-router) documentations.

## Requirements

1. 1-2 Balletto DK for Matter devices.
2. nRF8240 USB Dongle for Thread Border router with Network RPC Co-Processor binary
3. Ubuntu 22.04 PC for acting as a Border router


## Getting started

Alif Matter SDK is using Zephyr platform setup with OpenThread. So initial setup for Zephyr environment is needed.
This chapter defines how to set up Matter environment for Linux Ubuntu 22.04 based operating system and how to config Thread border router for Matter.

### Prerequites for Ubuntu 22.04

Install dependencies:

```
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev \
     libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev \
     python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev
```

### Matter SDK helper scripts

Alif SDK has helper scripts to create a Matter build environment and building it.
These scripts need to be called from the Alif SDK root folder.

```
scripts/matter
├── activate_env.sh
└── matter_env_setup.sh
```

### Matter subsystem

Matter subsystem is a glue layer between a Matter open source project and a Zephyr application.
It defines an API for initializing Matter stack and its main features.
```
subsys/matter
├── binding
│   ├── BindingHandler.cpp
│   └── BindingHandler.h
├── common
│   ├── FabricTableDelegate.cpp
│   ├── FabricTableDelegate.h
│   ├── MatterStack.cpp
│   └── MatterStack.h
└── pwmdevice
    ├── PWMDevice.cpp
    └── PWMDevice.h
```

### Matter samples

* light-bulb: Matter light switch can be controlled by Matter controller `chip-tool` or Matter light switch.
* light-switch: Matter light switch which can be bind to Matter light by `chip-tool` for control.

```
samples/matter
├── light-bulb
│   ├── b1_factorydata_partition.overlay
│   ├── CMakeLists.txt
│   ├── factory_data.conf
│   ├── Kconfig
│   ├── prj.conf
│   └── src
│       ├── AppTask.cpp
│       ├── include
│       │   ├── AppConfig.h
│       │   ├── AppEvent.h
│       │   ├── AppTask.h
│       │   ├── BoardUtil.h
│       │   └── CHIPProjectConfig.h
│       ├── main.cpp
│       └── ZclCallbacks.cpp
└── light-switch
    ├── b1_factorydata_partition.overlay
    ├── CMakeLists.txt
    ├── factory_data.conf
    ├── Kconfig
    ├── prj.conf
    └── src
        ├── AppTask.cpp
        ├── include
        │   ├── AppConfig.h
        │   ├── AppEvent.h
        │   ├── AppTask.h
        │   ├── BoardUtil.h
        │   ├── CHIPProjectConfig.h
        │   ├── LightSwitch.h
        │   └── ShellCommands.h
        ├── LightSwitch.cpp
        ├── main.cpp
        └── ShellCommands.cpp

```

### Create Python virtual env and install Matter tools

Matter open source project provides a script that creates an own Python virtual environment and installs ZAP tools.
1. The script calls Matter bootstrap script
2. Installs all necessary Python packets
3. Builds Matter SDK Host tools
4. Adds the tools to PATH with following commands:
```
source scripts/matter/matter_env_setup.sh
```

After running this installation script, Matter SDK build system is ready for compiling the samples.

Before building Matter ZAP or Chip-Tool, Matter virtual environment must be activated with following command:
```
source scripts/matter/activate_env.sh
```

If the activate script says the environment is out of date, you can update it by running the following command:

```
source scripts/matter/matter_env_setup.sh
```

Deactivate Matter build environment with the following command:

```
deactivate
```

### Thread Border router setup

This guide covers building and configuration of Open Thread Border router (OTBR) with nRF52840 USB Dongle with Open Thread Co-Processor application.

### Configure and build

Clone the OTBR repository and install default setup:

```
git clone https://github.com/openthread/ot-br-posix
cd ot-br-posix
./script/bootstrap
```

Compile and install by using Ethernet interface:
```
INFRA_IF_NAME=eth0 ./script/setup
```
**NOTE** Ethernet interface may be something else than eth0 on your system so adjust it accordingly

#### Configure RCP device

Create a custom udev rule file to identify the nRF52840 dongle based on it's vendor- and product-id.

Attach the flashed RCP device to the Border Router platform via USB and check device serial number by using command:
```
sudo dmesg
```

Edit a `/etc/udev/rules.d/99-acm.rules` and add following line with your dongle's information

```
SUBSYSTEM=="tty", ATTRS{idVendor}=="1915", ATTRS{idProduct}=="0000", ATTRS{serial}=="YOUR_DONGLES_SERIAL_HERE", SYMLINK+="ttyOTBR"

```
Check that the device is found with the given name by the following command:

```
ls /dev/tty*

```
You should see `/dev/ttyOTBR`.

To configure the RCP device's serial port in otbr-agent settings:
```
sudo nano /etc/default/otbr-agent
```
Edit `OTBR_AGENT_OPTS` line by following way for device `/dev/ttyOTBR` and `eth0`:
```
OTBR_AGENT_OPTS="-I wpan0 -B eth0 spinel+hdlc+uart:///dev/ttyOTBR trel://eth0"
```
Next, power cycle the dongle to activate the border router and create a `wpan0`.

## Light switch samples

This sample demonstrates a device acting as a light switch which controls a light bulb
Build sample with following commands:
```
cd samples/matter/light-switch
west build -b alif_b1_fpga_rtss_he_ble
```

## Light bulb sample

This sample demonstrates a device acting as a light bulb and how to bind it to a light switch or used direct by `chip-tool` controller.
Build sample with following commands:
```
cd samples/matter/light-bulb
west build -b alif_b1_fpga_rtss_he_ble
```

## Commission device to Thread network over BLE

After flashing the sample device will activate default private thread network named as `ot_zephyr`. The device will also activate BLE advertisement for commissioning over BLE.
Thread and Matter commissioning is handled by Matter SDK's `chip-tool` which is build and added to `PATH`.

### Commissioning requirements
`chip-tool` commissioning over BLE to Thread network command API:
```
$ chip-tool pairing ble-thread <node_id> hex:<operational_dataset> <pin_code> <discriminator>
```

Before start process we need to know the following parameters:
* node_id: Device Node identfier given by user
* operational_dataset: Thread Border router active Dataset
* pin_code: Device BLE Setup Pincode, Default: `20202021`
* discriminator: Device 12-bit Discriminator, Default: `3840` in hex: `0xf00`

### How to get Thread border router active dataset

Border router active data is checked by following command:
```
sudo ot-ctl dataset active -x
35060004001fffe00c0402a0f7f8051000112233445566778899aabbccddee00030e4f70656e54687265616444656d6f0410445f2b5ca6f2a93a55ce570a70efeecb000300001a02081111111122222222010212340708fd110022000000000e0800000003601c0000
Done
```
### How to get Device information:

Matter shell has a command `matter config` to print active configuration.
Here is an example:
```
uart:~$ matter config
VendorId:        65521 (0xFFF1)
ProductId:       32772 (0x8004)
HardwareVersion: 0 (0x0)
PinCode:         20202021
Discriminator:   f00
Done
```

## Matter light control demo

Start `chip-tool` in interactive mode by the following command:
```
chip-tool interactive start
```
Interactive mode is enabled to call multiple commands without timeouts.

### Commission Matter devices

Flash the light bulb device and commission that to Thread network and use endpoint `node_id´ 1:
```
pairing ble-thread 1 hex:35060004001fffe00c0402a0f7f8051000112233445566778899aabbccddee00030e4f70656e54687265616444656d6f0410445f2b5ca6f2a93a55ce570a70efeecb000300001a02081111111122222222010212340708fd110022000000000e0800000003601c0000 20202021 3840 --timeout 65535
```
Flash the light switch device and commission that to Thread network and use endpoint `node_id´ 2:
```
pairing ble-thread 2 hex:35060004001fffe00c0402a0f7f8051000112233445566778899aabbccddee00030e4f70656e54687265616444656d6f0410445f2b5ca6f2a93a55ce570a70efeecb000300001a02081111111122222222010212340708fd110022000000000e0800000003601c0000 20202021 3840 --timeout 65535
```

### Bind the light switch to the light bulb

Update Light bulb's access list to include the light switch by the following command:
```
accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [2], "targets": [{"cluster": 6, "endpoint": 1, "deviceType": null}, {"cluster": 8, "endpoint": 1, "deviceType": null}]}]' 1 0
```
Next, bind the light switch device to the light bulb device:
```
binding write binding '[{"fabricIndex": 1, "node": 1, "endpoint": 1, "cluster": 6}, {"fabricIndex": 1, "node": 1, "endpoint": 1, "cluster": 8}]' 2 1
```
Now the light switch device can control the light bulb device's led.

### Light control

The light switch sample supports three commands.

* Switch on: `matter light on`
* Switch off:`matter light off`
* Toggle light state:`matter light toggle`

The `chip-tool` controller can be used too control a commissioned Matter light device.
```
onoff <command> <destination-id> <endpoint-id>

`command` Light control command:
* `on`: Switch On
* `off`: Switch Off
* `toggle`: Toggle Light state
`destinatio-id` is device commisioned `node_id`.
`endpoint-id` is 1.
```

Example for toggling `node_id` 1's light state:
```
onoff toggle 1 1
```

