# Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

Connect P6_4 (led0 output) to P1_4 (led1 input) and P1_6 (led2 input)
Wiring: P6_4 → P1_4 and P6_4 → P1_6

.. GPIO_testcases:

Testcases
######
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

Test Specification:
==================
- Alif GPIO tests: See test_specification_alif_gpio_automation.xml (10 tests)
- Alif LPGPIO tests: See test_specification_alif_lpgpio_automation.xml (17 tests: 10 Alif + 7 Upstream)
- Combined (Alif GPIO + Upstream): See test_specification_gpio_combined.xml (17 tests)
- Test comparison: See TEST_COMPARISON.md for analysis of Alif vs Upstream tests

Note: There are NO duplicate test cases between Alif GPIO/LPGPIO and upstream Zephyr GPIO tests.
Both test suites provide complementary coverage and should be run together.

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

   # LPGPIO
   west build -b alif_b1_dk/rtss_he tests/drivers/gpio/ -DSNIPPET=alif-lpgpio

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
