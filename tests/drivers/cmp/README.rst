.. zephyr:code-sample:: alif-cmp
	name: Analog Comparator (CMP)

	Verify the Analog Comparator (CMP) functionality including hysteresis, polarity, and filter tap settings.

#########
Overview
#########

This test suite validates the Analog Comparator (CMP) driver for Alif Semiconductor devices. It covers basic comparison, polarity inversion, and digital filter configurations.

The tests verify:
* **Basic Comparison**: Validates the output state based on positive and negative input levels.
* **Devicetree properties**: Checks which ``alif,cmp`` properties apply to high-speed CMP vs LPCMP.
* **Hysteresis**: Verification of different hysteresis levels (6mV to 36mV). Applies to both CMP and LPCMP.
* **Digital Filter**: Testing of various filter tap configurations (1 to 4 taps). High-speed CMP only.
* **Polarity**: Verification of output polarity inversion. High-speed CMP only.
* **Reference Source**: High-speed CMP: DAC6, internal Vref, external pin. LPCMP: external analog pin only (P2_0 default).
* **Trigger Modes**: Verification of all trigger configurations (rising edge, falling edge, both edges, none).
* **Output & Pending**: Verification of ``comparator_get_output`` and ``comparator_trigger_is_pending`` APIs. High-speed CMP only (``cmp-gpios``).
* **Negative Tests**: Robustness tests including callback overwrite, trigger without callback, and repeated reads.
* **Response-Time Feature**: Manual hardware validation (CMP ``< 5 ns``, LPCMP ``< 10 μs``).

LPCMP vs CMP properties
***********************

LPCMP is a separate module in PD0 (HRM 19.4): rail-to-rail, supply
``VDD_IO_1V8``, programmable hysteresis, response time ``< 10 μs``, and the
LPCMP interrupt can wake from STANDBY and STOP. It uses the same ``alif,cmp``
compatible with ``CMP_INSTANCE_LP``. There is no board ``cmp-gpios`` output
and no DAC6 path.

* **Applicable (tested)**: ``driver_instance``, ``positive_input``, ``negative_input``, ``hysteresis_level``
* **Not applicable (tests skip)**: ``filter_taps``, ``prescaler``, ``polarity_en``, ``cmp-gpios``, DAC6

LPCMP analog pins (``pinctrl_lpcmp``)
************************************

LPCMP has **no internal negative** (no AON / DAC6). Both sides are analog pads:

Positive:

* ``CMP_POS_IN0`` = **P2_4** (``ANA_S20``), ``IN1`` = P2_5, ``IN2`` = P2_6, ``IN3`` = P2_7

Negative (external only):

* ``CMP_NEG_IN1`` = **P2_0** (``ANA_S16``, default), ``IN2`` = P2_1, ``IN3`` = P2_2

How to check LPCMP
******************

Do **not** jumper the LED into the analog path. Drive both comparator pads
from analog sources. On each LPCMP IRQ the test turns **aled0** on (~300 ms)
then off.

**E7/E8 DK** (``boards/alif_lpcmp.overlay``):

* Positive: analog on **P2_4** (``CMP_POS_IN0``)
* Negative: analog / DC on **P2_0** (``CMP_NEG_IN0``)
* IRQ LED: **P12_3** (no jumper to P2_4)

**B1/E1C DK** (``boards/alif_lpcmp_spark.overlay``):

* Positive: analog on **P2_4**
* Negative: analog / DC on **P2_0**
* IRQ LED: **P4_5**

Raise POS above NEG (unplug/replug P2_4, or sweep the analog source) within
20 s per event. LPCMP typically IRQs when POS > NEG; falling POS often
produces no IRQ.

STANDBY/STOP wake-from-LPCMP is not covered by this ztest suite.

Building and Running
********************

The application will build only for a target that has a devicetree entry with
the compatible ``alif,cmp``. Run all commands from the ``alif/`` module
directory. Replace ``<board>`` with the west board identifier.

