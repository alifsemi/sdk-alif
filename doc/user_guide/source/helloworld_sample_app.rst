HelloWorld Sample Application
=============================

This section discusses building the HelloWorld application which prints a "Hello World" message along with the board name. The application is run from MRAM.

For RTSS-HE, the MRAM boot address is 0x80000000 and for RTSS-HP, the MRAM boot address is 0x80200000.

Supported Targets
-----------------
- alif_e3_dk_rtss_he
- alif_e3_dk_rtss_hp
- alif_e7_dk_rtss_he
- alif_e7_dk_rtss_hp

Build for RTSS-HE
-----------------

.. code-block:: console

    west build -b alif_e7_dk_rtss_he samples/hello_world/ -DCONFIG_ROM_ITCM=n

.. note::

   This application runs from MRAM 0x80000000

Build for RTSS-HP
-----------------

.. code-block:: console

   west build -b alif_e7_dk_rtss_hp samples/hello_world/ -DCONFIG_ROM_ITCM=n

.. note::

   This application runs from MRAM 0x80200000.

.. note::

   **Building with CMake System:**

   To build the application using CMake, add ``-- -G "Unix Makefiles"`` to the Ninja command line, as shown below:


