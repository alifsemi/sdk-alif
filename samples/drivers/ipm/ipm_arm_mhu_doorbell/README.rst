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
  with the freed address as an acknowledgement.
* The initiator verifies the acknowledged address matches the block it
  sent.

Because both M55 cores have D-cache enabled, the writer cleans (flushes)
its D-cache after writing shared SRAM and the reader invalidates its
D-cache before reading. The strict doorbell ping-pong guarantees only one
core touches the shared heap at a time.

**M55-HE to A32/APSS (shared-SRAM data exchange)**

The A32 (Linux) cannot operate on the Zephyr heap, so this case uses a
fixed pair of shared SRAM1 regions described by ``struct shared_msg``
(one for each direction) instead of a dynamically allocated heap block.

* The initiator (HE) writes its payload into the HE-to-A32 region and
  rings the MHU doorbell passing the physical address of that region.
* The responder (A32) maps the shared SRAM via ``/dev/mem`` (``O_SYNC``
  non-cacheable), reads and validates the message, writes a response
  into the A32-to-HE region, and rings the doorbell back with the
  physical address of its response.
* The initiator invalidates its D-cache, reads the response, and
  validates the magic value, message id and checksum.

The M55 writer cleans (flushes) its D-cache after writing shared SRAM and
invalidates its D-cache before reading; the A32 side relies on its
non-cacheable ``/dev/mem`` mapping for coherency.

**A32/APSS-initiated cases (doorbell token echo)**

The A32-initiated test cases use the MHU purely as a doorbell carrying a
small sequence token: the initiator rings the doorbell with a non-zero
token (the MHU doorbell value is a bitmask, so zero rings nothing), and
the responder echoes the same token back as an acknowledgement.

This sample includes test cases for communication between:

* M55-HE (initiator) and M55-HP (responder) -- shared-heap data exchange
* M55-HE (initiator) and A32/APSS (responder) -- shared-SRAM data exchange
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

Select the test case using CMake options:

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - ``APSS_R``
     - A32 (APSS) responder for messages from M55
   * - ``HE_HP_S``
     - M55-HE to M55-HP initiator/sender
   * - ``HP_HE_R``
     - M55-HP to M55-HE responder
   * - ``HE_A32_S``
     - M55-HE to A32 (APSS) initiator/sender

By default MHU0 is used for communication. To use MHU1 instead, add
``-DCONFIG_USE_MHU1=y`` to the build command.

Building M55-HE to M55-HP communication (requires both images):

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhu_doorbell -DHE_HP_S=ON
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/ipm/ipm_arm_mhu_doorbell -DHP_HE_R=ON

Building inter-core communication using MHU1 (requires both images):

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhu_doorbell -DHE_HP_S=ON -DCONFIG_USE_MHU1=y
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/ipm/ipm_arm_mhu_doorbell -DHP_HE_R=ON -DCONFIG_USE_MHU1=y

Building M55-HE to A32 communication (M55-HE initiates, A32 responds):

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhu_doorbell -DHE_A32_S=ON

Building M55-HE and M55-HP as responders to A32-initiated communication:

This configuration has both M55-HE and M55-HP waiting for doorbell messages
initiated by the A32 core running Linux. It requires:

* Both M55-HE and M55-HP Zephyr firmware images
* The A32 Linux test application running on the APSS core
* A different flash base address for M55-HE to avoid overwriting Linux in MRAM

By default, Linux and M55-HE share UART2. To use a separate UART for M55-HE, uncomment the
UART overlay in ``boards/alif_e8_dk_ae822fa0e5597xx0_rtss_he.overlay`` and rebuild.

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/ipm/ipm_arm_mhu_doorbell -DAPSS_R=ON
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhu_doorbell -DAPSS_R=ON -DCONFIG_FLASH_BASE_ADDRESS=0x80400000

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

While the doorbell protocol runs, each core idles into
``PM_STATE_SUSPEND_TO_IDLE`` whenever it has nothing to do. Suspend-to-idle
keeps the MHU and shared SRAM powered and wakes on the MHU doorbell IRQ
(via the IWIC), while the LPRTC idle timer wakes the initiator's timed
``k_sleep()`` calls. The deeper states (``SUSPEND_TO_RAM`` / ``SOFT_OFF``)
are locked for the duration of the protocol so the MHU/SRAM are never torn
down mid-exchange.

``PM_STATE_SUSPEND_TO_IDLE`` is the deepest low-power state that can be
woken by the MHU. The MHU doorbell is delivered as an ordinary NVIC
interrupt (numbers < 64), and only the internal wake controller (IWIC) --
which suspend-to-idle uses -- can wake the core on such an interrupt. The
deeper states (``SUSPEND_TO_RAM`` / ``SOFT_OFF``) power-gate the core and
rely on the external wake controller (EWIC), which only wakes on the
configured AON wake-events (e.g. the LPRTC/LPTIMER), never on the MHU.
Entering one of those while waiting for a doorbell would therefore make
the core deaf to the other side, so they stay locked until the protocol
finishes.

Two overlays are passed on the command line:

* ``lpm_shared_sram1.overlay`` (both cores) keeps the ``VBAT_AON`` power
  domain (so the LPRTC idle timer keeps ticking) and the shared SRAM1
  block retained across the SE run profile, and enables the
  ``suspend_idle`` state node on RTSS-HP.
* ``lpuart.overlay`` (RTSS-HE) moves the HE console to the LPUART so it is
  independent of the UART2 console used elsewhere.

Build both cores with the matching snippet and overlays:

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/ipm/ipm_arm_mhu_doorbell \
       -S pm-system-off-he -- -DHE_HP_S=ON \
       "-DEXTRA_DTC_OVERLAY_FILE=lpuart.overlay;lpm_shared_sram1.overlay"

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/ipm/ipm_arm_mhu_doorbell \
       -S pm-system-off-hp -- -DHP_HE_R=ON \
       -DEXTRA_DTC_OVERLAY_FILE=lpm_shared_sram1.overlay


Sample Output
*************

M55-HE to M55-HP shared-heap exchange:

.. code-block:: none

==========================================
M55-HE -> M55-HP : MHU0 Doorbell + Shared SRAM1 heap (initiator)
  Shared heap ctrl: 0x027dc000
  Shared heap mem : 0x027dc100 (size 0x700)
==========================================
MHU0 devices ready
Waiting for HP to be ready...
[0] HE->HP: alloc block @ 0x027dc134 msg_id=0 len=16 cksum=0x78
M55-HE: Doorbell TX Ch0 done
M55-HE: Doorbell RX Ch0, addr=0x027dc134
[0] HP->HE: ack free of block @ 0x027dc134 PASS

[1] HE->HP: alloc block @ 0x027dc134 msg_id=1 len=16 cksum=0x88
M55-HE: Doorbell TX Ch0 done
M55-HE: Doorbell RX Ch0, addr=0x027dc134
[1] HP->HE: ack free of block @ 0x027dc134 PASS

   ...
