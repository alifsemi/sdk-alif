.. _dma-scatter-gather-sample:

DMA Scatter-Gather Sample
##########################

Overview
********

This sample demonstrates both software and hardware scatter-gather DMA using the
PL330 DMA driver.

Three memory blocks of different sizes are chained together and transferred
back-to-back in two separate runs:

- **SW scatter-gather**: the driver chains blocks via ISR, starting a new
  microcode program for each block on completion of the previous one.
- **HW scatter-gather**: the driver builds a single microcode program covering
  all blocks; the hardware executes the entire chain autonomously and raises one
  interrupt at the end.

Both runs use memory-to-memory transfers so no peripheral or hardware setup is
required.

Building and Running
********************

The application requires the ``alif-dma`` snippet to select the
correct DMA controller node for the target core:

- **RTSS-HE** (Ensemble HE, E1C, Balletto): uses ``dma2``
- **RTSS-HP** (Ensemble HP): uses ``dma1``

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/dma_scatter_gather
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build flash
   :gen-args: -S alif-dma

To build for RTSS-HP:

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/dma_scatter_gather
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_hp
   :goals: build flash
   :gen-args: -S alif-dma

Sample Output
=============

.. code-block:: console

   [00:00:00.000,000] <inf> dma_pl330: Device dma2@400c0000 initialized
   *** Booting Zephyr OS build v4.1.0-... ***
   [00:00:00.000,000] <inf> dma_sg_test: DMA scatter-gather test starting
   [00:00:00.000,000] <inf> dma_sg_test: Running SW scatter-gather test
   [00:00:00.000,000] <inf> dma_sg_test: Starting scatter-gather: block1=64 bytes, block2=128 bytes, block3=32 bytes
   [00:00:00.000,000] <inf> dma_sg_test: DMA scatter-gather complete on channel 0
   [00:00:00.000,000] <inf> dma_sg_test: PASS: block1 (64 bytes)
   [00:00:00.000,000] <inf> dma_sg_test: PASS: block2 (128 bytes)
   [00:00:00.000,000] <inf> dma_sg_test: PASS: block3 (32 bytes)
   [00:00:00.000,000] <inf> dma_sg_test: All blocks passed
   [00:00:00.000,000] <inf> dma_sg_test: Running HW scatter-gather test
   [00:00:00.000,000] <inf> dma_sg_test: Starting scatter-gather: block1=64 bytes, block2=128 bytes, block3=32 bytes
   [00:00:00.000,000] <inf> dma_sg_test: DMA scatter-gather complete on channel 0
   [00:00:00.000,000] <inf> dma_sg_test: PASS: block1 (64 bytes)
   [00:00:00.000,000] <inf> dma_sg_test: PASS: block2 (128 bytes)
   [00:00:00.000,000] <inf> dma_sg_test: PASS: block3 (32 bytes)
   [00:00:00.000,000] <inf> dma_sg_test: All blocks passed