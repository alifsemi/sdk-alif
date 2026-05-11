.. SPDX-License-Identifier: LicenseRef-Alif

.. Copyright Alif Semiconductor - All Rights Reserved.
   Use, distribution and modification of this code is permitted under the
   terms stated in the Alif Semiconductor Software License Agreement.
   You should have received a copy of the Alif Semiconductor Software
   License Agreement with this file. If not, please write to:
   contact@alifsemi.com, or visit: https://alifsemi.com/license

GPIO Test Cases
###############

This test suite validates the Alif GPIO driver functionality including basic checks,
performance/stress tests, and interrupt handling.

Test Suites:
============

1. gpio_check
   - test_gpio_direction_query: Tests GPIO pin direction query functionality
   - test_gpio_invalid_pin_index: Tests handling of invalid pin indices
   - test_gpio_invalid_simultaneous_in_out: Tests prevention of simultaneous input/output config
   - test_gpio_reconfigure_cycles: Tests pin reconfiguration between input/output modes

2. gpio_stress
   - test_gpio_toggle_stress: Stress test with 10000 toggle iterations
   - test_gpio_pin_set_performance: Performance test for gpio_pin_set_dt
   - test_gpio_pin_get_performance: Performance test for gpio_pin_get_dt
   - test_gpio_pin_toggle_performance: Performance test for gpio_pin_toggle_dt
   - test_gpio_interrupt_storm: Interrupt storm test with 200 edge drives

3. gpio_test_interrupts
   - test_gpio_multi_input_single_output_interrupt: Tests multiple inputs driven by single output

Hardware Requirements:
=====================
- Alif E8 DK board
- Jumper wires for GPIO connections
- Wiring: Connect P6_4 (output) to P1_4 (input1) and P1_6 (input2)

.. note::
   The above wiring instructions apply to the alif_ensemble_dk_rtss_he.overlay.
   Other board-specific overlays may use different GPIO pins. Refer to the
   overlay file for your specific board for the correct pin assignments.

Test Coverage:
==============
- Alif GPIO tests: 10 tests covering Alif-specific GPIO functionality
- Alif LPGPIO tests: 17 tests total (10 Alif-specific tests + 7 upstream tests)
- Combined (Alif GPIO + Upstream, RTSS-HE overlay only): 17 tests

Note: There are NO duplicate test cases between Alif GPIO/LPGPIO and upstream Zephyr GPIO tests.
Both test suites provide complementary coverage and should be run together where the
required upstream devicetree nodes are present; in this directory, combined coverage is
currently documented only for the RTSS-HE overlay.

LPGPIO Tests:
=============
The same test suite can be run with LPGPIO controller by using the alif-lpgpio snippet.
LPGPIO overlay uses:
- led0 (output): P6_2 (gpio6 pin 2)
- led1 (input): LPGPIO pin 4
- led2: Not available in LPGPIO overlay (multi-input interrupt test will skip)

Build Commands:
===============

Alif Balletto B1 DK (RTSS-HE):
.. code-block:: bash

   # GPIO
   west build -b alif_b1_dk/rtss_he tests/drivers/gpio/ -DSNIPPET=alif-gpio

.. note::
   LPGPIO is not supported for ``alif_b1_dk/rtss_he`` by the ``alif-lpgpio``
   snippet in this directory.

Alif Ensemble E1C DK (RTSS-HE):
.. code-block:: bash

   # GPIO
   west build -b alif_e1c_dk/rtss_he tests/drivers/gpio/ -DSNIPPET=alif-gpio

Alif Ensemble E7 DK (RTSS-HE):
.. code-block:: bash

   # GPIO
   west build -b alif_e7_dk/rtss_he tests/drivers/gpio/ -DSNIPPET=alif-gpio

   # LPGPIO
   west build -b alif_e7_dk/rtss_he tests/drivers/gpio/ -DSNIPPET=alif-lpgpio

Alif Ensemble E7 DK (RTSS-HP):
.. code-block:: bash

   # GPIO
   west build -b alif_e7_dk/rtss_hp tests/drivers/gpio/ -DSNIPPET=alif-gpio

Alif Ensemble E8 DK (RTSS-HE):
.. code-block:: bash

   # GPIO
   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he tests/drivers/gpio/ -DSNIPPET=alif-gpio

   # LPGPIO
   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he tests/drivers/gpio/ -DSNIPPET=alif-lpgpio
