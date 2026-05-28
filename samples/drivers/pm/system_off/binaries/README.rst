.. _alif-pm-sram0-binaries:

HP SRAM0 S2RAM — Companion Binaries
#####################################

Overview
********

The ``pm-system-off-s2ram-sram0`` snippet for the HP core requires two
companion binaries in addition to the Zephyr application:

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - File
     - Description
   * - ``demo_pm_M55_HP.bin``
     - HP first-stage loader: powers up SRAM0/1 via SE run profile and
       jumps to the Zephyr image.
   * - ``demo_pm_M55_HE.bin``
     - HE auto-off binary: puts the HE core into the off-state. HE domain will be
       switched ON when any low power peripherals like RTC/LPTIMER are accessed
       by other cores. The HE core must be in the off-state to test the
       complete PM state transition on HP.

.. note::
   Requires SE Firmware v1.111 or higher.

Memory Map
**********

.. code-block:: text

   MRAM
     0x8000_0000    HE auto-off binary     (demo_pm_M55_HE.bin)
     0x8010_0000    HP first-stage loader  (demo_pm_M55_HP.bin)
     0x8020_0000    HP Zephyr application  (M55_HP.bin)

   SRAM0
     0x0200_0000    Zephyr application data RAM

The Zephyr application runs from MRAM (``0x8020_0000``) with SRAM0 as data
RAM (``CONFIG_SRAM_BASE_ADDRESS = 0x02000000``).

SE and aiPM Profile Reference
******************************

The profiles below document the SE power configuration used by each
pre-built binary.  They are provided as a reference; users may adapt
wakeup sources, clock settings, and power domains to suit their own
requirements.

HP First-Stage Loader (``demo_pm_M55_HP.bin``)
===============================================

The loader applies a minimal SE run profile to power up SRAM0 and SRAM1,
then transfers execution to the Zephyr image entry point.

Reference run-profile configuration:

.. code-block:: c

   /* Enable SYSTOP and bring up SRAM0/SRAM1 before jumping to Zephyr */
   runp.power_domains = PD_SSE700_AON_MASK | PD_SYST_MASK;
   runp.memory_blocks = MRAM_MASK | SERAM_MASK | SRAM0_MASK | SRAM1_MASK;

HE Auto-Off Binary (``demo_pm_M55_HE.bin``)
============================================

The HE binary boots from MRAM, applies the SE run and off profiles below,
then immediately enters deep sleep.  The RTC is not used as an
idle timer by this binary.

Reference run and off profile configuration:

.. code-block:: c

   /* Run profile */
   runp.dcdc_voltage    = 825;
   runp.dcdc_mode       = DCDC_MODE_PWM;
   runp.run_clk_src     = CLK_SRC_PLL;
   runp.vdd_ioflex_3V3  = IOFLEX_LEVEL_1V8;
   runp.ip_clock_gating = 0;
   runp.phy_pwr_gating  = 0;
   runp.cpu_clk_freq    = CLOCK_FREQUENCY_160MHZ;
   runp.scaled_clk_freq = SCALED_FREQ_XO_LOW_DIV_38_4_MHZ;
   runp.power_domains   = PD_SYST_MASK | PD_SSE700_AON_MASK;
   runp.aon_clk_src     = CLK_SRC_LFRC;
   runp.memory_blocks   = MRAM_MASK | SERAM_MASK;

   /* Off profile (SOFT_OFF) */
   offp.power_domains   = PD_VBAT_AON_MASK;
   offp.aon_clk_src     = CLK_SRC_LFRC;
   offp.stby_clk_src    = CLK_SRC_HFRC;
   offp.stby_clk_freq   = SCALED_FREQ_RC_STDBY_76_8_MHZ;
   offp.ewic_cfg        = EWIC_RTC_A | EWIC_VBAT_TIMER | EWIC_VBAT_GPIO;
   offp.wakeup_events   = WE_LPRTC | WE_LPTIMER | WE_LPGPIO;
   offp.vtor_address    = SCB->VTOR;   /* HE MRAM boot address */
   offp.memory_blocks   = MRAM_MASK | SERAM_MASK;
   offp.dcdc_mode       = DCDC_MODE_PWM;
   offp.dcdc_voltage    = 825;

Dual-Core Setup
***************

The HP Zephyr application configures RTC as its idle timer
(``zephyr,cortex-m-idle-timer``).  Ensure no other core is using RTC
while it runs.  Two options:

**Option A — Pre-built binaries (this directory):**

Use ``demo_pm_M55_HE.bin`` and ``demo_pm_M55_HP.bin`` directly.  Flash
using the SE Tools JSON below.

**Option B — Custom implementation:**

Users may implement their own binaries that meet the same requirements,
using the reference profiles above as a starting point:

* HE binary: must enter ``PM_STATE_SOFT_OFF`` without using the RTC as an
  idle timer; the off-profile wakeup events and EWIC configuration may be
  adapted for the application.
* HP binary: must power up SRAM0 and SRAM1 via the SE run profile before
  transferring execution to the Zephyr entry point.

SE Tools JSON
*************

.. code-block:: json

   {
       "DEVICE": {
           "disabled": false,
           "binary": "app-device-config.json",
           "version": "0.5.00",
           "signed": true
       },
       "A32_APP": {
           "disabled": true,
           "binary": "a32_stub_0.bin",
           "version": "1.0.0",
           "signed": true,
           "loadAddress": "0x02000000",
           "cpu_id": "A32_0",
           "flags": ["load", "boot"]
       },
       "HP_APP_BL": {
           "disabled": false,
           "binary": "demo_pm_M55_HP.bin",
           "version": "1.0.0",
           "signed": true,
           "mramAddress": "0x80100000",
           "cpu_id": "M55_HP",
           "flags": ["boot"]
       },
       "HP_APP_Z": {
           "disabled": false,
           "binary": "M55_HP.bin",
           "version": "1.0.0",
           "signed": true,
           "mramAddress": "0x80200000",
           "cpu_id": "M55_HP",
           "flags": ["deferred"]
       },
       "HE_APP_MRAM": {
           "disabled": false,
           "binary": "demo_pm_M55_HE.bin",
           "version": "1.0.0",
           "signed": true,
           "mramAddress": "0x80000000",
           "cpu_id": "M55_HE",
           "flags": ["boot"]
       }
   }
