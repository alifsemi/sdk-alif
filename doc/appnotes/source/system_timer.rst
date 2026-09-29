.. _system-timers:

==============
System Timers
==============

Introduction
============

This document explains how to use the ARM memory-mapped Generic Timer
blocks (system timers) as Zephyr counters on Alif Semiconductor
Ensemble and Balletto devices. Each timer has a shared control block
and one or more CNTBaseN frames. The Zephyr driver exposes one frame as
a counter: applications read the physical count (CNTPCT) and use the
physical compare interrupt.

The same Zephyr alarm sample used for LPRTC, LPTIMER, and UTIMER
(``samples/drivers/counter/alarm``) is the demo. SoC Devicetree describes
every frame as disabled. An overlay enables the single frame.

Overview
--------

Two system-timer blocks sit in the PD2 (SSE-700 always-on) domain:

- **REFCLK_TMR** — increments from SYST_REFCLK (``ALIF_SYSREF_CLK``).
  The rate follows the run profile / P-state (typically tens of MHz).
- **S32KCLK_TMR** — increments from S32K_CLK (``ALIF_S32K_CLK``),
  sourced from either ``lfxo`` or ``lfrc`` (typically 32768 Hz).

Each block provides:

- A 64-bit up-counter (CNTPCT). ``counter_get_value()`` returns the low
  32 bits; ``counter_get_value_64()`` returns the full count.
- One physical compare channel per frame (CNTP_CVAL / CNTP_CTL).
- ``set_top_value()`` is not supported. The software top is
  ``UINT32_MAX``.

``CONFIG_COUNTER_ARM_TIMER_MEM`` defaults to ``y`` when a frame node is
enabled. No extra ``prj.conf`` overlay is required.

REFCLK (SYST_REFCLK)
--------------------

+------------------+------------------+------------------------+
| Role             | Address          | Notes                  |
+==================+==================+========================+
| Control          | ``0x1A200000``   | CNTCR, CNTFID0         |
+------------------+------------------+------------------------+
| Read             | ``0x1A210000``   | Reserved in the driver |
+------------------+------------------+------------------------+
| CTL              | ``0x1A220000``   | CNTACR, CNTTIDR        |
+------------------+------------------+------------------------+
| CNTBase0-3       | ``0x1A230000``-  | IRQ 67-70              |
|                  | ``0x1A260000``   |                        |
+------------------+------------------+------------------------+

Clock: ``clocks = <&clockctrl ALIF_SYSREF_CLK>``.

Four frames are described. The application chooses which frame to
enable. If APSS is enabled, it uses CNTBase0
(``arm,armv8-timer`` at ``0x1A230000``); do not also enable
``refclk_cntbase0`` as a Zephyr counter in that case.

+--------+---------------------+
| Frame  | Node                |
+========+=====================+
| 0      | ``refclk_cntbase0`` |
+--------+---------------------+
| 1      | ``refclk_cntbase1`` |
+--------+---------------------+
| 2      | ``refclk_cntbase2`` |
+--------+---------------------+
| 3      | ``refclk_cntbase3`` |
+--------+---------------------+

S32K (S32K_CLK)
---------------

+------------------+------------------+------------------------+
| Role             | Address          | Notes                  |
+==================+==================+========================+
| Control          | ``0x1A400000``   | CNTCR, CNTFID0         |
+------------------+------------------+------------------------+
| Read             | ``0x1A410000``   | Reserved in the driver |
+------------------+------------------+------------------------+
| CTL              | ``0x1A420000``   | CNTACR, CNTTIDR        |
+------------------+------------------+------------------------+
| CNTBase0–1       | ``0x1A430000``,  | IRQ 71–72              |
|                  | ``0x1A440000``   |                        |
+------------------+------------------+------------------------+

Clock: ``clocks = <&clockctrl ALIF_S32K_CLK>``. The source is ``lfxo``
or ``lfrc`` (typically 32768 Hz). Only two frames exist. The
application chooses which frame to enable.

+--------+-------------------+
| Frame  | Node              |
+========+===================+
| 0      | ``s32k_cntbase0`` |
+--------+-------------------+
| 1      | ``s32k_cntbase1`` |
+--------+-------------------+

Shared start and stop
---------------------

``counter_start()`` sets ``CNTCR.EN`` on the parent control block.
``counter_stop()`` clears it and disarms that frame’s compare.

``CNTCR.EN`` is **shared** by every frame of that timer and by both
RTSS cores. Stopping any frame of REFCLK (or of S32K) stops the count
for every frame of that timer, including the other core.

The driver does not refcount or refuse ``stop``. The application must
not call ``counter_stop()`` on a timer the other core still needs.

Overlays
--------

SoC Devicetree (``system_timer.dtsi``) sets every frame to
``status = "disabled"``. The parent nodes stay okay so an overlay can
enable a child.

Examples:

.. code-block:: dts

   /* S32K frame 0 (boards/alif_system_timer.overlay) */
   &s32k_cntbase0 {
           status = "okay";
   };

   /* S32K frame 1 */
   &s32k_cntbase1 {
           status = "okay";
   };

   /* REFCLK frame 2 */
   &refclk_cntbase2 {
           status = "okay";
   };

   /* REFCLK frame 1 */
   &refclk_cntbase1 {
           status = "okay";
   };

Build a System Timer Application with Zephyr
============================================

The S32K HE overlay is
``samples/drivers/counter/alarm/boards/alif_system_timer.overlay``.

Follow these steps using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to
   the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in
   the build command accordingly. For more information, refer to the
   `ZAS User Guide`_, under the section Setting Up and Building Zephyr
   Applications.

2. Build on the M55 HE/HP core (S32K frame 0):

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/counter/alarm/ \
     -- \
     -DDTC_OVERLAY_FILE=boards/alif_system_timer.overlay


Once the build command completes successfully, executable images will be
generated and placed in the `build/zephyr` directory. Both `.bin` and
`.elf` files will be available.

To run the same sample on REFCLK, enable any unused REFCLK frame
(not ``refclk_cntbase0`` if APSS is enabled). REFCLK alarms use a
much higher tick rate than S32K; the sample still programs a
two-second delay.

Executing Binary on the DevKit
==============================

To execute binaries on the DevKit follow the command

.. code-block:: console

   west flash

Loading Binaries with SE Tools
==============================

For detailed instructions on loading executables using SE Tools, refer to
the *Getting Started with ZAS for Ensemble* documentation.

Sample Output
=============

The sample alarm application runs until stopped. On S32K the first alarm
is about two seconds:

.. code-block:: text

   *** Booting Zephyr OS build v4.1.0-884-g02f39c38fc98 ***
   Counter alarm sample

   Set alarm in 2 sec (65536 ticks)
   !!! Alarm !!!
   Now: 2
   Set alarm in 4 sec (131072 ticks)
   !!! Alarm !!!
   Now: 6
   Set alarm in 8 sec (262144 ticks)
   !!! Alarm !!!
   Now: 14
   Set alarm in 16 sec (524288 ticks)
   !!! Alarm !!!
   Now: 30
   Set alarm in 32 sec (1048576 ticks)
