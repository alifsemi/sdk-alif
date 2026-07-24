.. _setting_up_and_building_zephyr:

Setting Up and Building Zephyr Applications
===========================================

This guide covers:

- Setting up the host system.
- Fetching and installing the Alif Zephyr SDK.
- Building a sample application.
- Flashing and debugging an application.

.. note::
   Examples in this document use the Alif E7 DevKit unless otherwise specified.

Setting Up the Host System
--------------------------

Follow these steps to install dependencies and configure the environment for the Alif SDK.

1. Update the package list:

   .. code-block:: console

      sudo apt update -y

2. Install the required dependencies:

   .. code-block:: console

      sudo apt install -y --no-install-recommends \
         python3-dev python3-pip python3-venv \
         git wget xz-utils file make \
         python3-setuptools python3-wheel \
         ninja-build build-essential cmake libmagic1

3. Create and activate a Python virtual environment.

   a. Create the virtual environment:

      .. code-block:: console

         python3 -m venv ~/zephyrproject/.venv

   b. Activate the virtual environment:

      .. code-block:: console

         source ~/zephyrproject/.venv/bin/activate

      Your shell prompt displays ``(.venv)`` when the environment is active.

      .. note::

         Activate the virtual environment in every new terminal session before
         running Python or ``west`` commands. When finished, deactivate it by
         running:

         .. code-block:: console

            deactivate

4. Install the required Python packages:

   .. code-block:: console

      pip install west pyelftools


Fetching and Installing the Alif Zephyr SDK
-------------------------------------------

This section explains how to fetch the Alif Zephyr SDK and install the required
components for building Zephyr applications. For information about supported
toolchains, refer to Zephyr's documentation:
`Toolchain Selection <https://docs.zephyrproject.org/latest/develop/toolchains/index.html>`_.

1. Create a workspace:

   .. code-block:: console

      mkdir sdk-alif
      cd sdk-alif

2. Initialize the workspace:

   .. code-block:: console

      west init -m https://github.com/alifsemi/sdk-alif --mr ${REVISION}

   Replace ``${REVISION}`` with the required SDK revision (branch, tag, or
   commit SHA). Use ``main`` for the latest development branch.

   **Syntax:**

   .. code-block:: console

      west init -m <URL> --mr <REVISION>

   **Example:**

   .. code-block:: console

      west init -m https://github.com/alifsemi/sdk-alif --mr v2.2-zas-branch

   .. note::

      If ``west init`` fails because an incorrect revision was specified,
      the ``.west`` workspace may still be created. Remove it before retrying.

      .. code-block:: console

         rm -rf .west

3. Fetch all dependency repositories defined in the west manifest:

   .. code-block:: console

      west update

   .. note::

      To switch to a different SDK branch after the workspace has been
      initialized, navigate to the alif repository directory within the
      workspace:

      .. code-block:: console

         cd alif
         git fetch
         git checkout <branch-name>
         git pull
         west update

4. Install the Python packages required by Zephyr's build system.

   These packages are used by Zephyr for building, flashing, and debugging.

   .. code-block:: console

      pip install -r zephyr/scripts/requirements.txt

5. Install the Zephyr SDK.

   This installs the GCC cross-compilation toolchain required to build Zephyr
   applications.

   .. code-block:: console

      west sdk install


Building an Application
-----------------------

Supported Board Targets as per new hardware model v2 (Zephyr v4.1.0 and onwards):

::

   <board name[@revision][/board qualifiers]>

- alif_e7_dk/ae722f80f55d5xx/rtss_he
- alif_e7_dk/ae722f80f55d5xx/rtss_hp
- alif_e7_dk/ae722f80f55d5xx/apss
- alif_e7_dk/ae302f80f55d5xx/rtss_he
- alif_e7_dk/ae302f80f55d5xx/rtss_hp
- alif_e7_ak/ae722f80f55d5xx/rtss_he
- alif_e7_ak/ae722f80f55d5xx/rtss_hp
- alif_e8_dk/ae822fa0e5597xx0/apss
- alif_e8_dk/ae822fa0e5597xx0/rtss_he
- alif_e8_dk/ae822fa0e5597xx0/rtss_hp
- alif_e8_dk/ae402fa0e5597xx0/rtss_he
- alif_e8_dk/ae402fa0e5597xx0/rtss_hp
- alif_e8_ak/ae822fa0e5597xx0/rtss_he
- alif_e8_ak/ae822fa0e5597xx0/rtss_hp
- alif_e1c_dk/ae1c1f4051920hh/rtss_he
- alif_b1_dk/ab1c1f4m51820ph0/rtss_he
- alif_b1_dk/ab1c1f1m41820hh0/rtss_he
- alif_b1_dk/ab1c1f1m41820ph0/rtss_he

a. Navigate to the Zephyr directory:

   .. code-block:: console

      cd zephyr

b. Build the Hello World application.

   The Hello World application prints a greeting message along with the board
   name. By default, the application executes from MRAM.

**RTSS-HE**

- Build for MRAM (Address: ``0x80000000``):

  .. code-block:: console

     west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/hello_world

- Build for ITCM:

  .. code-block:: console

     west build -p always \
       -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       samples/hello_world \
       -DCONFIG_FLASH_BASE_ADDRESS=0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0 \
       -DCONFIG_FLASH_SIZE=256

**RTSS-HP**

- Build for MRAM (Address: ``0x80200000``):

  .. code-block:: console

     west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/hello_world

- Build for ITCM:

  .. code-block:: console

     west build -p always \
       -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       samples/hello_world \
       -DCONFIG_FLASH_BASE_ADDRESS=0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0 \
       -DCONFIG_FLASH_SIZE=256

.. note::

   Ninja is used as the default build system. To use Unix Makefiles instead,
   append the following option to the ``west build`` command:

   .. code-block:: console

      -- -G "Unix Makefiles"


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

   **Example:**

   .. code-block:: console

      export ALIF_SE_TOOLS_DIR=/home/<username>/app-release-exec-linux-SE_FW_1.111.00_DEV

4. Set device permissions:

   .. note::

      Replace ``/dev/ttyACM0`` with the appropriate SE-UART port for your board.
      This step prevents "permission denied" errors during flashing.

   .. code-block:: console

      sudo chmod 666 /dev/ttyACM0

5. Flash the application:

   From the Zephyr build directory, rebuild the binary and flash it to your board:

   .. code-block:: console

      west flash

   .. note::

      If the SE-UART port is different, use the ``--com-port`` option.

      **Example:**

   .. code-block:: console

      west flash --com-port=ACM0

6. Debug the application:

   From the zephyr build directory, start a debug session on your board:

   .. code-block:: console

      west debug

7. The application boots automatically:

   a. Open a serial console application on the host PC with a baud rate of 115200.

   b. Select the appropriate tty port:

      - For RTSS-HE: Example: ``/dev/ttyACM1``
      - For RTSS-HP: Example: ``/dev/ttyUSB0``

   c. Observe the greeting on the serial console:

      .. code-block:: console

         *** Booting Zephyr OS build 4b48dd532761 ****
         Hello World! alif_e7_devkit
