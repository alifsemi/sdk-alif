I3C Controller and Target Tests
###############################

Overview
********

Two images share ``common/``. The target board is an always-running I3C
service. The controller board owns the Ztest suite, selects transfer modes,
checks returned data, and measures throughput. A second board is optional:
tests that need the target skip when it is not connected.

``CONFIG_I3C_TEST_CONTROLLER`` builds the Ztest controller.
``CONFIG_I3C_TEST_TARGET`` builds the persistent service. Each image has its
own ``app.overlay`` and ``boards/`` files. The controller overlay describes the
remote target. The target overlay sets ``target-mode``. Application code
selects the bus with ``zephyr,i3c``.

Test groups:

- **Local** -- readiness, null buffer, zero length, invalid address, bad IBI
  length, frame checksum, and unsupported controller-role request
- **Discovery** -- DAA, PID, BCR, and DCR
- **CCC** -- GETPID, GETBCR, GETDCR, GETSTATUS, GETMRL, SET/GETMWL, ENEC/DISEC
- **Transfer** -- private write and echo, fill/read, checksum, and sequence
- **Performance** -- negotiated bulk chunks, byte counters, SDR speed sweep,
  elapsed time, and effective payload rate
- **HDR** -- HDR-DDR echo, then one command that returns the target to SDR.
  The test skips when the controller does not report HDR-DDR.
- **IBI** -- 1 to 5 byte payloads, disable/re-enable, and IBI after RSTDAA
- **Handoff** -- capability-gated reject, accept, controller-role request, and
  handback
- **Stress** -- repeated echo, IBI, and CCC

PID Value example
*****************

The target PID is a 48-bit value. It must be unique on the bus and must
match ``I3C_TEST_MFG_ID`` and ``I3C_TEST_PART_ID`` in ``common/i3c_protocol.h``.
Do not reuse a sensor manufacturer ID.

Construct it as follows:

1. Manufacturer ID is 15 bits, PID ``[47:33]``. This test uses ``0x0001``.
2. Bit ``[32]`` is the type bit. Keep it ``0`` for a fixed part number. The
   driver rejects a random PID.
3. Part and instance ID is PID ``[31:0]``. This test uses ``0x00000001``.
4. ``pid_hi`` is the manufacturer ID shifted left by 1, with the type bit in
   bit 0. That is PID ``[47:32]``: ``0x0001 << 1 = 0x0002``.
5. ``pid_lo`` is the part ID unchanged: ``0x00000001``.
6. The full PID is ``(pid_hi << 32) | pid_lo``, which is ``0x000200000001``.
7. The devicetree ``reg`` property is ``<static_addr pid_hi pid_lo>``.
   The static address is a 7-bit value in ``0x08``–``0x7D``, or ``0`` when
   the device has no static address. ``assigned-address`` is the preferred
   dynamic address in the same range. Both must be unique on the bus.
8. The node name is the static-address byte followed by the PID written as
   16 hex digits: ``0x50`` and ``0000000200000001`` become
   ``@500000000200000001``.

.. code-block:: console

   /*
    * reg = <static_addr pid_hi pid_lo>;
    *
    * 48-bit PID:
    *   [47:33] manufacturer ID (15 bits)
    *   [32]    0 = fixed part, 1 = random (keep 0)
    *   [31:0]  part and instance ID
    *
    * pid_hi = (manufacturer_id << 1) | type_bit     PID[47:32]
    * pid_lo = part_id                               PID[31:0]
    *
    * manufacturer 0x0001, part 0x00000001:
    *   pid_hi = 0x0002
    *   pid_lo = 0x00000001
    *   PID    = 0x000200000001
    *
    * Node name = static address byte + PID as 16 hex digits:
    *   0x50 + 0000000200000001 -> @500000000200000001
    *
    * assigned-address = preferred dynamic address, 0x08..0x7D,
    * unique on this bus. Must match the target firmware.
    */
   i3c_test_target: i3c_test_target@500000000200000001 {
      reg = <0x50 0x0002 0x00000001>;
      assigned-address = <0x09>;
   };

Building and Running
********************

Commands are run from the SDK workspace.

Twister builds
==============

The target scenarios are build-only because the service intentionally never
exits. Flash the target first, then run or flash the controller test image.

.. code-block:: console

   west twister -T alif/tests/drivers/i3c_controller_target/controller \
                --platform <board>

.. code-block:: console

   west twister -T alif/tests/drivers/i3c_controller_target/target \
                --platform <board>

Test scenarios (``-s`` filter):

