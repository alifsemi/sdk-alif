.. _clkctrl_ztest:

Alif Clock Control Tests
========================

Overview
********

This test suite validates the Alif clock control driver through the Zephyr
``clock_control`` API. It targets the ``clockctrl`` device-tree node
(compatible ``alif,clk``) and covers get-rate, set-rate, enable/disable, and
error-path behavior that is not already covered by the upstream
``clock_control_api`` tests.

The suite currently contains 10 ``ZTEST()`` cases in ``src/test_clkctrl.c``.

Requirements
************

- A board with a ``clockctrl`` node (``DT_NODELABEL(clockctrl)``).
- ``CONFIG_CLOCK_CONTROL=y`` (enabled in ``prj.conf``).
- Ensemble (E7/E8) or Balletto (B1) SoC family clock bindings.

No extra overlay is required. The test uses clocks that are present on Alif
platforms (UART0 PCLK, UTIMER, LPTIMER0 32 kHz, ADC0, GPIO0 debounce).


Test Cases
**********

- ``test_clock_get_rate``: ``clock_control_get_rate`` on UART0 system PCLK
  returns a non-zero frequency.
- ``test_clock_get_rate_multiple_clocks``: get-rate on UTIMER and LPTIMER0
  32 kHz clocks. Skips if none of the clocks support get-rate.
- ``test_clock_get_rate_all_domains``: get-rate across UART, UTIMER, LPTIMER,
  and GPIO debounce clocks. Skips if none of the clocks support get-rate.
- ``test_invalid_subsystem_get_rate``: get-rate with an invalid clock ID fails.
- ``test_clock_set_rate``: set-rate to the current GPIO0 debounce frequency
  succeeds and leaves the rate unchanged.
- ``test_clock_set_rate_unsupported``: set-rate on a clock with no divisor
  (UART0 PCLK) returns ``-ENOTSUP`` or 0.
- ``test_clock_on_already_enabled``: a second ``clock_control_on`` on ADC0
  returns ``-EALREADY`` (Ensemble) or 0 (Balletto). Turns ADC0 off only if
  this test enabled it.
- ``test_clock_get_rate_consistency``: ADC0 get-rate is unchanged across an
  on/off cycle. Restores ADC0 if it was already enabled. UART clock is not
  gated.
- ``test_clock_status_unsupported_clock``: dummy clock (UTIMER, ``en_mask == 0``)
  reports ``CLOCK_CONTROL_STATUS_UNKNOWN``.
- ``test_clock_on_off_unsupported_clock``: on/off of the dummy UTIMER clock
  returns 0.

Basic on/off/status coverage is left to the upstream ``clock_control_api``
tests.

Configuration
*************

Key Kconfig options set in ``prj.conf``:

- ``CONFIG_ZTEST=y``
- ``CONFIG_CLOCK_CONTROL=y``
- ``CONFIG_LOG=y``
- ``CONFIG_LOG_MODE_DEFERRED=y``
- ``CONFIG_LOG_DEFAULT_LEVEL=3``

Building and Running
********************

Board selection is covered in the user guide.

.. code-block:: console

   west build -p always board_type tests/drivers/clkctrl

   west flash
