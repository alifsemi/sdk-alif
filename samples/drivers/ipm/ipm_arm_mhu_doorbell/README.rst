.. _ipm_arm_mhu_doorbell_sample:

MHU Doorbell Sample
###################

Overview
********

This sample demonstrates inter-processor communication (IPC) using
MHU doorbell interrupts on the Alif Ensemble family of devices. The MHU
channel is used as a doorbell: the 32-bit value it carries notifies the
other core and, depending on the test case, carries either a small
sequence token or the address of a shared-memory data block.

**M55-HE to M55-HP (shared-heap data exchange)**

The two M55 cores share a ``struct sys_heap`` whose control block and
managed memory both live in shared SRAM1, so both cores operate on the
same heap instance.

* The initiator (HE) dynamically allocates a message block from the
  shared heap, fills it with a payload, and rings the MHU doorbell
  passing the physical address of the allocated block.
* The responder (HP) invalidates its D-cache, reads and validates the
  block, frees it back to the shared heap, and rings the doorbell back
  with the freed address as an acknowledgment.
* The initiator verifies the acknowledged address matches the block it
  sent.

Because both M55 cores have D-cache enabled, the writer cleans (flushes)
its D-cache after writing shared SRAM and the reader invalidates its
D-cache before reading. The strict doorbell ping-pong guarantees only one
core touches the shared heap at a time.

**M55-HE to A32/APSS (shared-heap data exchange)**

This case uses the same shared ``struct sys_heap`` mechanism as the
HE-to-HP case, but with A32 (Linux) as the responder. Because A32 cannot
operate on the Zephyr heap, HE owns both the allocation and the free; A32
only reads and writes the data block whose address HE passes over the
doorbell.

* The initiator (HE) dynamically allocates a message block from the
  shared heap, fills it with a payload, and rings the MHU doorbell
  passing the physical address of the allocated block.
* The responder (A32) maps the shared SRAM via ``/dev/mem`` (``O_SYNC``
  non-cacheable), reads and validates the block, writes its response into
  the same block, and rings the doorbell back with the block address as
  an acknowledgment.
* The initiator invalidates its D-cache, validates A32's response (magic
  value, message id and checksum), and frees the block back to the shared
  heap.

The heap base is page-aligned and its whole span fits in one 4 KB page so
A32 can reach every block through a single ``/dev/mem`` mapping. The M55
writer cleans (flushes) its D-cache after writing the shared block and
invalidates its D-cache before reading; the A32 side relies on its
non-cacheable ``/dev/mem`` mapping for coherency.

**A32/APSS-initiated cases (doorbell token echo)**

The A32-initiated test cases use the MHU purely as a doorbell carrying a
small sequence token: the initiator rings the doorbell with a non-zero
token (the MHU doorbell value is a bitmask, so zero rings nothing), and
the responder echoes the same token back as an acknowledgment.

This sample includes test cases for communication between:

* M55-HE (initiator) and M55-HP (responder) -- shared-heap data exchange
* M55-HE (initiator) and A32/APSS (responder) -- shared-heap data exchange
* A32/APSS (initiator) and M55-HE/M55-HP (responder) -- doorbell token echo

Requirements
************

* Alif E8 DevKit
* Two or more cores enabled (e.g., RTSS-HE and RTSS-HP, or RTSS-HE and APSS)

Supported Targets
*****************

* alif_e8_dk/ae822fa0e5597xx0/rtss_hp
* alif_e8_dk/ae822fa0e5597xx0/rtss_he

Building
********

The code can be found in :zephyr_file:`samples/drivers/ipm/ipm_arm_mhu_doorbell`.

Select the test case with the corresponding Kconfig option:

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - ``CONFIG_APSS_R``
     - A32 (APSS) responder for messages from M55
   * - ``CONFIG_HE_HP_S``
     - M55-HE to M55-HP initiator/sender
   * - ``CONFIG_HP_HE_R``
     - M55-HP to M55-HE responder
   * - ``CONFIG_HE_A32_S``
     - M55-HE to A32 (APSS) initiator/sender

By default MHU0 is used for communication. To use MHU1 instead, add
``-DCONFIG_USE_MHU1=y`` to the build command.

Building M55-HE to M55-HP communication (requires both images):

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhu_doorbell -DCONFIG_HE_HP_S=y
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/ipm/ipm_arm_mhu_doorbell -DCONFIG_HP_HE_R=y

