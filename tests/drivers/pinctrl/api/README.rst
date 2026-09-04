.. _pinctrl_api_ztest:

Pinctrl API Test Suite
######################

Overview
********

This suite validates the Alif pinctrl driver through the Zephyr pinctrl
API. It checks Devicetree pin encoding, state lookup/apply, pad-config
bit fields, and dynamic state updates.

The tests use two UART nodes as pinctrl clients (the UARTs themselves are
not exercised):

- **dev0** (uart2 on Ensemble, uart3 on Balletto/E1C): default state only
  (sleep is present in the overlay but skipped without ``CONFIG_PM_DEVICE``)
- **dev1** (uart6 on Ensemble, uart5 on Balletto/E1C): default + custom
  ``mystate``

Validation uses Zephyr APIs only (``pinctrl_lookup_state()``,
``pinctrl_apply_state()``, ``pinctrl_apply_state_direct()``,
``pinctrl_configure_pins()``, ``pinctrl_update_states()``) plus the
encoded pin data from ``PINCTRL_DT_DEV_CONFIG_GET``. The suite does not
read or write pinctrl registers directly.

Requirements
************

Hardware
========

- Alif Ensemble E7/E8 DK (RTSS-HE or RTSS-HP), or
- Alif Balletto B1 DK / Ensemble E1C DK (RTSS-HE)

The suite remuxes **P1_0 .. P1_4**. Do not use those pins for the console
UART or JTAG while the test is running.

Software
========

``prj.conf`` enables:

- ``CONFIG_ZTEST=y``
- ``CONFIG_PINCTRL=y``
- ``CONFIG_PINCTRL_DYNAMIC=y`` (required for tests 11 and 12)
- ``CONFIG_PINCTRL_TEST_STORE_REG=y`` (stores peripheral register addresses)

Test Cases
**********

The ``pinctrl_api`` suite contains 12 tests (10 always-on, 2 gated by
``CONFIG_PINCTRL_DYNAMIC``):

#. ``test_metadata`` -- state count, pin count, state IDs, and (when
   ``CONFIG_PINCTRL_STORE_REG``) UART register addresses from DT.
#. ``test_pin_encoding`` -- port, function, and DSC bit fields for every
   pin in every state.
#. ``test_lookup`` -- ``pinctrl_lookup_state()`` for DEFAULT, MYSTATE,
   skipped SLEEP, and an unknown ID (``-ENOENT``).
#. ``test_apply_all_states`` -- apply every state via
   ``pinctrl_apply_state()`` and ``pinctrl_apply_state_direct()``.
#. ``test_apply_invalid_id`` -- unknown state ID returns ``-ENOENT``.
#. ``test_apply_transitions`` -- idempotent apply and 16× DEFAULT/MYSTATE
   toggle.
#. ``test_pad_config_fields`` -- REN, STE, SRE, DSC, ODS, and DRV fields
   against overlay / DT defaults.
#. ``test_apply_default_restore`` -- MYSTATE then DEFAULT via
   ``pinctrl_apply_state()``.
#. ``test_devices_independent`` -- devices use disjoint pins and can be
   applied independently.
#. ``test_configure_pins`` -- ``pinctrl_configure_pins()`` with
   ``PINCTRL_REG_NONE``.
#. ``test_update_states_valid`` -- ``pinctrl_update_states()`` swap,
   apply, and restore (``CONFIG_PINCTRL_DYNAMIC``).
#. ``test_update_states_invalid`` -- wrong state count or unknown ID
   returns ``-EINVAL`` (``CONFIG_PINCTRL_DYNAMIC``).

Board Overlays
**************

- ``boards/alif_ensemble_pinctrl.overlay`` -- E7/E8 (uart2 + uart6)
- ``boards/alif_balletto_pinctrl.overlay`` -- B1/E1C (uart3 + uart5)

dev1 ``mystate`` sets non-default pad config so encoding can be checked:

- group0 (P1_2): ``read-enable = 0``
- group1 (P1_3): DSC=1, ``slew-rate = 1``, ``drive-strength = 12``
- group2 (P1_4): DSC=2, ``schmitt-enable = 0``, ``driver = 1``

Building and Running
********************

Board selection is covered in the Alif SDK user guide. Build from the
west workspace root using ``tests/drivers/pinctrl/api``.

Ensemble (E7/E8)
================

.. code-block:: console

   west build -p always -b <board> tests/drivers/pinctrl/api \
              -- -DDTC_OVERLAY_FILE=boards/alif_ensemble_pinctrl.overlay
   west flash

Balletto B1 / Ensemble E1C
==========================

.. code-block:: console

   west build -p always -b <board> tests/drivers/pinctrl/api \
              -- -DDTC_OVERLAY_FILE=boards/alif_balletto_pinctrl.overlay
   west flash

Sample Output
*************

.. code-block:: console

   *** Booting Zephyr OS ***
   Running TESTSUITE pinctrl_api
   ===================================================================
   START - test_metadata
    PASS - test_metadata
   ===================================================================
   START - test_pin_encoding
    PASS - test_pin_encoding
   ===================================================================
   START - test_lookup
    PASS - test_lookup
   ===================================================================
   START - test_apply_all_states
    PASS - test_apply_all_states
   ===================================================================
   START - test_apply_invalid_id
    PASS - test_apply_invalid_id
   ===================================================================
   START - test_apply_transitions
    PASS - test_apply_transitions
   ===================================================================
   START - test_pad_config_fields
    PASS - test_pad_config_fields
   ===================================================================
   START - test_apply_default_restore
    PASS - test_apply_default_restore
   ===================================================================
   START - test_devices_independent
    PASS - test_devices_independent
   ===================================================================
   START - test_configure_pins
    PASS - test_configure_pins
   ===================================================================
   START - test_update_states_valid
    PASS - test_update_states_valid
   ===================================================================
   START - test_update_states_invalid
    PASS - test_update_states_invalid
   ===================================================================
   TESTSUITE pinctrl_api succeeded

   ------ TESTSUITE SUMMARY START ------

   SUITE PASS - 100.00% [pinctrl_api]: pass = 12, fail = 0, skip = 0, total = 12

   ------ TESTSUITE SUMMARY END ------

   PROJECT EXECUTION SUCCESSFUL

Notes
*****

- Sleep is declared on dev0 so the suite can confirm it is omitted when
  ``CONFIG_PM_DEVICE`` is disabled.
- Suite teardown restores DEFAULT on both devices via
  ``pinctrl_apply_state()``.
