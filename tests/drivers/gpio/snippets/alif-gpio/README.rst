.. SPDX-License-Identifier: LicenseRef-AlifSemiconductor-Software-License
.. Copyright (C) 2025 Alif Semiconductor

.. _snippet-alif-gpio:

Alif GPIO Support
=================

Overview
--------

This snippet demonstrates how to enable and use GPIO on the Alif B1 Development Kit.
It provides basic functionality for controlling GPIO pins using the Zephyr RTOS GPIO API.

Building and Running
-------------------

To build the example, run the following command from your Zephyr workspace:

.. code-block:: console

	rm -rf build
	west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he Zephyr_tests/tests/drivers/gpio/ -- -DSNIPPET=alif-gpio

Once the build completes, you can flash the application to the board using:

.. code-block:: console

	west flash

Verifying
---------

You can verify GPIO functionality by connecting LEDs, switches, or other GPIO peripherals to the board pins
as defined in the application. The example snippet toggles GPIO pins to demonstrate basic input/output control.

Additional Notes
----------------

	Ensure your board is correctly connected and powered.
	Modify pin configuration in the device tree overlay if you are using different GPIO pins.
	Refer to the Zephyr GPIO API documentation for advanced usage.

