UART driver tests
#################

Overview
********

This ztest suite validates the UART driver for Alif Semiconductor devices.

The tests verify:

* **Internal loopback**: self-test with ASCII, binary, numbers, and strings.
  Baud rates from 300 to 2.5 Mbps are applied at runtime with
  ``uart_configure()`` (no per-baud overlay).
* **External loopback**: physical TX-RX connection between the two chosen nodes.
* **RTS/CTS flow control**: hardware handshaking when snippet ``uart0-rts-cts``
  is applied.
* **DMA async API**: DMA TX/RX via the Zephyr async UART API (buffer
  request/release, TX abort, RX timeout).

Board names and supported DevKit variants are listed in the Alif user guide.
Use ``-b <board>`` from that list.

Chosen nodes
************

* ``zephyr,devnode1`` is required for every suite. Apply
  ``boards/alif_uart.overlay`` (or ``boards/alif_lpuart.overlay``), or
  ``-S alif-uart-dma`` for the DMA suite.
* ``zephyr,devnode2`` is required only for ``CONFIG_TEST_EXTERNAL_LB``.

Configuration
*************

All suite Kconfig flags default to ``n``. ``prj.conf`` enables
``CONFIG_TEST_INTERNAL_LB`` for the default (non-DMA) build.

* ``CONFIG_TEST_INTERNAL_LB=y``: builds ``src/uart_internal_loopback.c``
  and registers ``uart_internal_loopback``.
* ``CONFIG_TEST_EXTERNAL_LB=y``: builds ``src/uart_external_loopback.c``.
* ``CONFIG_TEST_UART_RTSCTS=y``: builds the RTS/CTS case in
  ``src/uart_internal_loopback.c`` and registers ``uart_RTSCTS_suite``.
  Also pass ``-S uart0-rts-cts``.
* ``CONFIG_TEST_UART_DMA=y``: builds ``src/uart_dma_tests.c``. Pass
  ``-DEXTRA_CONF_FILE=dma.conf`` (async API + DMA +
  ``CONFIG_TEST_INTERNAL_LB=n``) and ``-S alif-uart-dma``. Do not enable
  this flag alone.

Overlays and snippets
*********************

Files in ``boards/`` (non-DMA suites):

* ``alif_uart.overlay``: ``zephyr,devnode1`` = uart0, ``zephyr,devnode2`` = uart1.
* ``alif_lpuart.overlay``: same tests on the LP-UART instance (devnode1).

Snippets:

* ``-S uart0-rts-cts``: sdk-alif snippet that enables UART0 RTS/CTS pins
  (per-board overlay). Combine with ``boards/alif_uart.overlay``.
* ``-S alif-uart-dma``: test-local snippet (same pattern as
  ``samples/drivers/uart/echo_dma``). Selects the DMA overlay by board:
  E7/E8 use uart0 request IDs 8/16 on dma0; B1/E1C use uart2 request IDs
  10/18 on dma2 and move the console to lpuart.

Build examples
**************

.. code-block:: console

   # Internal loopback (default prj.conf)
   west build -p always -b <board> tests/drivers/uart \
     -DDTC_OVERLAY_FILE="boards/alif_uart.overlay"

   # External loopback (physical TX-RX required)
   west build -p always -b <board> tests/drivers/uart \
     -DDTC_OVERLAY_FILE="boards/alif_uart.overlay" \
     -DCONFIG_TEST_EXTERNAL_LB=y

   # RTS/CTS flow control
   west build -p always -b <board> tests/drivers/uart \
     -S uart0-rts-cts \
     -DDTC_OVERLAY_FILE="boards/alif_uart.overlay" \
     -DCONFIG_TEST_UART_RTSCTS=y

   # Internal loopback on LP-UART
   west build -p always -b <board> tests/drivers/uart \
     -DDTC_OVERLAY_FILE="boards/alif_lpuart.overlay"

   # DMA async API (overlay chosen per board by the snippet)
   west build -p always -b <board> tests/drivers/uart \
     -S alif-uart-dma -DEXTRA_CONF_FILE=dma.conf

Twister
*******

``testcase.yaml`` defines ``drivers.uart``, ``drivers.uart.rtscts``, and
``drivers.uart.dma``.

Sample output
*************

.. code-block:: console

   *** Booting Zephyr OS build ZAS-v4.1.0 ***
   Running TESTSUITE uart_internal_loopback
   ===================================================================
   START - test_interrupt_internal_loopback_ascii_data
   uart@49019000 UART device is ready
   Configuration passed
   Complete transmission is completed

   Internal Loop back test for Ascii with baudrate 115200 has PASSED
    PASS - test_interrupt_internal_loopback_ascii_data in 4.092 seconds
   ===================================================================
   TESTSUITE uart_internal_loopback succeeded
