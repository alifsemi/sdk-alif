.. _dma-pl330-user-mcode-sample:

PL330 DMA User Microcode Sample
################################

Overview
********

This sample demonstrates how to build and execute a custom ARM PL330 DMA
microcode program using the Zephyr PL330 driver external microcode API.

It shows:

- Constructing a PL330 microcode program using the ``dma_pl330_gen_*()``
  helpers from ``<zephyr/drivers/dma/dma_pl330_opcode.h>``.
- Executing the microcode via ``dma_pl330_start_with_mcode()``.
- Performing a memory-to-memory copy of 1000 bytes using 1-byte beats
  and a burst length of 16, with a residual tail burst for the remaining
  bytes.

The channel is configured via the standard ``dma_config()`` API to
register the completion callback.

Building and Running
********************

The application requires the ``alif-dma-mcode`` snippet to select the
correct DMA controller node for the target core:

- **RTSS-HE** (Ensemble HE, E1C, Balletto): uses ``dma2``
- **RTSS-HP** (Ensemble HP): uses ``dma1``

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/dma_user_mcode
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build flash
   :gen-args: -S alif-dma-mcode

To build for RTSS-HP:

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/dma_user_mcode
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_hp
   :goals: build flash
   :gen-args: -S alif-dma-mcode

Sample Output
=============

.. code-block:: console

   [00:00:00.000,000] <inf> dma_pl330: Device dma2@400c0000 initialized
   ***** delaying boot 5000ms (per build configuration) *****
   *** Booting Zephyr OS build v4.1.0-517-g6f4ae8e1ecc7 (delayed boot 5000ms) ***
   Microcode size: 36 bytes
   DMA memcpy PASS
