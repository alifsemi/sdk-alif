.. _snippet-alif-gpio:

Alif GPIO Testcode
=================

Overview
--------
This directory contains functional tests for the GPIO

Building and Running
-------------------

To build the example, run the following command from your Zephyr workspace:

.. code-block:: console

	rm -rf build
	west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he Zephyr_tests/tests/drivers/gpio/ -- -DSNIPPET=alif-gpio

Additional Notes
----------------

	Ensure your board is correctly connected and powered.
	Modify pin configuration in the device tree overlay if you are using different GPIO pins.
	Refer to the Zephyr GPIO API documentation for advanced usage.
