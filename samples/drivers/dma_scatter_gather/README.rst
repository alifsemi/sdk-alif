.. _dma-scatter-gather-sample:

DMA Scatter-Gather Sample
##########################

Overview
********

This sample demonstrates scatter-gather DMA support using the PL330 DMA driver.

Three memory blocks of different sizes are chained together and transferred in a
single DMA operation without CPU involvement between blocks. The sample uses
memory-to-memory transfers so no peripheral or hardware setup is required.

Building and Running
********************

The application will build only for a target that has a devicetree entry with
:dt-compatible:`arm,pl330` as a compatible.

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/dma_scatter_gather
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :gen-args: -S alif-dma

Sample Output
=============

.. code-block:: console

   [00:00:00.000,000] <inf> dma_pl330: Device dma0@49080000 initialized
   *** Booting Zephyr OS build fcc8b4926c15 ***
   [00:00:00.000,000] <inf> dma_sg_test: DMA scatter-gather test starting
   [00:00:00.000,000] <inf> dma_sg_test: Starting scatter-gather: block1=64 bytes, block2=128 bytes, block3=32 bytes
   [00:00:00.000,000] <inf> dma_sg_test: DMA scatter-gather complete on channel 0
   [00:00:00.000,000] <inf> dma_sg_test: PASS: block1 (64 bytes)
   [00:00:00.000,000] <inf> dma_sg_test: PASS: block2 (128 bytes)
   [00:00:00.000,000] <inf> dma_sg_test: PASS: block3 (32 bytes)
   [00:00:00.000,000] <inf> dma_sg_test: All blocks passed