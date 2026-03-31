.. _snippet-lpuart:

LPUART Shell UART
#################

Overview
********

This snippet enables the LPUART peripheral and sets it as the Zephyr shell UART (``zephyr,shell-uart``).
The overlay is common for all Alif boards.

Building and Running
********************

Build the sample as follows, changing ``alif_e8_dk/ae822fa0e5597xx0/rtss_he`` for
your board:

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/uart/echo_bot
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :gen-args: -S lpuart
   :compact:

Sample Output
=============

.. code-block:: console

    Hello! I'm your echo bot.
    Tell me something and press enter:
    # Type e.g. "Hi there!" and hit enter!
    Echo: Hi there!
