**Building Application Binaries**
=================================

Follow this guide to:

- Set up the host system on Ubuntu 20.04 LTS or later.

- Build a sample application

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

5. Download and extract the ARM Zephyr toolchain:

   .. code-block:: console

      wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.5/toolchain_linux-x86_64_arm-zephyr-eabi.tar.xz
      tar -xf toolchain_linux-x86_64_arm-zephyr-eabi.tar.xz

6. Update the PATH environment variable for the ARM Zephyr toolchain:

   .. code-block:: console

      export PATH=$PATH:`pwd`/arm-zephyr-eabi/bin

7. Download the Zephyr SDK:

   .. code-block:: console

      wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.5/zephyr-sdk-0.16.5_linux-x86_64_minimal.tar.xz

8. Extract the SDK:

   .. code-block:: console

      tar -xf zephyr-sdk-0.16.5_linux-x86_64_minimal.tar.xz

9. Set up the Zephyr toolchain:

   .. code-block:: console

      cd zephyr-sdk-0.16.5
      ./setup.sh -h -c

10. Return to the previous directory:

    .. code-block:: console

       cd ..

Fetch the SDK
-------------

For details on how the toolchain selection is done, refer Zephyr's documentation: `Toolchain Selection`_.

This section explains building Zephyr using the GCC toolchain.

1. Fetch the Alif Zephyr SDK source from the `main` branch from the following location: `Alif Zephyr SDK Source`_.

   .. code-block:: console

       mkdir sdk-alif
       cd sdk-alif
       west init -m https://github.com/alifsemi/sdk-alif.git --mr main
       west update

2. Build an Application

      Supported Targets

      - alif_e3_dk_rtss_he

      - alif_e3_dk_rtss_hp

      - alif_e7_dk_rtss_he

      - alif_e7_dk_rtss_hp


   a. Navigate to the Zephyr directory:

      .. code-block:: console

          cd zephyr

   b. Build HelloWorld Application

      An application that prints a Hello World message along with the board name.

      **RTSS-HE**

	Build for ITCM :

        .. code-block:: console

            west build -b alif_e7_dk_rtss_he samples/hello_world

        Build for MRAM : (Address : 0x80000000)

        .. code-block:: console

            west build -b alif_e7_dk_rtss_he samples/hello_world -DCONFIG_ROM_ITCM=n


      **RTSS-HP**

	Build for ITCM :

        .. code-block:: console

            west build -b alif_e7_dk_rtss_hp samples/hello_world

        Build for MRAM : (Address : 0x80200000)

	.. code-block:: console

    	   west build -b alif_e7_dk_rtss_hp samples/hello_world -DCONFIG_ROM_ITCM=n


      .. note::
         By default, Ninja is used.
	 To switch to using Unix Makefiles, add the following option:
	 ``-- -G "Unix Makefiles"``


   c. Save the binaries

     **RTSS-HE**

        .. code-block:: console

               cp build/zephyr/zephyr.bin YOUR_WORKSPACE/app-release-exec-linux/build/images/zephyr_e7_rtsshe_helloworld.bin

     **RTSS-HP**

        .. code-block:: console

               cp build/zephyr/zephyr.bin YOUR_WORKSPACE/app-release-exec-linux/build/images/zephyr_e7_rtsshp_helloworld.bin


   To verify booting, program MRAM as described in :ref:`programming_an_application`.


