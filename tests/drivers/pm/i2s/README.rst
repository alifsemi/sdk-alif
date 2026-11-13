I2S Power Management Tests
##########################

Overview
********

This test exercises Zephyr device power management and CPU power states
with the Alif I2S driver. One image tests one routing. ``-S i2s`` selects
the I2S pair for the board. ``-S lpi2s`` selects ``&i2s4`` on HE. Add
``-S s2ram`` for standby and stop.

The device-PM cases call ``pm_device_action_run()`` directly. The MRAM
image then runs runtime idle, suspend-to-idle, and soft-off. The HE s2ram
image runs runtime idle, suspend-to-idle, standby, and stop. E7 and E8 HE
can test I2S and LPI2S. B1 and E1C HE test I2S. HP tests I2S only, and has
no s2ram image.

PM states exercised on the MRAM image:

* **Device PM**: suspend, resume, turn on, and turn off of the I2S device.
* **Runtime idle**: 18 s sleep. The CPU stays in WFI.
* **PM_STATE_SUSPEND_TO_IDLE**: short sleep. ``rtc0`` is the wakeup timer.
* **PM_STATE_SOFT_OFF**: 10 s sleep. Minimum residency is 9 s. The core
  resets on wakeup.

PM states exercised when ``-S s2ram`` is used:

* **Device PM**, runtime idle, and suspend-to-idle, as above.
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: deep sleep with TCM retention.

  * Substate 0 (STANDBY): 6 s sleep. Minimum residency is 5 s.
  * Substate 1 (STOP): 9 s sleep. Minimum residency is 8 s.

* **PM_STATE_SOFT_OFF** stays locked. This image does not enter it.

``S2RAM_SUPPORTED``
  True when the image runs on the HE core and ``SCB->VTOR`` is below
  ``0x80000000`` (TCM boot). An s2ram image that booted from MRAM fails the
  standby and stop cases.

The console is UART2. ``rtc0`` is the idle timer. HP soft-off wakes from
``timer0``.

B1 and E1C use the I2S snippet only. HP has no s2ram entry.

Requirements
************

* Alif Ensemble (E7, E8) or Balletto (B1, E1C) development board
* I2S loopback, or a microphone and speaker, on the instance selected by
  the snippet
* The s2ram image programmed to HE ITCM. SE must boot that image.
* ``rtc0`` for wakeup. The overlay selects it as the idle timer.

Building and Running
********************

HE Core — TCM boot S2RAM
========================

.. code-block:: console

   west build -p always -b <board> /test/drivers/pm/i2s \
     -S <instance> -S s2ram

HE Core — MRAM boot
===================

No ``-S s2ram``. Standby and stop are not part of this image.

.. code-block:: console

   west build -p always -b <board> /test/drivers/pm/i2s \
     -S <instance>

HP Core — MRAM boot
===================

.. code-block:: console

   west build -p always -b <board> /test/drivers/pm/i2s \
     -S i2s
