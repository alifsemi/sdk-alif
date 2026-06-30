.. _setting_up_and_building_zephyr:

Setting Up and Building Zephyr Applications
===========================================

This guide covers:

- Building a sample application.

.. note::
   Examples in this document use the Ensemble E7 DevKit unless otherwise specified.

Setting Up the Host System
------------------------------

Follow these steps to install dependencies and configure the environment for the Alif SDK.

1. Update the package list:

   .. code-block:: console

      sudo apt update -y

2. Install required dependencies using ``apt``:

   .. code-block:: console

     sudo apt install -y --no-install-recommends \
     python3-dev python3-pip python3-venv \
     git wget xz-utils file make \
     python3-setuptools python3-wheel \
     ninja-build build-essential cmake libmagic1


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

Fetching and installing the Alif Zephyr SDK
----------------------------------------------

This section explains how to build Zephyr using the GCC toolchain. For details on
toolchain selection, refer to Zephyr's documentation:
`Toolchain Selection <https://docs.zephyrproject.org/latest/develop/toolchains/index.html>`_.

Fetch the SDK source from the ``main`` branch:

.. code-block:: console

   mkdir sdk-alif
   cd sdk-alif
   west init -m https://github.com/alifsemi/sdk-alif --mr ${revision}
   west update

Replace ``${revision}`` with the desired SDK revision (branch, tag, or commit SHA).
Use ``main`` for the latest state, or specify a commit SHA or tag.

**Syntax:**

.. code-block:: console

   west init -m <URL> --mr <REVISION>

**Example:**

.. code-block:: console

   west init -m https://github.com/alifsemi/sdk-alif --mr v2.2.0-zas-branch

**Install required Python packages for building:**

.. code-block:: console

   pip install -r zephyr/scripts/requirements.txt

**Install sdk:**

.. code-block:: console

   west sdk install


Building an Application
-----------------------

Supported Board Targets as per new hardware model v2 (Zephyr v4.1.0 and onwards):
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

b. Build the Hello World application:

   An application that prints a "Hello World" message along with the board name.
   By default, code execution takes place from MRAM.

**RTSS-HE**

- Build for MRAM (Address: 0x80000000):

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

- Build for MRAM (Address: 0x80200000):

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


Advanced Multicore Flash Support (Ensemble DevKit Only)
-----------------------------------------------------------

For multicore applications on the Alif Ensemble DevKit, additional binaries
can be programmed for multiple CPU cores in a single flash operation.

The supported CPU cores are:

- Cortex-M55 HE
- Cortex-M55 HP
- Cortex-A32

**CPU Binary Options**

The following example programs the binary generated by the current build target
along with the specified HP binary.

Example:

.. code-block:: console

   west flash --hp-bin ./myprebuild_bins/hp.bin

Use the following options to specify binaries for CPU cores other than the
current build target.

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Option
     - Description
   * - ``--he-bin <file>``
     - Specifies the binary for the Cortex-M55 HE core.
   * - ``--hp-bin <file>``
     - Specifies the binary for the Cortex-M55 HP core.
   * - ``--a32-bin <file>``
     - Specifies the binary for the Cortex-A32 core.

.. note::

   The binary generated by the current build target is always programmed.
   Additional CPU binaries can be programmed using the options listed above.

**MRAM Boot**

The following example programs the specified HP binary and boots it directly
from MRAM.

Example:

.. code-block:: console

   west flash \
     --hp-bin ./myprebuild_bins/hp.bin --hp-mram-boot 1

Use the corresponding core-specific ``--*-mram-boot`` option with a value of ``1`` (for example, ``--hp-mram-boot 1``) to boot that core's application directly from MRAM.
For Cortex-A32, the Zephyr image executes XIP from MRAM by design.

The default MRAM boot addresses are:

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - CPU Core
     - Default MRAM Boot Address
   * - Cortex-M55 HE
     - ``0x80000000``
   * - Cortex-A32
     - ``0x80100000``
   * - Cortex-M55 HP
     - ``0x80200000``

**ITCM Boot**

The following example demonstrates the default boot behavior. Since the
``--hp-mram-boot`` option is not specified, the specified HP binary is loaded
into TCM and boots using the default boot mode.

Example:

.. code-block:: console

   west flash --hp-bin ./myprebuild_bins/hp.bin

By default, additional CPU binaries provided via ``--*-bin`` are loaded into TCM and booted from it.
The corresponding ``--*-mram-boot 1`` option is only required when booting a specified CPU binary directly from MRAM.

The default TCM load addresses are:

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - CPU Core
     - Default TCM Load Address
   * - Cortex-M55 HE
     - ``0x58000000``
   * - Cortex-M55 HP
     - ``0x50000000``

.. note::

   Specify ``--*-mram-boot 1`` only when booting an application directly from
   MRAM. Otherwise, the application is loaded into TCM and boots using the
   default boot mode.

**Console Output**

After flashing, the programmed applications boot automatically. Open the
appropriate serial console for each CPU core to observe the application output.

When multiple CPU binaries are programmed, the console output from each
application can be observed on its corresponding serial port.

