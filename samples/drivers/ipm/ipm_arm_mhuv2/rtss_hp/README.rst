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

Building and Running
********************

This sample will build only for a target that has a devicetree entry
with :dt compatible:`arm,mhuv2` as a compatible string.
It does not work on QEMU.
In this example below the :ref:`alif_e7_dk/ae722f80f55d5xx/rtss_hp` board is used.

1. Building SE services testcase.

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/ipm/ipm_arm_mhuv2
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_hp
   :goals: build

Sample output
*************

.. code-block:: console
    SE Service test on alif_e7_dk
    !!!Read board config using SE Services!!!
    Device part number is 46080
    Revision is SES B4 v1.102.0 Nov  8 2024 17:52:19
    Revision ID = 46080 (0xb400)
    Alif PN = 414537323246383046353544354c530
    Serial Number = 00000000
    HBK0 = f049442ecc790a838d34c31d71e62e6
    DCU settings = 0000000000000000
    config = c1000
    HBK1 = 0000000000000000
    HBK_FW = 00000000000000000000
    MfgData = 4135700017000000000017807c0d88303c0a8000c
    LCS = 1 (0x1)
    !!!Board config read successfully!!!

2. Building MHU0 sample application to exchange messages
   between two processors (RTSS_HE and RTSS_HP)

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/ipm/ipm_arm_mhuv2
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_hp
   :goals: build
   :gen-args: -DRTSS_HE_MHU0=ON

Sample output
*************
    sample MHU0 output between RTSS_HP and RTSS_HE

.. code-block:: console
    RTSS-HP RTSS-HE MHU 0 example on alif_e7_dk
    RTSS-HP: MSG sent on Ch:0 is 0x12345678
    RTSS-HP: MSG rcvd on ch:0 is 0xaddedace
    RTSS-HP: MSG sent on Ch:1 is 0xa5a5fafa
    RTSS-HP: MSG rcvd on ch:1 is 0xbeadbead
    RTSS-HP: MSG sent on Ch:0 is 0x12345678
    RTSS-HP: MSG rcvd on ch:0 is 0xaddedace
    RTSS-HP: MSG sent on Ch:1 is 0xa5a5fafa
    RTSS-HP: MSG rcvd on ch:1 is 0xbeadbead
    ...
