PDM Power Management Tests
##########################

Overview
********

This test exercises Zephyr device power management and suspend-to-RAM with
the Alif PDM driver. One image tests one instance. ``-S pdm`` selects
``&pdm``. ``-S lppdm`` selects ``&lppdm``. Add ``-S s2ram`` for standby and
stop.

The device-PM cases call ``pm_device_action_run()`` directly. On an s2ram
image, one case records, enters standby, and requires the programmed
channel registers plus a non-zero sample. The other records, enters
standby, records, enters stop, requires those registers to be 0, then
programs them again and requires a non-zero sample. E7 and E8 HE can test LpPDM and PDM. B1 and E1C HE test LpPDM only.
HP tests PDM only, and only the device-PM cases.

PM states exercised when ``-S s2ram`` is used:

* **Device PM**: suspend, resume, turn on, and turn off of the PDM device.
  The CPU does not sleep.
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: deep sleep with TCM retention.

  * Substate 0 (STANDBY): 6 s sleep. Minimum residency is 5 s.
  * Substate 1 (STOP): 9 s sleep. Minimum residency is 8 s.

* **PM_STATE_SOFT_OFF** stays locked. This test does not enter it.

``S2RAM_SUPPORTED``
  True when the image runs on the HE core and ``SCB->VTOR`` is below
  ``0x80000000`` (TCM boot). An s2ram image that booted from MRAM fails the
  standby and stop cases.

The HE s2ram console is UART2. ``rtc0`` is the idle timer.

B1 and E1C have no ``&pdm`` node. HP has no LpPDM and no s2ram entry.

.. note::

   On the HE s2ram image the console is UART2. UART4 is disabled by the
   s2ram overlay.

Requirements
************

* Alif Ensemble (E7, E8) or Balletto (B1, E1C) development board
* A PDM microphone on the instance selected by the snippet
* The s2ram image programmed to HE ITCM. SE must boot that image.
* ``rtc0`` for wakeup. The s2ram overlay selects it as the idle timer.

Building and Running
********************

The channel flags below select stereo channels 4 and 5 in mode 1. Pass a
different ``CONFIG_TEST_PDM_CHx`` and ``CONFIG_TEST_PDM_MODEx`` to change
the set. One group and one mode are required.

HE Core — TCM boot S2RAM
========================

.. code-block:: console

   west build -p always -b <board> test/drivers/pm/pdm \
     -S <instance> -S s2ram  \
     -DCONFIG_TEST_PDM_STEREO_CH=y -DCONFIG_TEST_PDM_CH4=y \
     -DCONFIG_TEST_PDM_CH5=y -DCONFIG_TEST_PDM_MODE1=y \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

HE Core — MRAM boot, device PM
==============================

No ``-S s2ram``. Standby and stop are not part of this image.

.. code-block:: console

   west build -p always -b <board> test/drivers/pm/pdm \
     -S <instance>  \
     -DCONFIG_TEST_PDM_STEREO_CH=y -DCONFIG_TEST_PDM_CH4=y \
     -DCONFIG_TEST_PDM_CH5=y -DCONFIG_TEST_PDM_MODE1=y

HP Core — device PM
===================

.. code-block:: console

   west build -p always -b <board> test/drivers/pm/pdm \
     -S pdm  \
     -DCONFIG_TEST_PDM_STEREO_CH=y -DCONFIG_TEST_PDM_CH4=y \
     -DCONFIG_TEST_PDM_CH5=y -DCONFIG_TEST_PDM_MODE1=y
