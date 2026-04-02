.. zephyr:code-sample:: alif-uart
	name: Universal Asynchronous Receiver/Transmitter (UART)

	Verify UART driver functionality including loopback modes, baud rates, and RTS/CTS flow control.

#########
Overview
#########

This test suite validates the UART driver for Alif Semiconductor devices. It covers internal/external loopback modes, multiple baud rates, and RTS/CTS flow control testing.

The tests verify:
* **Internal Loopback**: Self-testing mode with various data types (ASCII, binary, numbers, strings) across multiple baud rates.
* **External Loopback**: Physical loopback testing requiring TX-RX connection.
* **Baud Rate Testing**: Verification across standard rates (300 to 2.5 Mbps).
* **RTS/CTS Flow Control**: Hardware handshaking verification.

Building and Running
********************

The application will build only when the devicetree defines the chosen nodes ``zephyr,devnode1`` and ``zephyr,devnode2``.

Manual Build Instructions
*************************

To build the UART tests for specific configurations, use the following manual build approach.

**Basic build (Default UART):**

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he tests/drivers/uart -DDTC_OVERLAY_FILE="boards/alif_uart.overlay"

**Available Configuration Flags (Kconfig):**

Specify these using ``-DCONFIG_<FLAG>=y``:

* ``CONFIG_TEST_INTERNAL_LB=y``: Enables Internal Loopback test suite.
* ``CONFIG_TEST_EXTERNAL_LB=y``: Enables External Loopback test suite (requires physical TX-RX connection).
* ``CONFIG_TEST_UART_RTSCTS=y``: Enables RTS/CTS flow control tests.

**Overlay Files:**

All helper files are located in the ``boards/`` directory.

*   **Base UART Overlays**:
    *   ``alif_uart.overlay``: Standard UART (Default, 115200 baud).
    *   ``alif_lpuart.overlay``: Run the same UART tests using the LP-UART instance.

*   **RTS/CTS Overlays (Flow Control)**:
    *   ``$PWD/../../../boards/arm/alif_e7_devkit/alif_e7_dk_rtss_he_hp_UART0_RTS_CTS.overlay``: E7 DevKit RTS/CTS overlay.
    *   ``$PWD/../../../boards/arm/alif_b1_devkit/alif_b1_dk_rtss_he_UART0_RTS_CTS.overlay``: B1 DevKit RTS/CTS overlay.

*   **Baud Rate Overlays** (Standard UART):
    *   ``alif_uart_300.overlay``: 300 baud.
    *   ``alif_uart_9600.overlay``: 9600 baud.
    *   ``alif_uart_19200.overlay``: 19200 baud.
    *   ``alif_uart_38400.overlay``: 38400 baud.
    *   ``alif_uart_57600.overlay``: 57600 baud.
    *   ``alif_uart_921600.overlay``: 921600 baud.
    *   ``alif_uart_2500000.overlay``: 2.5 Mbps.

*   **Baud Rate Overlays** (LP-UART):
    *   ``alif_lpuart_300.overlay``: 300 baud.
    *   ``alif_lpuart_9600.overlay``: 9600 baud.
    *   ``alif_lpuart_19200.overlay``: 19200 baud.
    *   ``alif_lpuart_38400.overlay``: 38400 baud.
    *   ``alif_lpuart_57600.overlay``: 57600 baud.
    *   ``alif_lpuart_921600.overlay``: 921600 baud.
    *   ``alif_lpuart_2500000.overlay``: 2.5 Mbps.

**Example Build Commands:**

.. code-block:: console

   # Internal Loopback Test
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he tests/drivers/uart -DDTC_OVERLAY_FILE="boards/alif_uart.overlay" -DCONFIG_TEST_INTERNAL_LB=y

   # External Loopback Test (requires physical TX-RX connection)
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he tests/drivers/uart -DDTC_OVERLAY_FILE="boards/alif_uart.overlay" -DCONFIG_TEST_EXTERNAL_LB=y

   # High-Speed Test (2.5 Mbps) - Use specific overlay for baud rate
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he tests/drivers/uart -DDTC_OVERLAY_FILE="boards/alif_uart_2500000.overlay"

   # RTS/CTS Flow Control Test (Example for E7/E8)
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he tests/drivers/uart -DDTC_OVERLAY_FILE="boards/alif_uart.overlay;boards/alif_UART0_RTS_CTS.overlay" -DCONFIG_TEST_UART_RTSCTS=y

   # Internal Loopback Test on LP-UART
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he tests/drivers/uart -DDTC_OVERLAY_FILE="boards/alif_lpuart.overlay" -DCONFIG_TEST_INTERNAL_LB=y

Sample Output
=============

**Internal Loopback Test:**

.. code-block:: console

	*** Booting Zephyr OS build ZAS-v4.1.0 ***
	Running TESTSUITE uart_Internalloopback
	===================================================================
	START - test_Interrupt_InternalloopbackAsciiData
	uart@49019000 UART device is ready
	Configuration passed
	Complete transmission is completed

	Internal Loop back test for Ascii with baudrate 115200 has PASSED
	 PASS - test_Interrupt_InternalloopbackAsciiData in 4.092 seconds
	===================================================================
	TESTSUITE uart_Internalloopback succeeded
