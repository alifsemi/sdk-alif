.. _lpi2c-sample:

LPI2C Sample
####################

Overview
********

This sample demonstrates using the alif LPI2C driver.
This sample uses I2C instances one as master and the alif lpi2c as slave.
By default it uses i2c0 as master and lpi2c0 as slave.

Building and Running
********************

The application will build only for a target that has a devicetree entry with
:dt compatible:`alif,lpi2c` (lpi2c0) and `snps,designware-i2c` (i2c) as compatible strings.

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/lpi2c
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build

NOTE
============

When lpi2c0 transmits 3 bytes then, master reception has to be called 3 times
to receive the 3 bytes sent by the slave.

Sample Output
=============
.. code-block:: console
        [00:00:00.000,000] <inf> ALIF_LPI2C: Start Master trasmit and Slave receive
        [00:00:00.001,000] <inf> ALIF_LPI2C: Master transmit and slave receive successful
        [00:00:00.002,000] <inf> ALIF_LPI2C: Start Slave transmit and Master receive
        [00:00:00.006,000] <inf> ALIF_LPI2C: Slave transmit and Master receive successful
        [00:00:00.006,000] <inf> ALIF_LPI2C: Transfer completed
