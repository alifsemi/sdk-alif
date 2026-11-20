.. _alif-test-pstate:

P-states in a driver test
=========================

``alif/tests/common`` runs the ztest suites already compiled into an image
once per selected RTSS P-state. Do not add a new ``ZTEST`` and do not edit
the existing case bodies.

Files
-----

* ``Kconfig.pstate`` selects the rows.
* ``pstate_common.overlay`` sets ``status = "okay"`` on those rows. HP and
  HE PLL labels are in this one file.
* ``pstate_dt.cmake`` forwards the Kconfig symbols into the devicetree
  preprocessor. This Zephyr preprocesses overlays before ``autoconf.h``
  exists.
* ``pstate_main.c`` replaces the weak ``test_main()``. For each okay row it
  idles the console, calls ``cpu_freq_pstate_set()``, prints EXTSYS, AHB,
  APB, and SysTick, then calls ``ztest_run_all()``.

With ``CONFIG_ALIF_CPU_FREQ_PSTATE`` left off, ``pstate_main.c`` is not
linked. The overlay include then enables nothing, and the test stays on
the boot PLL.

Include the files
-----------------

These paths are for a test at ``alif/tests/drivers/<name>/``. A test nested
deeper must change ``../../common``.

``CMakeLists.txt``, before ``find_package(Zephyr)``:

.. code-block:: cmake

   include(${CMAKE_CURRENT_SOURCE_DIR}/../../common/pstate_dt.cmake)

And with the other sources:

.. code-block:: cmake

   target_sources_ifdef(CONFIG_ALIF_CPU_FREQ_PSTATE app PRIVATE
                        ../../common/pstate_main.c)

Test ``Kconfig``, before ``source "Kconfig.zephyr"``:

.. code-block:: kconfig

   rsource "../../common/Kconfig.pstate"

Board overlay:

.. code-block:: dts

   #include "pstate_common.overlay"

Use that quoted name. ``pstate_dt.cmake`` adds ``alif/tests/common`` to the
devicetree include path. A ``../../common/pstate_common.overlay`` include
does not resolve, because Zephyr force-includes the overlay from the
command line.

Select the rows
---------------

``CONFIG_ALIF_CPU_FREQ_PSTATE=y`` is required. ``CONFIG_POWER_DOMAIN`` is
not. The hook already selects ``CONFIG_PM_DEVICE``.

* One PLL row: ``CONFIG_ALIF_TEST_PSTATE_PLL_200`` or ``_100`` on HP;
  ``_120``, ``_80``, or ``_60`` on HE, E1C, and Balletto. HP has no 60 MHz
  PLL row.
* One HFRC row: ``CONFIG_ALIF_TEST_PSTATE_HFRC_76P8`` through
  ``_0P6``.
* ``CONFIG_ALIF_TEST_PSTATE_ALL=y`` enables every extra PLL row and HFRC at
  76.8, 38.4, and 19.2 MHz. The 9.6 MHz through 0.6 MHz rows stay off
  unless their own symbol is set. Those slow rows are hidden while ``ALL``
  is on.

The boot PLL is always in the walk: 400 MHz on HP (``pstate_pll_400``),
160 MHz on HE (``pstate_pll_160``).

Configs
-------

Add these to the test ``prj.conf``, an overlay conf, or the twister
``extra_configs``. The hook is required. Pick one selection.

Every extra PLL row and HFRC down to 19.2 MHz:

.. code-block:: kconfig

   CONFIG_ALIF_CPU_FREQ_PSTATE=y
   CONFIG_ALIF_TEST_PSTATE_ALL=y

One extra PLL row, in addition to the boot PLL. Use a symbol this core
has. HP: ``CONFIG_ALIF_TEST_PSTATE_PLL_200`` or ``_100``. HE, E1C, and
Balletto: ``_120``, ``_80``, or ``_60``.

.. code-block:: kconfig

   CONFIG_ALIF_CPU_FREQ_PSTATE=y
   CONFIG_ALIF_TEST_PSTATE_PLL_200=y

One HFRC row. ``_76P8``, ``_38P4``, and ``_19P2`` are also covered by
``ALL``. ``_9P6`` through ``_0P6`` are only available on their own.

.. code-block:: kconfig

   CONFIG_ALIF_CPU_FREQ_PSTATE=y
   CONFIG_ALIF_TEST_PSTATE_HFRC_19P2=y

Several individual rows can be set together. They cannot be combined
with ``CONFIG_ALIF_TEST_PSTATE_ALL``.

Before the suites for that row start, the log prints the clocks just read
back:

.. code-block:: text

   P-state <node> clocks: EXTSYS1=<hz> Hz AHB=<hz> Hz APB=<hz> Hz SysTick=<hz> Hz
   === <node> (1/N) ===

EXTSYS0 is the HP CPU clock. EXTSYS1 is the HE CPU clock. A failed
``cpu_freq_pstate_set()`` or a failed clock read skips that frequency.
The boot PLL is restored after the last pass.
