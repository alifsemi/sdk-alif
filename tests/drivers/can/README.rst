CAN Normal Mode Test
####################

Overview
********

This test validates the Alif CAN driver in normal mode using an
external CAN analyzer (e.g. Vector CANalyzer, PCAN, etc.).

The suite is split into two independent ztest suites:

* ``can_normal_tx`` — self-contained TX and robustness tests.
* ``can_normal_rx`` — tests that require the analyzer to inject specific
  frames. These skip gracefully if no stimulus is present.

Bus Behavior
************

**TX tests** send frames onto the CAN bus. The analyzer only needs to
**ACK** the frames at the physical layer. It does **not** need to store
or display them for the tests to pass. Any node on the bus that ACKs
is sufficient.

**RX tests** expect the analyzer to **inject** one specific frame per test.
Each test installs its own RX filter, waits for a single matching frame,
then removes the filter. A frame sent before that test is waiting is not
kept for a later test.

Prerequisites
*************

* Alif board with CAN0 connected to a physical CAN transceiver.
* CAN analyzer on the same bus, configured for **500 kbps nominal**
  bitrate and **2 Mbps data** bitrate (for FD tests).
* The analyzer must **ACK** transmitted frames or the TX tests will fail.

Building
********

.. code-block:: console

   west build -b <board> tests/drivers/can
   west flash

Test Suites
***********

can_normal_tx (self-contained)
==============================

These tests only require a healthy bus with an ACKing node.

* ``test_capabilities`` — verify NORMAL mode is supported.
* ``test_send_std_classic`` — TX standard 11-bit ID, 8 data bytes.
* ``test_send_ext_classic`` — TX extended 29-bit ID, 4 data bytes.
* ``test_send_rtr_std`` — TX standard RTR frame.
* ``test_send_rtr_ext`` — TX extended RTR frame.
* ``test_send_zero_dlc`` — TX classic frame with DLC = 0.
* ``test_send_fd`` — TX CAN-FD with BRS, 64-byte payload.
* ``test_send_fd_no_brs`` — TX CAN-FD without BRS, 32-byte payload.
* ``test_back_to_back_tx`` — rapid-fire 10 classic frames.
* ``test_bus_recovery_api`` — manual recovery mode transitions.
* ``test_error_state_active`` — verify ERROR_ACTIVE, zero counters.
* ``test_stats_accessors`` — read all driver statistic counters.

can_normal_rx (requires external stimulus)
==========================================

Each test logs ``=== Waiting for ... ===``, installs one RX filter, and
waits up to ``CONFIG_CAN_TEST_RX_TIMEOUT_MS`` (default 60 s). Send that
test's frame after the prompt and before the timeout. If no frame arrives,
the test skips. Sending every frame at suite start does not work: nothing
is buffered between tests.

CANalyzer Injection Frames
--------------------------

Use the following frame definitions to configure your analyzer.

Classic standard frame::

   ID:      0x123
   Type:    Classic CAN
   DLC:     8
   Data:    AA BB CC DD EE FF 11 22

Classic extended frame::

   ID:      0x18ABCDEF
   Type:    Classic CAN
   DLC:     4
   Data:    DE AD BE EF

RTR frame::

   ID:      0x123
   Type:    Classic CAN
   DLC:     8
   RTR:     Yes
   Data:    (none)

CAN-FD frame with BRS::

   ID:      0x18ABCDEF
   Type:    CAN FD
   BRS:     Yes
   DLC:     15 (64 bytes)
   Data:    00 01 02 03 04 05 06 07
            08 09 0A 0B 0C 0D 0E 0F
            10 11 12 13 14 15 16 17
            18 19 1A 1B 1C 1D 1E 1F
            20 21 22 23 24 25 26 27
            28 29 2A 2B 2C 2D 2E 2F
            30 31 32 33 34 35 36 37
            38 39 3A 3B 3C 3D 3E 3F

Test-to-frame mapping::

   test_receive_std_from_CANalyzer  -> standard frame (0x123, DLC=8)
   test_receive_ext_from_CANalyzer  -> extended frame (0x18ABCDEF, DLC=4)
   test_receive_rtr_from_CANalyzer  -> RTR frame (0x123, DLC=8)
   test_receive_fd_from_CANalyzer   -> FD+BRS frame (0x18ABCDEF, DLC=15)

**Note:** RX timeout does **not** cause bus errors. Only TX without an
ACK will increment error counters.

Test Variants
*************

* ``drivers.can.alif.normal`` — classic CAN + TX suite only
* ``drivers.can.alif.normal.fd`` — includes CAN-FD tests

Twister example::

   west twister -T tests/drivers/can -p alif_e7_dk/ae722f80f55d5xx/rtss_he
