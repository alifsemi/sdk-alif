.. _ipm_ipm_arm_mhuv2_sample:

MHUv2 test Sample
#################

Overview
********

This sample demonstrates how to use Arm Message Handling Unit(MHUv2) for
communicating messages/request services on a heterogeneous architecture
like Alif DevKit Ensemble family of devices. The MHUv2 in the
Arm Corestone-700 architecture supports full duplex message communication,
that has dedicated channels for transmission and reception. The messages
can be communicated along two channels using two message handling units
MHU0 and MHU1 simultaneously.

The testcases,
 * exchange data between two different processors using MHUv2 through
   interrupts.
 * request service (info )from root of trust processor such as Secure Enclave.

Requirements
************

The sample uses the MHUv2 of Corestone 700 architecture to send or
receive messages on the dedicated sender and receiver channels.
Confirmation of MHU messages sent or received are intimated through
interrupts and registered callback functions are called for usage.

This sample has been tested on :ref:`alif_e7_dk_rtss_hp`, using
Alif DevKit Ensemble family board.

Supported Targets
*****************
* alif_e3_dk_rtss_hp
* alif_e7_dk_rtss_hp

Building
********

The code can be found in :zephyr_file:`samples/drivers/ipm/ipm_arm_mhuv2`.

:ref:`alif_e7_dk_rtss_hp`

To build the application:

Building SE services testcase.

.. code-block::

   west build -b alif_e7_dk_rtss_hp samples/drivers/ipm/ipm_arm_mhuv2 -- -G"Unix Makefiles"

Sample output
*************

.. code-block:: none
   *** Booting Zephyr OS build zephyr-v3.0.0-116-g04b52f5c6ba4  ***
   MHU SE services example on alif_e7_dk_rtss_hp
   Heartbeat test ...
   heartbeat = 0
   service ID is 0
   Heartbeat test done successfullyRND test ...
   RND test case
   service ID is 400
   RND numbers are below
   0x1a
   0x2e
   0xe9
   0xec
   0x9b
   0xbf
   0x36
   0x06
   LCS test ...
   lcs = 401
   service ID is 401
   LCS response is 0x0
   TOC version test
   toc = 200
   service ID is 200
   The TOC version is 0x0
   TOC number test
   number = 201
   service ID is 201
   TOC no. 3
   SE revision test
   In service_get_se_revision
   service ID is 103
   SE Revision is below
   SES A1 EVALUATION_BOARD SE_FW_0.64.000_DEV v0.64.0 Jan 15 2023 03:14:02
   uart test ...
   service ID is 104
   UART message sent successfully
   Done
   Get TOC data test
   In service_get_toc_data
   service ID is 205
   TOC number of entries is 7

:ref:`devkit_e7_rtss_hp`
1. Building SE services testcase.

.. code-block::

   west build -b alif_e7_dk_rtss_hp samples/drivers/ipm/ipm_arm_mhuv2 -- -G"Unix Makefiles"

2. Building testcase to exchange messages between two processors.

.. code-block::
   west build -b alif_e7_dk_rtss_hp samples/drivers/ipm/ipm_arm_mhuv2 -- -G"Unix Makefiles" -DAPSS_MHU0=ON
   west build -b alif_e7_dk_rtss_hp samples/drivers/ipm/ipm_arm_mhuv2 -- -G"Unix Makefiles" -DAPSS_MHU1=ON
   west build -b alif_e7_dk_rtss_hp samples/drivers/ipm/ipm_arm_mhuv2 -- -G"Unix Makefiles" -DRTSS_HE_MHU0=ON
   west build -b alif_e7_dk_rtss_hp samples/drivers/ipm/ipm_arm_mhuv2 -- -G"Unix Makefiles" -DRTSS_HE_MHU1=ON

Sample output
*************

.. code-block:: none
   *** Booting Zephyr OS build zephyr-v3.0.0-116-g04b52f5c6ba4  ***
   MHU SE services example on alif_e7_dk_rtss_hp
   Heartbeat test ...
   heartbeat = 0
   service ID is 0
   Heartbeat test done successfullyRND test ...
   RND test case
   service ID is 400
   RND numbers are below
   0x1a
   0x2e
   0xe9
   0xec
   0x9b
   0xbf
   0x36
   0x06
   LCS test ...
   lcs = 401
   service ID is 401
   LCS response is 0x0
   TOC version test
   toc = 200
   service ID is 200
   The TOC version is 0x0
   TOC number test
   number = 201
   service ID is 201
   TOC no. 3
   SE revision test
   In service_get_se_revision
   service ID is 103
   SE Revision is below
   SES A1 EVALUATION_BOARD SE_FW_0.64.000_DEV v0.64.0 Jan 15 2023 03:14:02
   uart test ...
   service ID is 104
   UART message sent successfully
   Done
   Get TOC data test
   In service_get_toc_data
   service ID is 205
   TOC number of entries is 7
