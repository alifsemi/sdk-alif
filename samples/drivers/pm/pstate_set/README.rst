.. _alif-pstate-set-sample:

Alif RTSS P-state set
#####################

Overview
********

Walks every enabled child of ``/performance-states`` with
``cpu_freq_pstate_set()``. Uses the board default console (uart2 on HE,
uart4 on HP). No extra UART or pinctrl.

After each P-state the sample prints Secure Enclave EXTSYS, AXI, AHB,
and APB frequencies, compares ``sys_clock_hw_cycles_per_sec()`` to EXTSYS, then
sleeps ``k_msleep(1000)`` and compares that interval to RTC (``rtc0``),
the only up-counter. ``k_uptime`` alone cannot prove SysTick: it uses
the same clock. RTC does not.

The SoC dtsi lists the **PLL and HFRC** states. Only the boot PLL
row is okay. The snippet sets ``status = okay`` on the demo subset.
Walk order is DT child order:

* HE: 160 / 80 MHz PLL, then HFRC 76.8 RC with scaled 76.8 / 38.4 /
  19.2 MHz. 120 / 60 MHz PLL rows are in dtsi (disabled); enable them
  in an overlay to include them in the walk.
* HP: 400 / 100 MHz PLL, then the same three HFRC rows. The 200 MHz
  PLL row is in dtsi (disabled).

SE may reject a combination. That row prints the errno and the walk
continues.

Requirements
************

* Alif Ensemble or Balletto RTSS board
* ``CONFIG_ALIF_CPU_FREQ_PSTATE`` (set in ``prj.conf``)
* Matching snippet: ``pstate-he`` or ``pstate-hp``

Building and Running
********************

HE
==

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/pstate_set
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: pstate-he

HP
==

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/pstate_set
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: pstate-hp

Expected console
****************

::

   pstate_set: Demo for EXTSYS1 using reference clock lprtc@42000000, N states

   [1/N] pstate-pll-160: applying
   [1/N] pstate-pll-160: current clocks after apply  CPU(EXTSYS1)=160.0 MHz  AXI=...  AHB=...  APB=...  SysTick=160.0 MHz
   [1/N] pstate-pll-160: slept 1 s to check SysTick against the RTC wall clock
   [1/N] pstate-pll-160:   kernel uptime 1000 ms (from SysTick)
   [1/N] pstate-pll-160:   RTC wall clock 1000 ms (independent of CPU clock)
   [1/N] pstate-pll-160:   CPU cycles in that second ... (expect about 160.0 MHz)
   [1/N] pstate-pll-160: PASS — 1 s sleep matches RTC and EXTSYS1
   ...
   pstate_set: Demo PASSED (N states)
