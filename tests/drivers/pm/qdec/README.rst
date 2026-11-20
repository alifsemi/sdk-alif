Alif UTIMER QDEC power-management test
======================================

Ztest for ``alif/samples/drivers/pm/qdec_alif``. UART shows
``START - test_qdec_pm_`` and ``PASS - test_qdec_pm_``.

E7/E8 jumpers: ``P0_0`` to ``P0_2``, ``P0_1`` to ``P0_3``.
E1C/B1 jumpers: ``P2_4`` to ``P6_2``, ``P2_5`` to ``P6_3``.
Disconnect the debugger before a deep-sleep run. Console is ``uart2``,
115200 8N1.

HE TCM — S2RAM
--------------

From ``zephyr/``:

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/tests/drivers/pm/qdec \
     -S qdec-pm-s2ram-tcm \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Suites ``qdec_pm_a_common`` through ``qdec_pm_e_stays_up`` run.
``qdec_pm_z_soft_off`` is skipped. The last line is
``QDEC PM SEQUENCE COMPLETED`` and then ``PROJECT EXECUTION SUCCESSFUL``.

Each QDEC sample is 25 steps and 36 degrees. After ``SUSPEND_TO_IDLE`` the
count continues (216, 252, 288, 324, 0). STANDBY keeps that count, so the
first STANDBY series is 36 only because the counter was already 0. STOP
powers UTIMER down, and the next series starts at 36. The repeat suite
then continues across three more STANDBY cycles (216, then 36, then 216).

HE or HP MRAM — SOFT_OFF
------------------------

Do not pass ``FLASH_BASE_ADDRESS=0x0``.

.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/tests/drivers/pm/qdec -S qdec-pm-mram

HP uses ``rtss_hp`` and the same snippet. HP skips ``SUSPEND_TO_IDLE``
because that snippet has no idle timer. Wakeup is LPTIMER0 on HP and the
RTC on HE.

The S2RAM suites are skipped. ``test_qdec_pm_soft_off`` prints
``SOFT_OFF has no retention - system will reset on wakeup`` and the core
boots again. ``ERROR: Resumed after PM_STATE_SOFT_OFF`` is a fail.
The next boot prints ``SOFT_OFF wakeup seen. Staying up`` and
``PROJECT EXECUTION SUCCESSFUL``. It does not enter ``SOFT_OFF`` again.

Cases that stay on the bench: open jumpers (every sample is 0 deg) and a
debugger left attached. The manual steps are in
``testlink/TESTLINK_SPECS.rst``.