Built like this (without a ``pm-system-off-*`` snippet) the HE<->HP variant
runs with power management disabled: the two cores exchange messages
back-to-back and stop after 10 exchanges (``NUM_EXCHANGES``), printing a
pass/fail summary. Building both cores with the matching snippet instead
enables the low-power, reset-based model described in `Low-power Operation`_.

Building inter-core communication using MHU1 (requires both images):

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhu_doorbell -DCONFIG_HE_HP_S=y -DCONFIG_USE_MHU1=y
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/ipm/ipm_arm_mhu_doorbell -DCONFIG_HP_HE_R=y -DCONFIG_USE_MHU1=y

Building M55-HE to A32 communication (M55-HE initiates, A32 responds):

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhu_doorbell -DCONFIG_HE_A32_S=y

Building M55-HE and M55-HP as responders to A32-initiated communication:

This configuration has both M55-HE and M55-HP waiting for doorbell messages
initiated by the A32 core running Linux. It requires:

* Both M55-HE and M55-HP Zephyr firmware images
* The A32 Linux test application running on the APSS core
* A different flash base address for M55-HE to avoid overwriting Linux in MRAM

By default, Linux and M55-HE share UART2. If you want to use UART4 for M55-HE, uncomment the
UART overlay in ``boards/alif_e8_dk_ae822fa0e5597xx0_rtss_he.overlay`` and rebuild.

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/ipm/ipm_arm_mhu_doorbell -DCONFIG_APSS_R=y
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhu_doorbell -DCONFIG_APSS_R=y -DCONFIG_FLASH_BASE_ADDRESS=0x80400000

This example assumes:

* A32 Linux test application from https://github.com/alifsemi/alif_a32_linux_DD_testcases is compiled
  and copied to root filesystem.
* The Linux ``xipImage`` is already flashed to OSPI flash and the root
  filesystem is on the SD card.
* The ``linux_mpu_test_he_hp.json`` configuration file is used with the SE
  tool to program M55-HE, M55-HP, ``bl32.bin``, and ``devkit-e8.dtb`` to MRAM.


Low-power Operation
*******************

The M55-HE to M55-HP variant optionally runs both cores under power
management. PM is enabled automatically (via Kconfig) when the build
includes one of the ``pm-system-off-*`` snippets, which apply the SE
run/off power profiles and deep off-state nodes in devicetree.

Both cores spend their idle time in ``PM_STATE_SOFT_OFF`` -- the deepest
state, which power-gates the subsystem. On this MRAM-boot image (vector
table in MRAM, ``VTOR >= 0x80000000``) a SOFT_OFF wake re-enters through the
reset vector, so each wake is a full **reset/reboot** that re-runs
``main()``. A real dual-core SOFT_OFF does not retain SRAM1 content, so no
state carries across the power-off: every boot re-creates the cross-core
heap and performs exactly one HE->HP exchange. The test runs indefinitely
(one exchange per boot).

Each core has its own SOFT_OFF wake source:

* **M55-HE (initiator)** arms the always-on ``LPTIMER0`` for a 2 s wake
  before entering SOFT_OFF. Each LPTIMER0 wake resets HE, which performs one
  doorbell exchange and powers off again. While waiting for HP's ack
  during an exchange, HE idles in ``PM_STATE_SUSPEND_TO_IDLE`` (IWIC-woken by
  the MHU RX IRQ; the LPRTC idle timer backs the ``k_sleep()`` poll
  timeouts).
* **M55-HP (responder)** has no timer; it enters SOFT_OFF while waiting for a
  doorbell. The MHU RX IRQ wakes it: the M55 core's external wake controller
  (EWIC) automatically monitors the enabled NVIC interrupts (numbers < 64,
  which includes MHU0=41 / MHU1=43), so HE ringing the doorbell wakes HP --
  again via reset. On reboot HP re-arms its receivers and the latched
  doorbell is delivered, so it handles one exchange per boot.

Because the SOFT_OFF off-profile retains only MRAM (+SERAM on HP) by
default, the overlays add SRAM1 (``ALIF_SRAM1_MASK``) to the off-profile
``memory-blocks`` so the cross-core heap stays powered while one core is off
and the other is still running. Retaining SRAM1 *content* through a full
dual-core power-off would require ``ALIF_SRAM1_RET_MASK``, which keeps a rail
up and defeats the deep SOFT_OFF this sample demonstrates -- so the sample
treats every boot as a fresh exchange rather than carrying a counter.

