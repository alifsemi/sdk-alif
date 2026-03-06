.. zephyr:code-sample:: uart_echo_dma
   :name: UART Echo DMA
   :relevant-api: uart_async

   Read data from the Shell UART using DMA RX and echo it back using DMA TX.

Overview
********

This sample demonstrates how to use the UART driver with DMA based
transfers using the asynchronous UART API.

The application receives data over Shell UART using DMA and echoes the
received line back after the user presses the Enter key.

The application separates the debug console and Shell UART (DMA):

* **Console UART** – used only for debug output via ``printk``.
* **Shell UART** – used for RX and TX using DMA.

The console UART cannot be used as a DMA UART in this sample. The UART
instance used for the console is initialized before the DMA controller
during system boot. Since the UART DMA configuration depends on the
DMA controller device, using the console UART for DMA may cause a
device initialization dependency violation during build.

In such a case the build may fail with an error similar to:

.. code-block:: console

    ERROR: Device initialization priority validation failed, the sequence
    of initialization calls does not match the devicetree dependencies.
    ERROR: /soc/uart@XXXX <uart_ns16550_init> is initialized before its
    dependency /soc/dmax@XXXX <dma_pl330_initialize>
    (PRE_KERNEL_1 < POST_KERNEL)

For this reason, a **separate Shell UART instance must be used for DMA based
communication**, while the console UART remains dedicated for debug
output.

When a user types characters on the Shell UART(DMA) and presses Enter,
the application echoes the received line back over the same UART
using DMA transmit.

Building and Running
********************

Build and flash the sample as follows, replacing the board name with
your target board.

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/uart/echo_dma
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build flash
   :gen-args: -S alif-dk
   :compact:

After flashing:

1. Connect to the **console UART** to see debug messages.
2. Open a terminal connected to the **Shell UART(DMA)**.
3. Type anything and press **Enter** on Shell UART.

The application will echo the received line back using DMA transmit.

Sample Output
=============

Console UART (debug output):

.. code-block:: console

    UART DMA Echo Test
    Enabling UART RX DMA
    RX buffer request
    Type anything on Shell UART and press ENTER!
    DMA TX done
    DMA TX done
    DMA TX done

Shell UART (DMA) terminal:

.. code-block:: console

    Hi! How are you!
    Echo:
    Hi! How are you!
