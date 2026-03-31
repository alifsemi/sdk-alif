.. _snippet-uart2-hfosc-clk:

UART2 HFOSC Clock Selection
############################

Overview
********

This snippet enables the HFOSC 38.4 MHz clock (``clk_38p4m``) and configures UART2 to use it as
its clock source. The overlay is common for all Alif boards.

Building and Running
********************

Build the sample as follows, changing ``alif_e8_dk/ae822fa0e5597xx0/rtss_he`` for
your board:

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/uart/echo_bot
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :gen-args: -S uart2-hfosc-clk
   :compact:

Sample Output
=============

.. code-block:: console

    Hello! I'm your echo bot.
    Tell me something and press enter:
    # Type e.g. "Hi there!" and hit enter!
    Echo: Hi there!
