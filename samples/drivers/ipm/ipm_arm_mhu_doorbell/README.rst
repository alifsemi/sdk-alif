.. _ipm_arm_mhu_doorbell_sample:

MHU Doorbell Sample
###################

Overview
********

This sample demonstrates inter-processor communication (IPC) using
MHU doorbell interrupts on the Alif Ensemble family of devices. The MHU
channel is used as a doorbell: the 32-bit value it carries notifies the
other core and carries the address of a shared-memory data block that the
other core reads, validates, and echoes or frees.

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

**M55-HE/HP to A32/APSS (shared-heap data exchange)**

This case uses the same shared ``struct sys_heap`` mechanism as the
HE-to-HP case, but with the A32/APSS core (running Zephyr) as the
responder. The M55 initiator runs on either core (RTSS-HE or RTSS-HP).
Because A32 cannot operate on the Zephyr heap, the M55 initiator owns both
the allocation and the free; A32 only reads and writes the data block
whose address the M55 passes over the doorbell.

* The initiator (M55) dynamically allocates a message block from the
  shared heap, fills it with a payload, and rings the MHU doorbell
  passing the physical address of the allocated block.
* The responder (A32) maps the shared SRAM1 heap page non-cacheable,
  reads and validates the block, writes its response into the same
  block, and rings the doorbell back with the block address as an
  acknowledgment.
* The initiator invalidates its D-cache, validates A32's response (magic
  value, message id and checksum), and frees the block back to the shared
  heap.

The heap base is page-aligned and its whole span fits in one 4 KB page so
the A32 reaches every block through a single mapping. The M55 writer
cleans (flushes) its D-cache after writing the shared block and
invalidates its D-cache before reading; the A32 maps the region
non-cacheable, so its accesses go straight to memory and need no cache
maintenance.

This sample includes test cases for communication between:

* M55-HE (initiator) and M55-HP (responder) -- shared-heap data exchange
* M55-HE/HP (initiator) and A32/APSS (responder, running Zephyr) --
  shared-heap data exchange

Requirements
************

* Alif E8 DevKit
* Two or more cores enabled (e.g., RTSS-HE and RTSS-HP, or RTSS-HE and APSS)

Supported Targets
*****************

* alif_e8_dk/ae822fa0e5597xx0/rtss_hp
* alif_e8_dk/ae822fa0e5597xx0/rtss_he
* alif_e8_dk/ae822fa0e5597xx0/apss

Building
********

The code can be found in :zephyr_file:`samples/drivers/ipm/ipm_arm_mhu_doorbell`.

Select the test case with the corresponding Kconfig option:

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - ``CONFIG_HE_HP_S``
     - M55-HE to M55-HP initiator/sender
   * - ``CONFIG_HP_HE_R``
     - M55-HP to M55-HE responder
   * - ``CONFIG_HE_A32_S``
     - M55-HE/HP to A32 (APSS) initiator/sender
   * - ``CONFIG_A32_HE_R``
     - A32 (APSS, Zephyr) responder to M55-HE/HP initiator

By default MHU0 is used for communication. To use MHU1 instead, add
``-DCONFIG_USE_MHU1=y`` to the build command.

Building M55-HE to M55-HP communication (requires both images):

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/ipm/ipm_arm_mhu_doorbell
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p always
   :gen-args: -DCONFIG_HE_HP_S=y
   :compact:

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/ipm/ipm_arm_mhu_doorbell
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p always
   :gen-args: -DCONFIG_HP_HE_R=y
   :compact:

Built like this (without a ``pm-system-off-*`` snippet) the HE<->HP variant
runs with power management disabled: the two cores exchange messages
back-to-back and stop after 10 exchanges (``NUM_EXCHANGES``), printing a
pass/fail summary. Building both cores with the matching snippet instead
enables the low-power, reset-based model described in `Low-power Operation`_.

Building inter-core communication using MHU1 (requires both images):

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/ipm/ipm_arm_mhu_doorbell
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p always
   :gen-args: -DCONFIG_HE_HP_S=y -DCONFIG_USE_MHU1=y
   :compact:

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/ipm/ipm_arm_mhu_doorbell
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p always
   :gen-args: -DCONFIG_HP_HE_R=y -DCONFIG_USE_MHU1=y
   :compact:

Building M55-HE/HP to A32 communication (M55 initiates, A32 responds):

The M55 initiator is core-agnostic; build it for the RTSS-HE or RTSS-HP
board:

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/ipm/ipm_arm_mhu_doorbell
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p always
   :gen-args: -DCONFIG_HE_A32_S=y
   :compact:

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/ipm/ipm_arm_mhu_doorbell
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p always
   :gen-args: -DCONFIG_HE_A32_S=y
   :compact:

The A32 responder for this case is a Zephyr application built for the APSS
target with ``CONFIG_A32_HE_R``. The A32 has a separate MHU to each M55
core, so the responder listens on both the HE and HP channels and replies
on whichever one rang -- the initiator can therefore run on either M55 core
with the same A32 image. It maps the shared SRAM1 heap page non-cacheable
and needs no cache maintenance:

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/ipm/ipm_arm_mhu_doorbell
   :board: alif_e8_dk/ae822fa0e5597xx0/apss
   :goals: build
   :west-args: -p always
   :gen-args: -DCONFIG_A32_HE_R=y
   :compact:


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

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/ipm/ipm_arm_mhu_doorbell
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p always
   :snippets: pm-system-off-he
   :gen-args: -DCONFIG_HE_HP_S=y "-DEXTRA_DTC_OVERLAY_FILE=lpuart.overlay;lpm_shared_sram1.overlay"
   :compact:

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/ipm/ipm_arm_mhu_doorbell
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p always
   :snippets: pm-system-off-hp
   :gen-args: -DCONFIG_HP_HE_R=y -DEXTRA_DTC_OVERLAY_FILE=lpm_shared_sram1.overlay
   :compact:


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
