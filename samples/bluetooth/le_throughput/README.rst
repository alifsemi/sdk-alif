.. _bluetooth-throughput-app:

BLE Throughput
##############

Overview
********

Application to test BLE throughput

Requirements
************

* Alif Balletto Development Kit

Building and Running
********************

This sample can be found under :zephyr_file:`samples/bluetooth/le_throughput` in the
sdk-alif tree.

For testing throughput you need both, central and peripheral devices.

Shell Command User Guide
************************

The throughput sample provides the ``tp`` shell command with the following subcommands.

``tp info``
===========

Show throughput device information.

Usage::

	tp info

``tp connection``
=================

Set BLE connection parameters.

Usage::

	tp connection --min <min> --max <max> --supervision <supervision>

Where:

* ``<min>``: Minimum connection interval
* ``<max>``: Maximum connection interval
* ``<supervision>``: Supervision timeout

``tp interval``
==============

Set data acket send interval in milliseconds. If not set, the default interval is 0 ms, which means that the device will send packets as fast as possible.
If set, the device will wait for the specified interval between sending packets. This can be used to simulate different traffic patterns and to test how the throughput changes with different send intervals.

Usage::

	tp interval <interval_ms>

``tp peripheral``
=================

Configure device as BLE peripheral.

Usage::

	tp peripheral

``tp central``
==============

Configure device as BLE central.

Usage::

	tp central

``tp run``
==========

Run throughput test.

Usage::

	tp run
	tp run <duration_s>

If ``<duration_s>`` is provided, the test runs for that duration in seconds. Minimum duration is 2 seconds and maximum duration is 3 hours.
If not provided, duration is set with config BLE_THROUGHPUT_DURATION and is by default 20 seconds.

``tp data-sender``
==================

Select which side sends throughput data. Default is both.

Usage::

	tp data-sender central
	tp data-sender peripheral
	tp data-sender both

Typical Flow
============

1. On one board, run ``tp peripheral``.
2. On the other board, run ``tp central``.
3. Optionally configure connection, send interval and data sender with the following commands:

	 * ``tp connection --min <min> --max <max> --supervision <supervision>``
	 * ``tp interval <interval_ms>``
	 * ``tp data-sender central|peripheral|both``

4. Start test from central side with ``tp run`` or ``tp run <duration_s>``.
5. After test is done, throughput results are printed.




