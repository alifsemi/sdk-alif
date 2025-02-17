**Building Application Binaries**
=================================

Follow this guide to:

- Set up the host system on Ubuntu 20.04 LTS or later.
- Build a sample application

.. note::

   The examples in this document make use of Ensemble E7 Devkit, unless otherwise specified.


Setting up Host System
----------------------

Follow these commands to install dependencies, configure the environment, and prepare the Zephyr SDK and toolchain.

.. note::

   If you are using a virtual environment, make sure to install the Python dependencies within that isolated environment to manage dependencies more effectively.
   For more information, refer to the `Python virtual environment`_ documentation.

   .. _Python virtual environment: https://docs.python.org/3/library/venv.html

1. Update the package list:

   .. code-block:: console

      sudo apt update -y

2. Install required dependencies using `apt`:

   .. code-block:: console

      sudo apt install python3-dev -y
      sudo apt install -y --no-install-recommends python3-pip git \
      wget xz-utils file make python3-setuptools python3-wheel ninja-build \
      build-essential

3. Install Python packages:

   .. code-block:: console

      python3 -m pip install west pyelftools
      pip3 install cmake

4. Add the local bin to the PATH environment variable:

   .. code-block:: console

      export PATH=$PATH:$HOME/.local/bin

5. Download the Zephyr SDK:

   .. code-block:: console

      wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.5/zephyr-sdk-0.16.5_linux-x86_64_minimal.tar.xz

6. Extract the SDK:

   .. code-block:: console

      tar -xf zephyr-sdk-0.16.5_linux-x86_64_minimal.tar.xz

7. Set up the Zephyr toolchain:

   .. code-block:: console

      cd zephyr-sdk-0.16.5
      ./setup.sh -t arm-zephyr-eabi -h -c

8. Return to the previous directory:

   .. code-block:: console

      cd ..

Fetch the SDK
-------------

This section explains building Zephyr using the GCC toolchain.

For details on how the toolchain selection is done, refer to Zephyr's documentation: `Toolchain Selection`_.

1. Fetch the Alif Zephyr SDK Source
***********************************

Fetch the SDK source from the `main` branch:

.. code-block:: console

   mkdir sdk-alif
   cd sdk-alif
   west init -m https://github.com/alifsemi/sdk-alif.git --mr main
   west update

2. Build an Application
***********************

Supported Targets:

- alif_e3_dk_rtss_he
- alif_e3_dk_rtss_hp
- alif_e7_dk_rtss_he
- alif_e7_dk_rtss_hp
- alif_e1c_dk_rtss_he
- alif_b1_dk_rtss_he

.. note::
   The `alif_e7_dk_rtss_he` and `alif_e7_dk_rtss_hp` can also be run from Devkit E5.

a. Navigate to the Zephyr Directory

   .. code-block:: console

      cd zephyr

b. Build the HelloWorld Application

   An application that prints a "Hello World" message along with the board name. By default, code execution takes place from MRAM.

**RTSS-HE**

- Build for MRAM (Address: 0x80000000):

  .. code-block:: console

     west build -b alif_e7_dk_rtss_he samples/hello_world

- Build for ITCM:

  .. code-block:: console

     west build -b alif_e7_dk_rtss_he samples/hello_world -DCONFIG_FLASH_BASE_ADDRESS=0 -DCONFIG_FLASH_LOAD_OFFSET=0 -DCONFIG_FLASH_SIZE=256

**RTSS-HP**

- Build for MRAM (Address: 0x80200000):

  .. code-block:: console

     west build -b alif_e7_dk_rtss_hp samples/hello_world

- Build for ITCM:

  .. code-block:: console

     west build -b alif_e7_dk_rtss_hp samples/hello_world -DCONFIG_FLASH_BASE_ADDRESS=0 -DCONFIG_FLASH_LOAD_OFFSET=0 -DCONFIG_FLASH_SIZE=256

.. note::
   By default, Ninja is used. To switch to using Unix Makefiles, add the following option:
   ``-- -G "Unix Makefiles"``

c. Save the Binaries

**RTSS-HE**

.. code-block:: console

   cp build/zephyr/zephyr.bin YOUR_WORKSPACE/app-release-exec-linux/build/images/zephyr_e7_rtsshe_helloworld.bin

**RTSS-HP**

.. code-block:: console

   cp build/zephyr/zephyr.bin YOUR_WORKSPACE/app-release-exec-linux/build/images/zephyr_e7_rtsshp_helloworld.bin

To verify booting, program MRAM as described in :ref:`programming_an_application`.

.. note::
   This example assumes that the pre-built binaries delivered from Alif are located at YOUR_WORKSPACE.
