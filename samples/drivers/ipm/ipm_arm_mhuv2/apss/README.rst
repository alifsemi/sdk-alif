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

This sample has been tested on :ref:`devkit_e7_apss`, using
Alif DevKit Ensemble family boards.

Building
********

The code can be found in :zephyr_file:`samples/drivers/ipm/ipm_arm_mhuv2`.

To build the application:


1. Building SE services testcase.

.. code-block::

   west build -b devkit_e7_apss samples/drivers/ipm/ipm_arm_mhuv2 -- -G"Unix Makefiles"

2. Building testcase to exchange messages between two processors.

.. code-block::
   west build -b devkit_e7_apss samples/drivers/ipm/ipm_arm_mhuv2 -- -G"Unix Makefiles" -DRTSS_HE_MHU0=ON
   west build -b devkit_e7_apss samples/drivers/ipm/ipm_arm_mhuv2 -- -G"Unix Makefiles" -DRTSS_HE_MHU1=ON
   west build -b devkit_e7_apss samples/drivers/ipm/ipm_arm_mhuv2 -- -G"Unix Makefiles" -DRTSS_HP_MHU0=ON
   west build -b devkit_e7_apss samples/drivers/ipm/ipm_arm_mhuv2 -- -G"Unix Makefiles" -DRTSS_HP_MHU1=ON

Sample output
*************

.. code-block:: none
   *** Booting Zephyr OS build Zephyr-E7-APSS-v0.1.0-Beta-1-gf8d409152444 *
   MHU SE services example on devkit_e7_devboard
   Heartbeat test ...
   heartbeat = 0
   service ID is 0
   Heartbeat test done successfullyRND test ...
   RND test case
   service ID is 400
   RND numbers are below
   0xf2
   0xae
   0x6e
   0x7a
   0x1d
   0x71
   0x74
   0x90
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
   SES A1 EVALUATION_BOARD SE_FW_0.64.000_DEV v0.64.0 Jan 15 2023 03:14:0
   uart test ...
   service ID is 104
   UART message sent successfully
   Done
   Get TOC data test
   In service_get_toc_data
   service ID is 205
   TOC number of entries is 7
