.. _setting_up_and_building_zephyr:

Setting Up and Building Zephyr Applications
===========================================

This guide covers:

- Setting up the host system on Ubuntu 20.04 LTS or later.
- Building a sample application.

.. note::
   Examples in this document use the Ensemble E7 Devkit unless otherwise specified.

Setting Up the Host System
--------------------------

Follow these steps to install dependencies and configure the environment for the Alif SDK.

1. Update the package list:

   .. code-block:: console

      sudo apt update -y

2. Install required dependencies using ``apt``:

   .. code-block:: console

      sudo apt install -y --no-install-recommends python3-dev python3-pip python3-venv \
         git wget xz-utils file make python3-setuptools python3-wheel ninja-build \
         build-essential cmake libmagic1

3. Create and activate a Python virtual environment:

   a. Create the virtual environment if not already set up:

      .. code-block:: console

         python3 -m venv ~/zephyrproject/.venv

   b. Activate the virtual environment:

      .. code-block:: console

         source ~/zephyrproject/.venv/bin/activate

      Your shell prompt will show ``(.venv)`` when activated.

      .. note::
         Activate the virtual environment in every new terminal session before running
         Python or ``west`` commands. Deactivate with ``deactivate`` when done.

4. Install Python packages:

   .. code-block:: console

      pip install west pyelftools

Fetching the Alif Zephyr SDK
----------------------------

This section explains how to build Zephyr using the GCC toolchain. For details on
toolchain selection, refer to Zephyr's documentation:
`Toolchain Selection <https://docs.zephyrproject.org/latest/develop/toolchains/index.html>`_.

Fetch the SDK source from the ``main`` branch:

.. code-block:: console

   mkdir sdk-alif
   cd sdk-alif
   west init -m https://github.com/alifsemi/sdk-alif_2.0.git --mr ${revision}
   west update

Replace ``${revision}`` with the desired SDK revision (branch, tag, or commit SHA).
Use ``main`` for the latest state, or specify a commit SHA or tag.

**Syntax:**

.. code-block:: bash

   west init -m <URL> --mr <REVISION>

**Example:**

.. code-block:: bash

   west init -m https://github.com/alifsemi/sdk-alif_2.0.git --mr main

Building an Application
-----------------------

Supported Board Targets as per new hardware model v2 (Zephyr v4.1.0 and onwards):
<board name[@revision][/board qualifiers]>

- alif_e3_dk/ae302f80f55d5xx/rtss_he
- alif_e3_dk/ae302f80f55d5xx/rtss_hp
- alif_e4_dk/ae402fa0e5597xx0/rtss_he
- alif_e4_dk/ae402fa0e5597xx0/rtss_hp
- alif_e7_dk/ae722f80f55d5xx/rtss_he
- alif_e7_dk/ae722f80f55d5xx/rtss_hp
- alif_e8_dk/ae822fa0e5597xx0/rtss_he
- alif_e8_dk/ae822fa0e5597xx0/rtss_hp
- alif_e1c_dk/ae1c1f4051920hh/rtss_he
- alif_b1_dk/ab1c1f4m51820hh/rtss_he

.. note::
   The ``alif_e7_dk/ae722f80f55d5xx/rtss_he`` and ``alif_e7_dk/ae722f80f55d5xx/rtss_hp`` targets can also be run on
   the Devkit E5.

a. Navigate to the Zephyr directory:

   .. code-block:: console

      cd zephyr

b. Build the Hello World application:

   An application that prints a "Hello World" message along with the board name.
   By default, code execution takes place from MRAM.

**RTSS-HE**

- Build for MRAM (Address: 0x80000000):

  .. code-block:: console

     west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/hello_world

- Build for ITCM:

  .. code-block:: console

     west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/hello_world -DCONFIG_FLASH_BASE_ADDRESS=0 -DCONFIG_FLASH_LOAD_OFFSET=0 -DCONFIG_FLASH_SIZE=256

**RTSS-HP**

- Build for MRAM (Address: 0x80200000):

  .. code-block:: console

     west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/hello_world

- Build for ITCM:

  .. code-block:: console

     west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/hello_world -DCONFIG_FLASH_BASE_ADDRESS=0 -DCONFIG_FLASH_LOAD_OFFSET=0 -DCONFIG_FLASH_SIZE=256

.. note::
   By default, Ninja is used. To switch to Unix Makefiles, add the following option:
   ``-- -G "Unix Makefiles"``


Flashing the Application
------------------------

.. _flashing_the_application:

The Alif Security Toolkit provides tools to merge Zephyr images and binaries from
different cores. Ensure your user has sufficient access to the ``dialout`` group
for SE-UART device communication.

1. Verify ``dialout`` group membership:

   .. code-block:: console

      groups

   If ``dialout`` or ``tty`` is not listed, add the user to the ``dialout`` group:

   .. code-block:: console

      sudo usermod -a -G dialout $USER

   .. note::
      If the device is still not recognized, check for loose connections or try a
      different USB port.

2. Install the ``fdt`` Python module:

   .. code-block:: console

      pip install fdt

3. Set up the Alif Security Toolkit:

   Extract the toolkit and export its directory before running ``west flash``.
   Ensure the toolkit is properly configured for your DevKit.

   .. code-block:: console

      export ALIF_SE_TOOLS_DIR=path/of/extracted/directory

4. Set device permissions:

   .. note::

      Replace `/dev/ttyACM0` with the appropriate port for your board.
      This step prevents “permission denied” errors during flashing.

   .. code-block:: console

      sudo chmod 666 /dev/ttyACM0

5. Flash the application:

   From the Zephyr build directory, rebuild the binary and flash it to your board:

   .. code-block:: console

      west flash

6. The application boots automatically:

   a. Open a serial console application on the host PC with a baud rate of 115200.

   b. Select the appropriate tty port:

      - For RTSS-HE: Example: ``/dev/ttyACM1``
      - For RTSS-HP: Example: ``/dev/ttyUSB0``

   c. Observe the greeting on the serial console:

      .. code-block:: console

         *** Booting Zephyr OS build 4b48dd532761 ****
         Hello World! alif_e7_devkit