Two overlays are passed on the command line:

* ``lpm_shared_sram1.overlay`` (both cores) keeps the ``VBAT_AON`` power
  domain (so the LPRTC idle timer keeps ticking) and the shared SRAM1
  block retained across the SE run and SOFT_OFF profiles, and enables the
  ``suspend_idle`` state node on RTSS-HP.
* ``lpuart.overlay`` (RTSS-HE) moves the HE console to the LPUART, enables
  ``LPTIMER0`` and its SOFT_OFF wake, points the off-profile wake vector
  back at the MRAM image start, and lowers the SOFT_OFF min-residency so the
  policy selects SOFT_OFF for the 2 s inter-iteration sleep.

Build both cores with the matching snippet and overlays (MRAM boot, i.e. no
``CONFIG_FLASH_*`` TCM overrides):

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhu_doorbell \
       -S pm-system-off-he -- -DCONFIG_HE_HP_S=y \
       "-DEXTRA_DTC_OVERLAY_FILE=lpuart.overlay;lpm_shared_sram1.overlay"

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/ipm/ipm_arm_mhu_doorbell \
       -S pm-system-off-hp -- -DCONFIG_HP_HE_R=y \
       -DEXTRA_DTC_OVERLAY_FILE=lpm_shared_sram1.overlay


Sample Output
*************

M55-HE to M55-HP shared-heap exchange. Built without a ``pm-system-off-*``
snippet (default), power management is disabled: the cores run 10 exchanges
back-to-back and then stop with a pass/fail summary:

.. code-block:: none

   *** Booting Zephyr OS build e9e1b4aa19a4 ***
==========================================
M55-HP <-> M55-HE : MHU Doorbell + Shared SRAM1 heap (responder)
  MHU0 heap ctrl: 0x027dc000
  MHU1 heap ctrl: 0x027dc800
  PM: disabled -- exchanges run back-to-back (build with -S pm-system-off-hp for SOFT_OFF)
==========================================
MHU devices ready
Waiting for HE doorbell...

M55-HP: Doorbell RX (MHU0) addr=0x027dc134
HE->HP: block @ 0x027dc134 msg_id=0 len=16 cksum=0x78/0x78 PASS
HP: freed block @ 0x027dc134
M55-HP: Doorbell TX (MHU0) done
Complete


Built with the ``pm-system-off-*`` snippets, both cores run under power
management. Each LPTIMER0 wake resets HE, so every cycle reboots and runs one
exchange:

.. code-block:: none

   *** Booting Zephyr OS build e9e1b4aa19a4 ***

   ==========================================
   M55-HE -> M55-HP : MHU0 Doorbell + Shared SRAM1 heap (initiator)
     Shared heap ctrl: 0x027dc000
     Shared heap mem : 0x027dc100 (size 0x700)
     PM: SOFT_OFF between exchanges, 2000 ms LPTIMER0 wake (each wake resets HE)
   ==========================================
   HE->HP: alloc block @ 0x027dc134 msg_id=0 len=16 cksum=0x78
   M55-HE: Doorbell TX (MHU0) done
   M55-HE: Doorbell RX (MHU0) addr=0x027dc134
   HP->HE: ack free of block @ 0x027dc134 PASS

   M55-HE: entering SOFT_OFF; LPTIMER0 wake in 2000 ms (core resets)

   *** Booting Zephyr OS build e9e1b4aa19a4 ***

   ==========================================
   M55-HE -> M55-HP : MHU0 Doorbell + Shared SRAM1 heap (initiator)
     Shared heap ctrl: 0x027dc000
     Shared heap mem : 0x027dc100 (size 0x700)
     PM: SOFT_OFF between exchanges, 2000 ms LPTIMER0 wake (each wake resets HE)
   ==========================================
   HE->HP: alloc block @ 0x027dc134 msg_id=0 len=16 cksum=0x78
   M55-HE: Doorbell TX (MHU0) done
   M55-HE: Doorbell RX (MHU0) addr=0x027dc134
   HP->HE: ack free of block @ 0x027dc134 PASS

   M55-HE: entering SOFT_OFF; LPTIMER0 wake in 2000 ms (core resets)
   ...
