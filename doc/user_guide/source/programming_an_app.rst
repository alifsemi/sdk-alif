.. _programming_an_application:

Programming an Application
==========================

Alif offers a set of tools that can be used to merge Zephyr images and binaries of different cores, resulting in a single image known as an Application Table of Contents (ATOC) image. This ATOC image, which includes the Zephyr binary images (zephyr_e7_rtsshe_helloworld.bin, zephyr_e7_rtsshp_helloworld.bin) for the HE or HP Real-Time SubSystems[RTSS], is programmed into MRAM (a non-volatile memory) using the tools.

Getting the ATOC image onto MRAM consists of the following steps:

1. First, create an application image that contains the Zephyr binary images for the RTSS cores.

2. Next, you should program the ATOC, which includes the application images, into MRAM using the SETOOLS on the host.

3. Once programmed, the applications should print "Hello World!" on the UART4 and UART2 consoles for the RTSS-HE and RTSS-HP, respectively.

Creating ATOC
-------------

To create and deploy an ATOC image containing the application binaries, follow the steps below.

The JSON file (`zephyr_e7_rtsshe_rtsshp_helloworld.json`) is configured to boot these binaries from the ITCM of the RTSS-HE and RTSS-HP on the next power-on reset (POR).

The application binaries are:

- **RTSS-HE Application binary:** `zephyr_e7_rtsshe_helloworld.bin`
- **RTSS-HP Application binary:** `zephyr_e7_rtsshp_helloworld.bin`

#. Ensure that you have connected a USB PRG cable between the host and the SE-UART of the DevKit. Please refer section :ref:`connecting-usb-prg` to know how to connect the FTDI USB cable.
#. Press the reset button to power on DevKit.
#. Create a JSON file zephyr_e7_rtsshe_rtsshp_helloworld.json for the RTSS-HE and RTSS-HP. You can also use the file with the same filename that was copied into this directory during setup.

   .. code-block:: console

      cd app-release-exec-linux/
      gedit build/config


.. code-block:: json

    {
        "DEVICE": {
            "disabled": false,
            "binary": "app-device-config.json",
            "version": "0.5.00",
            "signed": true
        },
        "Zephyr-RTSS-HE": {
            "binary": "zephyr_e7_rtsshe_helloworld.bin",
            "version": "1.0.0",
            "cpu_id": "M55_HE",
            "loadAddress": "0x58000000",
            "flags": ["load", "boot"],
            "signed": false
        },
        "Zephyr-RTSS-HP": {
            "binary": "zephyr_e7_rtsshp_helloworld.bin",
            "version": "1.0.0",
            "cpu_id": "M55_HP",
            "loadAddress": "0x50000000",
            "flags": ["load", "boot"],
            "signed": false
        }
    }


#. Go to Alif Security Toolkit directory containing app-gen-toc, app-write-mram, and updateSystemPackage programs. (This example assumes the tools are located at /home/$USER/app-release-exec-linux).

   .. code-block:: console

       cd app-release-exec-linux

#. Create an Alif Application image using the app-gen-toc script and choose the above created json.

   .. code-block:: console

     ./app-gen-toc -f build/config/zephyr_e7_rtsshe_rtsshp_helloworld.json

   The above command creates an application image called build/AppTocPackage.bin under the build directory.

Deploying ATOC
--------------

Program Alif application image build/AppTocPackage.bin under the build directory through ISP.

Syntax:

.. code-block:: console

    ./app-write-mram -p

.. note::

   a. In the above example, the SE-UART is detected as `/dev/ttyACM0` on the host. Please identify the correct device node using the `dmesg` command.
   b. Please refer to `Alif Security Toolkit Quick Start Guide`_ for more information.

.. tip::

   Ensure that your user has sufficient access to the `dialout` group for SE-UART device communication.

   - To check if the current user has access, use:

     .. code-block:: console

        groups

   - If `dialout` or `tty` is not in the list, add the user to the `dialout` group with:

     .. code-block:: console

        sudo usermod -a -G dialout USERNAME

   - If the device is still not recognized, check for loose connections or try using a different USB port.


Booting the RTSS-HE and RTSS-HP with the Programmed Applications
----------------------------------------------------------------

#. Open a serial console application on host PC - baud rate of 115200.
#. Select the RTSS-HE USB port (Example: /dev/ttyACM1)
#. Select the RTSS-HP USB port (Example: /dev/ttyUSB0)
#. You can see the greeting on the serial console as below.

.. code-block:: console

   *** Booting Zephyr OS build 4b48dd532761 ****
   Hello World ! alif_e7_devkit

