.. _snippet-uart0-rts-cts:

UART0 Hardware Flow Control (RTS/CTS)
######################################

Overview
********

This snippet enables hardware flow control (RTS-CTS) for UART0 and sets it as the Zephyr shell UART
(``zephyr,shell-uart``). It configures the appropriate pinmux for RTS and CTS signals based on the
target board:

- **B1 / E1C boards**: Uses pins P8_0/P8_1 (RX/TX) and P6_6/P6_7 (CTS/RTS)
- **Ensemble E3-E8 boards**: Uses pins P0_0/P0_1 (RX/TX) and P0_2/P0_3 (CTS/RTS)

Building and Running
********************

Build the sample as follows, changing ``alif_e8_dk/ae822fa0e5597xx0/rtss_he`` for
your board:

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/uart/echo_bot
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :gen-args: -S uart0-rts-cts
   :compact:

Sample Output
=============

.. code-block:: console

    Hello! I'm your echo bot.
    Tell me something and press enter:
    # Type e.g. "Hi there!" and hit enter!
    Echo: Hi there!
