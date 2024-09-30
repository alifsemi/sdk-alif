.. _CMP-sample:

CMP Sample
####################

Overview
********

This sample demonstrates using the alif CMP driver.

Building and Running
********************

The application will build only for a target that has a devicetree entry with
:dt compatible:`alif,cmp` as a compatible.

.. code-block:: console

    *** Booting Zephyr OS build Zephyr-Alif-SDK-v0.5.0-17-g17b360353343 ***
    [00:00:02.000,000] <inf> ALIF_CMP: start comparing
    [00:00:02.050,000] <inf> ALIF_CMP: positive input voltage is greater than negative input voltage
    [00:00:02.101,000] <inf> ALIF_CMP: negative input voltage is greater than the positive input voltage
    [00:00:02.151,000] <inf> ALIF_CMP: positive input voltage is greater than negative input voltage
    [00:00:02.201,000] <inf> ALIF_CMP: negative input voltage is greater than the positive input voltage
    [00:00:02.251,000] <inf> ALIF_CMP: positive input voltage is greater than negative input voltage
    [00:00:02.301,000] <inf> ALIF_CMP: negative input voltage is greater than the positive input voltage
    [00:00:02.351,000] <inf> ALIF_CMP: positive input voltage is greater than negative input voltage
    [00:00:02.401,000] <inf> ALIF_CMP: negative input voltage is greater than the positive input voltage
    [00:00:02.451,000] <inf> ALIF_CMP: positive input voltage is greater than negative input voltage
    [00:00:02.501,000] <inf> ALIF_CMP: negative input voltage is greater than the positive input voltage
    [00:00:02.501,000] <inf> ALIF_CMP: Comparison Completed
