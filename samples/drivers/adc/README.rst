.. zephyr:code-sample:: adc12
   name: Analog-to-Digital Converter (ADC12)
Read analog input from ADC12 channels

Overview
********

This sample app demonstrates the usage of the Analog-to-Digital Conversion (ADC) driver. It displays the temperature based
on the board's temperature, which is converted into an analog signal and fed as input to ADC channel 6.

The temperature will only be displayed if the digital output is in 12-bit format.

Building and Running
********************

The application will build only for a target that has a devicetree entry with :dt compatible:`alif,adc` as a compatible.

.. note::

   - For the ``alif_e1c_dk_rtss_he`` and ``alif_b1_dk_rtss_he`` boards:
     - **ADC12 instance 0**:
       - Channels 0 and 1 pins are multiplexed with ``SE_UART_RX`` and ``SE_UART_TX``. These pins can be used after removing the SE UART jumpers
         and providing analog input to the respective pins after flashing the SE firmware.
       - Channel 2 is connected to the ``OSPI0_SS0`` pin (OSPI chip select) and cannot be used while flashing from the SE tool with OSPI XIP.
     - **ADC12 instance 0 and 1**:
       - Channel 7 is not usable as it is not connected to the internal VREF.

.. code-block:: console

	*** Booting Zephyr OS build ZAS-v1.0.0-rc1-62-g579264343215 ***
	[00:00:00.000,000] <inf> ALIF_ADC: Allocated memory buffer Address is 0x20002050
	[00:00:00.000,000] <inf> ALIF_ADC: Current temp 21.2 C
	[00:00:00.000,000] <inf> ALIF_ADC: ADC sampling Done

Optional: DMA and UTIMER-triggered sampling
*******************************************

The sample can be built to exercise two additional code paths in the
Alif ADC driver. Both are opt-in and share the same ``main.c`` – behaviour
is selected at build time via Kconfig options and a devicetree overlay.

DMA
===

Enable in ``prj.conf``:

.. code-block:: kconfig

   CONFIG_ADC_ALIF_DMA=y
   CONFIG_DMA=y

and add the following to the board overlay:

.. code-block:: dts

   &dma2       { status = "okay"; };
   &evtrtr2    { status = "okay"; };

   &adc0 {
       status = "okay";
       adc_conversion_mode = "CONTINUOUS_CONVERSION";
       dmas      = <&evtrtr2 ALIF_DMA_ENCODE(0, 0, 1) 4>;
       dma-names = "rxdma";
   };

In DMA mode the sample collects ``ADC_DMA_NUM_SAMPLES`` samples per
``adc_read()`` call (1 by default) and prints each one as a temperature
value.

UTIMER hardware trigger
=======================

Enable in ``prj.conf``:

.. code-block:: kconfig

   CONFIG_ADC_ALIF_UTIMER_TRIGGER=y
   CONFIG_USE_ALIF_HAL_UTIMER=y

and describe the UTIMER wiring in the overlay:

.. code-block:: dts

   &utimer0 { status = "okay"; };

   &adc0 {
       status = "okay";
       alif,utimer-trigger  = <&utimer0>;
       alif,utimer-driver   = <0>;
       alif,utimer-period   = <160000000>;
       alif,ext-trigger-src = <0x1>;   /* bit 0 = UTIMER0 driver A */
   };

The driver programs UTIMER0 in ``adc_init()`` and arms the ADC to fire on
the selected external trigger source. From then on, one UTIMER pulse
triggers one ADC conversion in hardware. Without DMA, the done interrupt
copies the sample; with DMA, the transfer is CPU-free per sample.

When combined with DMA the two are complementary: UTIMER paces the ADC,
DMA moves the samples. The DMA-mode ``CONTINUOUS_CONVERSION`` restriction
is lifted when UTIMER is configured, so ``SINGLE_SHOT_CONVERSION`` is
also allowed.

Example console output (UTIMER + DMA, ``SINGLE_SHOT_CONVERSION``,
``alif,utimer-period = <160000000>``, single temperature sample per read):

.. code-block:: console

   *** Booting Zephyr OS build ... ***
   [00:00:00.000,000] <inf> ALIF_ADC: ADC DMA mode - collecting 1 samples per read
   [00:00:00.000,000] <inf> ADC: ADC UTIMER trigger: timer_id=0 drv=0 period=160000000 src=0x1
   [00:00:00.200,000] <inf> ALIF_ADC: adc_read() blocked for 200 ms (1 samples)
   [00:00:00.200,000] <inf> ALIF_ADC: Sample[0]: 38.8 C
   [00:00:01.400,000] <inf> ALIF_ADC: adc_read() blocked for 200 ms (1 samples)
   [00:00:01.400,000] <inf> ALIF_ADC: Sample[0]: 38.8 C

