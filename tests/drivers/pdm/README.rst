.. _pdm-test:

PDM and LpPDM TestCode
######################

Overview
********

This directory contains functional tests for the PDM/LpPDM
driver on Alif Semiconductor boards.

Supported Boards:
	``alif_b1_dk/ab1c1f4m51820hh/rtss_he``
	``alif_e1c_dk/ae1c1f4051920hh/rtss_he``
	``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
	``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
	``alif_e7_dk/ae722f80f55d5xx/rtss_he``
	``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
	``alif_e4_dk/ae402fa0e5597xx0/rtss_he``
	``alif_e4_dk/ae402fa0e5597xx0/rtss_hp``
	``alif_e3_dk/ae302f80f55d5xx/rtss_he``
	``alif_e3_dk/ae302f80f55d5xx/rtss_hp``

Building and Running
********************

The application will build only for a target that has a devicetree entry with
*:dt compatible:`alif,alif-pdm`* as a compatible.
*The tests assume that the PDM node (``pdm``)  is available as ``&pdm``
	on RTSS_HP cores and aliased accordingly through a board overlay.
*The tests assume that the PDM node (``lppdm``)  is available as ``&lppdm``
        on RTSS_HE core and aliased accordingly through a board overlay.

How to Run
----------

**Example build commands**
.. code-block:: bash
**Mono Channel build commands**

``west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/tests/drivers/pdm -S pdm \
-DCONFIG_TEST_PDM_MONO_CH=y -DCONFIG_TEST_PDM_CH0=y``

To run mono channel testcases use set config CONFIG_TEST_PDM_MONO_CH=y and  \
CONFIG_TEST_PDM_CHx=y is to select channel.
.. note::
        Here x is channel number if user wants to run CH0 he has to select \
        CONFIG_TEST_PDM_CH0=y
        CH0 - CH7 total 8 channels can configure to test

**Stereo Channel build commands**

``west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/tests/drivers/pdm -S pdm \
-DCONFIG_TEST_PDM_STEREO_CH=y -DCONFIG_TEST_PDM_CH0=y -DCONFIG_TEST_PDM_CH1=y``

To run stereo channel testcases use set config CONFIG_TEST_PDM_STEREO_CH=y and  \
CONFIG_TEST_PDM_CHx=y CONFIG_TEST_PDM_CHy=y is to select channels.
.. note::
        Stereo means 2 channels need to configure. Here x is the first channel \
        CONFIG_TEST_PDM_CH0=y and y is 2nd channel CONFIG_TEST_PDM_CH2=y
        total 4 pair of  channels (0,1) (2,3) (4,5) (6,7) can configure to test

**Multi Channel build commands**

``west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/tests/drivers/pdm -S pdm \
-DCONFIG_TEST_PDM_MULTI_CH=y``

To run multi channel testcases use set config CONFIG_TEST_PDM_MULTI_CH=y and  \
CONFIG_TEST_PDM_CH0=y CONFIG_TEST_PDM_CH1=y CONFIG_TEST_PDM_CH4=y CONFIG_TEST_PDM_CH5=y
.. note::
        Multi means 4 channels (default 4 5 6 7) will  configure. \
        Generally 2 boards required to test multi channel PDM testcases

