UART driver tests
#################

Overview
********

This ztest suite validates the UART driver for Alif Semiconductor devices.

The tests verify:

* **Internal loopback**: self-test with ASCII, binary, numbers, and strings.
  Also stop bits, parity, data bits 5–9, 5 Mbps configure, and
  fractional baud (230400). Baud rates from 300 to 2.5 Mbps are applied
  at runtime with ``uart_configure()`` (no per-baud overlay).
* **External loopback**: physical TX-RX connection between the two chosen nodes.
* **RTS/CTS flow control**: hardware handshaking when snippet ``uart0-rts-cts``
  is applied.
* **DMA async API**: DMA TX/RX via the Zephyr async UART API (buffer
  request/release, TX abort, RX timeout).
* **TX stress**: interrupt-driven bursts of 300 KiB and 1 250 000 bytes
  on ``zephyr,devnode1``.
* **External-loopback performance**: timed 1 KiB and 25 KiB transfers
  between ``zephyr,devnode1`` and ``zephyr,devnode2`` at 115200, 921600,
  2.5 Mbps, and 5 Mbps (unsupported rates are skipped).
* **FIFO 33rd byte**: fill the 32-byte TX FIFO and confirm the next
  ``uart_fifo_fill()`` returns 0; RX flood of 33 bytes then recover
  (needs a TX-RX wire).
* **LPUART**: instance loopback when ``zephyr,devnode1`` is lpuart
  (``uart_lpuart.c``). STANDBY wake skips without a PM harness.
* **Hard plug**: unmount TX mid-transfer, detect RX stall and data
  loss, remount and recover (``uart_hotplug.c``, needs a TX-RX wire).

Board names and supported DevKit variants are listed in the Alif user guide.
Use ``-b <board>`` from that list.

Chosen nodes
************

* ``zephyr,devnode1`` is required for every suite. Apply
  ``boards/alif_uart.overlay`` (or ``boards/alif_lpuart.overlay``), or
  ``-S alif-uart-dma`` for the DMA suite.
* ``zephyr,devnode2`` is required for ``CONFIG_TEST_EXTERNAL_LB``,
  ``CONFIG_TEST_PERFORMANCE``, ``CONFIG_TEST_UART_LINE_ERRORS``, and
  ``CONFIG_TEST_UART_HOTPLUG``.

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
* ``CONFIG_TEST_STRESS=y``: builds ``src/uart_stress.c`` and registers
  ``uart_stress``.
* ``CONFIG_TEST_PERFORMANCE=y``: builds ``src/uart_perf.c``. Requires a
  physical TX-RX connection between the two chosen nodes.
* ``CONFIG_TEST_UART_LPUART=y``: builds ``src/uart_lpuart.c``. Use
  ``boards/alif_lpuart.overlay``.
* ``CONFIG_TEST_UART_FIFO=y``: builds ``src/uart_fifo_depth.c``. TX 33rd
  byte also runs with internal loopback. RX 33rd byte needs a TX-RX wire.
* ``CONFIG_TEST_UART_LINE_ERRORS=y``: builds ``src/uart_line_errors.c``.
  Needs a TX-RX wire (uart0 TX to uart1 RX).
* ``CONFIG_TEST_UART_HOTPLUG=y``: builds ``src/uart_hotplug.c``. Needs a
  TX-RX wire. Unmounts TX during a transfer, checks stall detection and
  lost bytes, then remounts.

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

   # TX stress (300 KiB and 1 250 000-byte bursts)
   west build -p always -b <board> tests/drivers/uart \
     -DDTC_OVERLAY_FILE="boards/alif_uart.overlay" \
     -DCONFIG_TEST_STRESS=y -DCONFIG_TEST_INTERNAL_LB=n

   # External-loopback performance (physical TX-RX required)
   west build -p always -b <board> tests/drivers/uart \
     -DDTC_OVERLAY_FILE="boards/alif_uart.overlay" \
     -DCONFIG_TEST_PERFORMANCE=y -DCONFIG_TEST_INTERNAL_LB=n

   # LPUART
   west build -p always -b <board> tests/drivers/uart \
     -DDTC_OVERLAY_FILE="boards/alif_lpuart.overlay" \
     -DCONFIG_TEST_UART_LPUART=y

   # FIFO 33rd-byte (RX case needs a TX-RX wire)
   west build -p always -b <board> tests/drivers/uart \
     -DDTC_OVERLAY_FILE="boards/alif_uart.overlay" \
     -DCONFIG_TEST_UART_FIFO=y -DCONFIG_TEST_INTERNAL_LB=n

   # Line break + false start (physical TX-RX required)
   west build -p always -b <board> tests/drivers/uart \
     -DDTC_OVERLAY_FILE="boards/alif_uart.overlay" \
     -DCONFIG_TEST_UART_LINE_ERRORS=y -DCONFIG_TEST_INTERNAL_LB=n

   # Hard plug (physical TX-RX required)
   west build -p always -b <board> tests/drivers/uart \
     -DDTC_OVERLAY_FILE="boards/alif_uart.overlay" \
     -DCONFIG_TEST_UART_HOTPLUG=y -DCONFIG_TEST_INTERNAL_LB=n
Twister
*******

``testcase.yaml`` defines ``drivers.uart``, ``drivers.uart.rtscts``,
``drivers.uart.dma``, ``drivers.uart.stress``, ``drivers.uart.perf``,
``drivers.uart.fifo``, ``drivers.uart.lpuart``,
``drivers.uart.hotplug``, and ``drivers.uart.line_errors``.
``drivers.uart.perf``, ``drivers.uart.hotplug``, and
``drivers.uart.line_errors`` are ``build_only`` because they need a
physical TX-RX wire; flash those images from the west commands above.
