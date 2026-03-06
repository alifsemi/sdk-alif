Alif LPTIMER Test Suite
=======================

Comprehensive test suite for Alif LPTIMER peripherals covering various clock sources, modes, and configurations.

Configuration Quick Reference
-----------------------------

+---------------------+-----------------------------+-----------------------------+
| Configuration      | Config File                 | Overlay Pattern             |
+=====================+=============================+=============================+
| Default            | -                           | ``alif_*_lptimer*.overlay`` |
+---------------------+-----------------------------+-----------------------------+
| 128kHz Clock       | ``alif_lptimer_128k.conf``  | ``alif_*_lptimer*_128.overlay``|
+---------------------+-----------------------------+-----------------------------+
| External Clock     | ``alif_lptimer*_ext.conf``  | ``alif_*_lptimer*_ext.overlay``|
+---------------------+-----------------------------+-----------------------------+
| Cascade Mode       | ``alif_lptimer_cascade*.conf``| ``alif_lptimer*_cascade.overlay``|
+---------------------+-----------------------------+-----------------------------+
| Output Toggle      | ``alif_lptimer*_opt.conf``  | ``alif_*_lptimer.overlay``   |
+---------------------+-----------------------------+-----------------------------+
| Wake-up            | ``alif_lptimer_wakeup.conf``| ``alif_*_lptimer.overlay``   |
+---------------------+-----------------------------+-----------------------------+

Build Commands
--------------

**Base Configuration (Default):**

.. code-block:: bash

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/tests/drivers/lptimer

**128kHz Clock Source:**

.. code-block:: bash

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/tests/drivers/lptimer \
     -DDTC_OVERLAY_FILE=$PWD/../alif/tests/drivers/lptimer/boards/alif_ensemble_lptimer0_128.overlay \
     -DOVERLAY_CONFIG=$PWD/../alif/tests/drivers/lptimer/boards/alif_lptimer_128k.conf

**External Clock Source:**

.. code-block:: bash

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/tests/drivers/lptimer \
     -DDTC_OVERLAY_FILE=$PWD/../alif/tests/drivers/lptimer/boards/alif_ensemble_lptimer0_ext.overlay \
     -DOVERLAY_CONFIG=$PWD/../alif/tests/drivers/lptimer/boards/alif_lptimer_ext.conf

**Cascade Mode (64-bit Timer):**

.. code-block:: bash

   # LPTIMER1+LPTIMER0 cascade
   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/tests/drivers/lptimer \
     -DDTC_OVERLAY_FILE=$PWD/../alif/tests/drivers/lptimer/boards/alif_lptimer1_cascade.overlay \
     -DOVERLAY_CONFIG=$PWD/../alif/tests/drivers/lptimer/boards/alif_lptimer_cascade.conf

   # LPTIMER1+LPTIMER0 cascade (ALIF_LPTIMER1_CASCADE_CLK)
   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/tests/drivers/lptimer \
     -DDTC_OVERLAY_FILE=$PWD/../alif/tests/drivers/lptimer/boards/alif_lptimer1_cascade.overlay \
     -DOVERLAY_CONFIG=$PWD/../alif/tests/drivers/lptimer/boards/alif_lptimer_cascade1.conf

   # LPTIMER3+LPTIMER2 cascade (ALIF_LPTIMER3_CASCADE_CLK)
   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/tests/drivers/lptimer \
     -DDTC_OVERLAY_FILE=$PWD/../alif/tests/drivers/lptimer/boards/alif_lptimer3_cascade.overlay \
     -DOVERLAY_CONFIG=$PWD/../alif/tests/drivers/lptimer/boards/alif_lptimer_cascade3.conf

**Output Toggle Mode:**

.. code-block:: bash

   west build -b alif_e7_dk/ae722f80f55d5xx0/rtss_he ../alif/tests/drivers/lptimer \
     -DDTC_OVERLAY_FILE=$PWD/../alif/tests/drivers/lptimer/boards/alif_ensemble_lptimer0.overlay \
     -DOVERLAY_CONFIG=$PWD/../alif/tests/drivers/lptimer/boards/alif_lptimer_opt.conf

**Low-Power Wake-up:**

.. code-block:: bash

   west build -b alif_e7_dk/ae722f80f55d5xx0/rtss_he ../alif/tests/drivers/lptimer \
     -DDTC_OVERLAY_FILE=$PWD/../alif/tests/drivers/lptimer/boards/alif_ensemble_lptimer0.overlay \
     -DOVERLAY_CONFIG=$PWD/../alif/tests/drivers/lptimer/boards/alif_lptimer_wakeup.conf

Test Suites
-----------

**counter_basic:** Standard Zephyr counter API tests (alarms, top values, channels)

**counter_no_callback:** Counter operations without callbacks

**test_LPTimer:** Alif-specific LPTIMER tests (32MHz clock, timing validation, error handling)

**lptimer_external_clock_source:** External clock source validation (GPIO-driven signals)

**lptimer_128k_clock_source:** 128kHz internal clock source tests

**lptimer_Output_toggle:** Output toggle mode functionality

**lptimer_cascade:** 64-bit cascaded timer operations (LPTIMER pairs: 0+1, 1+2, 3+4)

**lptimer_wakeup:** Low-power mode wake-up functionality (STANDBY/SUSPEND_TO_IDLE)

**Stress_test:** Repeated alarm set/cancel cycles

Notes
-----

- Replace board name with your target (``alif_e8_dk``, ``alif_ensemble``, ``alif_spark``)
- Add ``/rtss_hp`` suffix for HP cores (default is HE core)
- Tests require hardware; simulation/emulation not supported
- Use ``DTC_OVERLAY_FILE`` and ``OVERLAY_CONFIG`` for configuration
- Board overlays available in ``boards/`` directory
