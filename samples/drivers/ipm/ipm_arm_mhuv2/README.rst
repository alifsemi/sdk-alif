.. _ipm_arm_mhuv2_sample:

MHUv2 Inter-Processor Messaging Sample
#######################################

Overview
********

This sample demonstrates inter-processor communication using the Arm Message
Handling Unit v2 (MHUv2) on the Alif Ensemble family of devices. The MHUv2
in the Arm Corestone-700 architecture provides full-duplex message
communication with dedicated sender and receiver channels. Messages can be
exchanged simultaneously on two channels using MHU0 and MHU1.

This sample includes test cases for:

* Requesting services from the Secure Enclave (SE) processor, such as
  reading board configuration and device identity information.
* Exchanging register-based MHU messages between two processors
  (M55-HE, M55-HP, and A32/APSS).

Requirements
************

* Alif E8 DevKit (or E7 DevKit for SE services test)
* MHUv2 sender and receiver channels configured in the device tree
* For inter-core tests: both communicating processors must be enabled

Supported Targets
*****************

* alif_e8_dk/ae822fa0e5597xx0/rtss_hp
* alif_e8_dk/ae822fa0e5597xx0/rtss_he

Building
********

The code can be found in :zephyr_file:`samples/drivers/ipm/ipm_arm_mhuv2`.

Select the test case using CMake options:

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - ``SE_SERVICES`` (default: ON)
     - Secure Enclave services test (read board config)
   * - ``APSS_R``
     - M55 responder for messages from A32 (APSS) MHU
   * - ``HE_HP_S``
     - M55-HE to M55-HP MHU initiator/sender
   * - ``HP_HE_R``
     - M55-HP to M55-HE MHU responder
   * - ``HE_A32_S``
     - M55-HE to A32 (APSS) MHU initiator/sender

By default MHU0 is used for communication. To use MHU1 instead, add
``-DCONFIG_USE_MHU1=y`` to the build command.

Building the SE services test case (default):

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/ipm/ipm_arm_mhuv2

Building inter-core MHU message exchange (requires both images):

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhuv2 -DHE_HP_S=ON
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/ipm/ipm_arm_mhuv2 -DHP_HE_R=ON

Building inter-core MHU message exchange, using MHU1 (requires both images):

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhuv2 -DHE_HP_S=ON -DCONFIG_USE_MHU1=y
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/ipm/ipm_arm_mhuv2 -DHP_HE_R=ON -DCONFIG_USE_MHU1=y

Building M55-HE to A32 communication. The M55 (running Zephyr) can either initiate communication with A32 or
respond to communication started from A32:

.. code-block:: bash

   # M55-HE initiates communication to A32
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhuv2 -DHE_A32_S=ON

   # M55-HE responds to communication from A32
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhuv2 -DAPSS_R=ON

Building M55-HE and M55-HP as responders to A32-initiated communication:

This configuration has both M55-HE and M55-HP waiting for MHU messages
initiated by the A32 core running Linux. It requires:

* Both M55-HE and M55-HP Zephyr firmware images
* The A32 Linux test application running on the APSS core
* A different flash base address for M55-HE to avoid overwriting Linux in MRAM

By default, Linux and M55-HE share UART2. To use a separate UART for M55-HE, uncomment the
UART overlay in ``boards/alif_e8_dk_ae822fa0e5597xx0_rtss_he.overlay`` and rebuild.

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/ipm/ipm_arm_mhuv2 -DAPSS_R=ON
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhuv2 -DAPSS_R=ON -DCONFIG_FLASH_BASE_ADDRESS=0x80400000

This example assumes:

* A32 Linux test application from https://github.com/alifsemi/alif_a32_linux_DD_testcases is compiled
  and copied to root filesystem.
* The Linux ``xipImage`` is already flashed to OSPI flash and the root
  filesystem is on the SD card.
* The ``linux_mpu_test_he_hp.json`` configuration file is used with the SE
  tool to program M55-HE, M55-HP, ``bl32.bin``, and ``devkit-e8.dtb`` to MRAM.


Sample Output
*************

SE services test:

.. code-block:: none

   *** Booting Zephyr OS build e9e1b4aa19a4 ***
   !!!Read board config using SE Services!!!
   Device part number is 672
   Revision is SES A0 v1.110.0 Mar  4 2026 19:05:34
   Revision ID = 672 (0x2a0)
   Alif PN = AE822FA0E5597LS0
   Serial Number = 00000000
   HBK0 = cd74ac70972c3e27b8b8f73af34ede
   DCU settings = 0000000000000000
   config = 1000
   HBK1 = 0000000000000000
   HBK_FW = 00000000000000000000
   MfgData = 51176004e0011d000000118041a0daa4000900007c
   LCS = 1 (0x1)
   !!!Board config read successfully!!!


Inter-core MHU message exchange (M55-HE side):

.. code-block:: none

   M55-HE <-> M55-HP MHU0 example on alif_e8_dk
   M55-HE: MSG sent on MHU0 Ch:0 = 0x12345678
   M55-HE: MSG received on MHU0 Ch0 = 0xa5a5fafa
   M55-HE: MSG sent on MHU0 Ch:0 = 0x12345678
   M55-HE: MSG received on MHU0 Ch0 = 0xa5a5fafa
   ...
