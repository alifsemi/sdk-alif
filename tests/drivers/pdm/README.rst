.. _pdm-test:

PDM and LpPDM TestCode
######################

Overview
********

This directory contains functional tests for the PDM/LpPDM
driver on Alif Semiconductor boards.

Supported Boards:
	``alif_b1_dk/ab1c1f4m51820hh0/rtss_he``
	``alif_e1c_dk/ae1c1f4051920hh/rtss_he``
	``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
	``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
	``alif_e7_dk/ae722f80f55d5xx/rtss_he``
	``alif_e7_dk/ae722f80f55d5xx/rtss_hp``

Building and Running
********************

The application will build only for a target that has a devicetree entry with
:dtcompatible:`alif,alif-pdm` as a compatible.

Select one snippet per build (``DT_ALIAS(test_pdm)``):

* ``-S pdm``: E7 / E8 RTSS_HE and RTSS_HP → ``&pdm``
  (``alif_pdm_rtss_he.overlay`` / ``alif_pdm_rtss_hp.overlay``)
* ``-S lppdm``: B1 / E1C / E7 / E8 RTSS_HE → ``&lppdm``
  (``alif_lppdm_rtss_he.overlay``)

E7 / E8 HE have both instances; use separate builds. B1 / E1C HE have ``&lppdm``
only.

How to Run
----------

**Example build commands**

**Mono Channel build commands**

.. code-block:: bash

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/tests/drivers/alif_pdm -S pdm \
     -DCONFIG_TEST_PDM_MONO_CH=y -DCONFIG_TEST_PDM_CH0=y -DCONFIG_TEST_PDM_MODE1=y

 **HE build commands**

 .. code-block:: bash

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/tests/drivers/alif_pdm -S lppdm \
     -DCONFIG_TEST_PDM_MONO_CH=y -DCONFIG_TEST_PDM_CH0=y -DCONFIG_TEST_PDM_MODE1=y
     
To run mono channel testcases set CONFIG_TEST_PDM_MONO_CH=y and
CONFIG_TEST_PDM_CHx=y to select the channel.

.. note::
   Here x is the channel number. For CH0 use CONFIG_TEST_PDM_CH0=y.
   CH0 - CH7 (8 channels) can be configured.

**Stereo Channel build commands**

.. code-block:: bash

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/tests/drivers/alif_pdm -S pdm \
     -DCONFIG_TEST_PDM_STEREO_CH=y -DCONFIG_TEST_PDM_CH0=y -DCONFIG_TEST_PDM_CH1=y \
     -DCONFIG_TEST_PDM_MODE1=y

To run stereo channel testcases set CONFIG_TEST_PDM_STEREO_CH=y and
CONFIG_TEST_PDM_CHx=y CONFIG_TEST_PDM_CHy=y to select the channels.

.. note::
   Stereo means 2 channels. x is the first channel (CONFIG_TEST_PDM_CH0=y)
   and y is the second (CONFIG_TEST_PDM_CH1=y).
   Valid pairs: (0,1) (2,3) (4,5) (6,7).

**Multi Channel build commands**

.. code-block:: bash

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/tests/drivers/alif_pdm -S pdm \
     -DCONFIG_TEST_PDM_MULTI_CH=y -DCONFIG_TEST_PDM_CH0=y -DCONFIG_TEST_PDM_CH1=y \
     -DCONFIG_TEST_PDM_CH4=y -DCONFIG_TEST_PDM_CH5=y -DCONFIG_TEST_PDM_MODE6=y

To run multi channel testcases set CONFIG_TEST_PDM_MULTI_CH=y and enable ANY 3..8
channels via CONFIG_TEST_PDM_CHx=y. The channel count and map are derived from
the enabled channels.

.. note::
   Multi means 3..8 channels. Enable exactly the channels you wire, e.g.
   (0,1,2,3) (4,5,6,7) (0,1,4,5) (0,1,2,3,4,5). Multiple mics/boards are
   generally required.

Selecting the PDM Mode
----------------------

The PDM clock/sampling mode applies to all channels. Select exactly one mode \
per build with CONFIG_TEST_PDM_MODEx=y (x = 1..9). A mode flag is required; \
there is no default.
.. note::
        MODE0 (microphone sleep) produces no audio and is not part of the test matrix.

Generated Test Names (TestLink mapping)
----------------------------------------

Each capture build registers exactly ONE ztest. The suite follows the
``test-pdm`` alias (not the core):

* ``&pdm``  (``-S pdm``)   -> suite ``test_pdm``
* ``&lppdm`` (``-S lppdm``) -> suite ``test_lppdm``

The case name is generated at compile time from the active group, mode and enabled
channels:

``<suite>_<group>_mode<N>_ch[_<n>...]``

where <group> is ``mono`` / ``stereo`` / ``multi``, <N> is the mode number, and each
enabled channel appends ``_<n>``. Examples:

* ``-S pdm``,  HP,  mono, MODE1, CH3              -> ``test_pdm_mono_mode1_ch_3``
* ``-S pdm``,  E7 HE, stereo, MODE6, CH4 CH5      -> ``test_pdm_stereo_mode6_ch_4_5``
* ``-S lppdm``, HE, stereo, MODE6, CH4 CH5        -> ``test_lppdm_stereo_mode6_ch_4_5``
* ``-S pdm``,  HP,  multi, MODE6, CH0 CH1 CH4 CH5 -> ``test_pdm_multi_mode6_ch_0_1_4_5``
* ``-S lppdm``, HE, multi, MODE6, CH0 CH1 CH4 CH5 -> ``test_lppdm_multi_mode6_ch_0_1_4_5``

Because every distinct combination yields a distinct, deterministic name, the result \
string can be mapped one-to-one to a TestLink test case.

Validation
----------

A test PASSes when the driver API succeeds end to end: ``dmic_configure``, \
``dmic_trigger`` and ``dmic_read`` all return success and a full block is received \
without timeout/overflow. The first captured block is discarded to skip the PDM \
filter settling time. Captured audio content is logged as informational \
``Signal stats: ...`` (min/max/non-zero/first-nonzero) but never fails the test, \
since true silence is valid - play live audio during the run to see non-zero stats.