- ``drivers.i3c.controller``
- ``drivers.i3c.controller.lpi3c``
- ``drivers.i3c.target``
- ``drivers.i3c.target.lpi3c``

Manual west build
=================

Controller on I3C0
------------------

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
              alif/tests/drivers/i3c_controller_target/controller \
              -- -DDTC_OVERLAY_FILE=boards/alif_e7_i3c.overlay

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
              alif/tests/drivers/i3c_controller_target/controller \
              -- -DDTC_OVERLAY_FILE=boards/alif_e8_i3c.overlay

Target on I3C0
--------------

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
              alif/tests/drivers/i3c_controller_target/target \
              -- -DDTC_OVERLAY_FILE=boards/alif_e7_i3c.overlay

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
              alif/tests/drivers/i3c_controller_target/target \
              -- -DDTC_OVERLAY_FILE=boards/alif_e8_i3c.overlay

LPI3C
-----

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
              alif/tests/drivers/i3c_controller_target/controller \
              -- -DDTC_OVERLAY_FILE=boards/alif_e8_lpi3c.overlay

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
              alif/tests/drivers/i3c_controller_target/target \
              -- -DDTC_OVERLAY_FILE=boards/alif_e8_lpi3c.overlay

Kconfig Options
***************

Role (one image selects one):

- ``CONFIG_I3C_TEST_CONTROLLER``    Controller image
- ``CONFIG_I3C_TEST_TARGET``        Target image

Test tuning:

- ``CONFIG_I3C_TEST_STRESS_LOOPS``      Stress iterations (default 100)
- ``CONFIG_I3C_TEST_BULK_BYTES``        Total payload bytes (default 4096)
- ``CONFIG_I3C_TEST_BULK_CHUNK``        Payload per transaction (default 32)
- ``CONFIG_I3C_TEST_ENABLE_HDR``        Enable the controller HDR-DDR test
- ``CONFIG_I3C_TEST_HANDOFF``           Enable capability-gated handoff tests
- ``CONFIG_I3C_TEST_DISCOVER_WAIT_MS``  Controller wait for target hot-join (default 20 s)

Devicetree Overlays
*******************

Each image has the same overlay names. Use the file from the image you build.

- ``app.overlay``                    Default ``i3c0`` overlay
- ``boards/alif_e7_dk.overlay``      E7 ``i3c0``
- ``boards/alif_b1_dk.overlay``      B1 ``i3c0``
- ``boards/alif_e8_i3c_dk.overlay``  E8 ``i3c0``
- ``boards/alif_e8_lpi3c_dk.overlay`` E8 ``lpi3c0``

Hardware Requirements
*********************

Supported boards
================

- ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
- ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
- ``alif_e1c_dk/ae1c1f4051920hh/rtss_he``
- ``alif_b1_dk/ab1c1f4m51820hh0/rtss_he``

Peer
====

Flash and start the target once. It configures PID/BCR/DCR/MRL/MWL, registers
ISR-safe callbacks, and starts its service thread. With no dynamic address it
raises hot-join. After an address is assigned it sends one ready IBI.
``DONE`` clears the session but does not stop the service.

Start the controller at any time. If the test PID already has a dynamic
address in the devicetree device list, the controller waits for that ready
IBI. If that PID is not an addressed devicetree entry, the controller
broadcasts ENEC for every event, waits for the target hot-join, assigns an
address, and verifies GETPID before running peer tests. That ENEC and
hot-join wait is only in this controller test, and only for the test-target
PID.

Other children
==============

The controller bus also has a BMI323 child, static address ``0x69``, PID
``0x03b810431000``, preferred dynamic address ``0x0a``. Bus initialization
assigns every devicetree child. ``test_bmi323_child`` reads that sensor's
chip ID. The framed protocol tests still use only the test target. The
BMI323 test skips when that sensor was not assigned an address. On B1 the
interrupt pin is P6_6; on E7 and E8 DK it is P8_4.
Discovery, CCC, transfer, performance, HDR, IBI, and handoff tests skip when
the required peer or hardware capability is absent.

The reported throughput is application payload divided by elapsed transfer
time. DAA, capability discovery, logging, and suite setup are outside the
timed interval. Increase ``CONFIG_I3C_TEST_BULK_CHUNK`` only after validating
the FIFO depth of the selected I3C instance.

Identity
========

Both images use PID ``0x000200000001``, static address ``0x50``, preferred
dynamic address ``0x09``, and DCR ``0x00``. The controller devicetree child is
``alif,i3c-test-target``. The target requests the controller-capable BCR role;
handoff tests still skip if the DesignWare capability register or BCR readback
does not confirm that role.