Overlay selection:

* E7/E8: ``boards/alif_lpcmp.overlay`` and ``boards/alif_cmp1.overlay``
* B1/E1C (Spark): ``boards/alif_lpcmp_spark.overlay`` and
  ``boards/alif_cmp1_spark.overlay``

LPCMP is not present on E8. Do not combine LPCMP overlays with DAC6, Vref,
polarity, or filter-tap overlays (those tests skip on LPCMP).

Per board: 8 LPCMP builds + 14 high-speed CMP builds = 22.

LPCMP builds
************

**Default** (42 mV):

.. code-block:: console

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_lpcmp.overlay"

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_lpcmp_spark.overlay"

**External NEG** (``CMP_NEG_IN1`` / P2_0):

.. code-block:: console

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_lpcmp.overlay;boards/alif_lpcmp_ref_ext.overlay"

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_lpcmp_spark.overlay;boards/alif_lpcmp_ref_ext.overlay"

**Hysteresis** (unskips the matching ``test_Comp_Hys_*``). Repeat for
``hys12``, ``hys18``, ``hys24``, ``hys30``, ``hys36``. On Spark, replace
``alif_lpcmp.overlay`` with ``alif_lpcmp_spark.overlay``:

.. code-block:: console

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_lpcmp.overlay;boards/alif_lpcmp_hys6.overlay"

High-speed CMP builds
*********************

**Default CMP1** (filter_taps=3, 42 mV):

.. code-block:: console

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_cmp1.overlay"

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_cmp1_spark.overlay"

**Hysteresis.** Pass the matching Kconfig so the hysteresis testcase is
enabled. Repeat for ``hys12`` / ``HY12``, ``hys18`` / ``HY18``,
``hys24`` / ``HY24``, ``hys30`` / ``HY30``, ``hys36`` / ``HY36``.
On Spark, replace ``alif_cmp1.overlay`` with ``alif_cmp1_spark.overlay``:

.. code-block:: console

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_cmp1.overlay;boards/alif_hys6.overlay" \
       -DCONFIG_HY6=y

**Reference source:**

DAC6 (``CMP_NEG_IN3``, ``test_Comp_ref_dac6``):

.. code-block:: console

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_cmp1.overlay;boards/alif_ref_dac6.overlay" \
       -DCONFIG_REF_DAC6=y

Internal Vref (``CMP_NEG_IN2``, ``test_Comp_int_vref``):

.. code-block:: console

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_cmp1.overlay;boards/alif_ref_vref.overlay" \
       -DCONFIG_REF_VREF=y

External pin (``CMP_NEG_IN0``, ``test_Comp_ref_external_pin``). Requires an
external voltage on the CMP negative pad. If the board does not expose that
pin, use DAC6 or internal Vref instead:

.. code-block:: console

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_cmp1.overlay;boards/alif_ref_ext.overlay" \
       -DCONFIG_REF_EXT=y

**Filter taps** (default overlay already covers taps=3):

.. code-block:: console

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_cmp1.overlay;boards/alif_cmp1_ft1.overlay"

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_cmp1.overlay;boards/alif_cmp1_ft2.overlay"

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_cmp1.overlay;boards/alif_cmp1_ft4.overlay"

**Polarity:**

.. code-block:: console

   west build -p always -b <board> tests/drivers/cmp \
       -DDTC_OVERLAY_FILE="boards/alif_cmp1.overlay;boards/alif_cmp1_polarity.overlay"

Response-Time Validation
************************

The testcase ``test_Comp_response_time_manual_hw_validation`` is added as a
manual hardware-validation hook. It configures the trigger/callback path and
marks the test as skipped in ztest, because absolute delay cannot be validated
from software timing.

* High-speed CMP: ``< 5 ns``
* LPCMP (HRM 19.4): ``< 10 μs``

Use a signal generator and oscilloscope from input crossing to comparator
output (CMP) or interrupt assertion (LPCMP; no board output pin).
