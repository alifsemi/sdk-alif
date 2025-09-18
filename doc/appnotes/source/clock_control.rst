.. _appnote-zephyr-clock-control:

=============
Clock Control
=============

Introduction
============

The Clock Control API is the essential tool for managing clock signals in your embedded system. This application note provides a comprehensive overview of the API's capabilities, demonstrating how to control clock power states, adjust frequencies, configure sources, and monitor clock status. With the Clock Control API, you can achieve precise timing control to optimize system performance and power consumption.

Available Clocks
================

The following table lists all the available clocks:

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Clock Name
     - Description
   * - ALIF_CAMERA_PIX_SYST_ACLK
     - System ACLK for camera pixel interface
   * - ALIF_CAMERA_PIX_PLL_CLK3
     - PLL CLK3 for camera pixel interface
   * - ALIF_CDC200_PIX_SYST_ACLK
     - System ACLK for CDC200 pixel interface
   * - ALIF_CDC200_PIX_PLL_CLK3
     - PLL CLK3 for CDC200 pixel interface
   * - ALIF_CSI_PIX_SYST_ACLK
     - System ACLK for CSI pixel interface
   * - ALIF_CSI_PIX_PLL_CLK3
     - PLL CLK3 for CSI pixel interface
   * - ALIF_CPI_CLK
     - Clock for CPI interface
   * - ALIF_DPI_CLK
     - Clock for DPI interface
   * - ALIF_DMA0_CLK
     - Clock for DMA0
   * - ALIF_GPU_CLK
     - Clock for GPU
   * - ALIF_ETHERNET_CLK
     - Clock for Ethernet
   * - ALIF_ETH_RMII_REFCLK_PIN
     - Reference clock pin for RMII Ethernet
   * - ALIF_ETH_RMII_PLL_CLK_50M
     - 50MHz PLL clock for RMII Ethernet
   * - ALIF_SDC_CLK
     - Clock for SDC
   * - ALIF_USB_CLK
     - Clock for USB
   * - ALIF_CSI_CLK
     - Clock for CSI
   * - ALIF_DSI_CLK
     - Clock for DSI
   * - ALIF_MIPI_TXDPHY_CLK
     - Clock for MIPI TXDPHY
   * - ALIF_MIPI_RXDPHY_CLK
     - Clock for MIPI RXDPHY
   * - ALIF_MIPI_PLLREF_CLK
     - Reference clock for MIPI PLL
   * - ALIF_MIPI_BYPASS_CLK
     - Bypass clock for MIPI
   * - ALIF_BACKUP_RAM_CLK
     - Clock for backup RAM
   * - ALIF_PDM_76M8_CLK
     - 76.8MHz clock for PDM
   * - ALIF_PDM_AUDIO_CLK
     - Audio clock for PDM
   * - ALIF_UARTn_38M4_CLK(n)
     - 38.4MHz clock for UARTn
   * - ALIF_UARTn_SYST_PCLK(n)
     - System PCLK for UARTn
   * - ALIF_CANFD_HFOSC_CLK
     - HFOSC clock for CANFD
   * - ALIF_CANFD_160M_CLK
     - 160MHz clock for CANFD
   * - ALIF_I2Sn_76M8_CLK(n)
     - 76.8MHz clock for I2Sn
   * - ALIF_I2Sn_AUDIO_CLK(n)
     - Audio clock for I2Sn
   * - ALIF_I3C_CLK
     - Clock for I3C
   * - ALIF_ADCn_CLK(n)
     - Clock for ADCn
   * - ALIF_ADC24_CLK
     - Clock for ADC24
   * - ALIF_DACn_CLK(n)
     - Clock for DACn
   * - ALIF_CMPn_CLK(n)
     - Clock for CMPn
   * - ALIF_GPIOn_DB_CLK(n)
     - Debounce clock for GPIOn
   * - ALIF_LPUART_CLK
     - Clock for LPUART
   * - ALIF_LPTIMERn_S32K_CLK(n)
     - 32KHz clock for LPTIMERn
   * - ALIF_LPTIMERn_128K_CLK(n)
     - 128KHz clock for LPTIMERn
   * - ALIF_LPTIMERn_IO_PIN(n)
     - IO pin clock for LPTIMERn
   * - ALIF_LPTIMERn_CASCADE_CLK(n)
     - Cascade clock for LPTIMERn
   * - ALIF_LPRTC_CLK
     - Clock for LPRTC
   * - ALIF_NPU_HE_CLK
     - High-efficiency clock for NPU
   * - ALIF_DMA2_CLK
     - Clock for DMA2
   * - ALIF_LPPDM_76M8_CLK
     - 76.8MHz clock for LPPDM
   * - ALIF_LPPDM_AUDIO_CLK
     - Audio clock for LPPDM
   * - ALIF_LPCPI_CLK
     - Clock for LPCPI
   * - ALIF_LPSPI_CLK
     - Clock for LPSPI
   * - ALIF_LPI2S_76M8_CLK
     - 76.8 MHz clock for LPI2S
   * - ALIF_LPI2S_AUDIO_CLK
     - Audio clock for LPI2S
   * - ALIF_LPCPI_PXL_CLK
     - Pixel clock for LPCPI
   * - ALIF_NPU_HP_CLK
     - High-performance clock for NPU
   * - ALIF_DMA1_CLK
     - Clock for DMA1

Clock Node
==========

The Alif Clock node is available under the SOC node of the Alif DTSI file with a compatibility string of "alif,clock". This clock node contains the base addresses for all modules.

alif,clk.yaml File
==================

The `alif,clk.yaml` file is the compatible file for the Alif clock node. This file describes the clock-cells value as 1, and the cell name is mentioned as `clkid`.

.. code-block:: dts


   clock: clock-controller@4903f000 {
       compatible = "alif,clk";
       reg = <0x4903F000 0xB0
              0x4902F000 0xBC
              0x1A604000 0x44
              0x1A609000 0x14
              0x43007000 0x2C
              0x400F0000 0x14>;

       reg-names = "master_clkctrl",
                   "slave_clkctrl",
                   "aon_clkctrl",
                   "vbat_clkctrl",
                   "m55he_clkctrl",
                   "m55hp_clkctrl";

       #clock-cells = <1>;
       status = "okay";
   };

.. note:: A build error will occur if the clock-cells value or cell name differs in the case of other vendors.

Clock Node Integration with Peripheral Nodes
============================================

The Clock node can be seamlessly integrated with other peripheral nodes by referencing a clock macro value using the existing clocks property. The following example demonstrates the integration of the Clock node with the LPTIMER instance 0:


.. code-block:: dts

    timer0: timer@42001000 {
        compatible = "snps,dw-timers";
        reg = <0x42001000 0x14>;
        interrupts = <60 3>;
        clocks = <&clock ALIF_LPTIMERn_S32K_CLK(0)>;
        status = "disabled";
    };



Clock Control API Usage
=======================

You can use the clock control APIs in the respective driver code as follows:

Enabling a Module’s Clock
-------------------------

To enable a module's clock, use the `clock_control_on` API:

.. code-block:: c

   #if DT_ANY_INST_HAS_PROP_STATUS_OKAY(clocks)
       if (!device_is_ready(timer_config->clk_dev)) {
           LOG_ERR("clock controller device not ready");
           return -ENODEV;
       }

       ret = clock_control_on(timer_config->clk_dev, timer_config->clkid);
       if (ret != 0) {
           LOG_ERR("ERROR in clock enable");
           return ret;
       }
   #endif

Disabling a Module’s Clock
--------------------------

To disable a module's clock, use the `clock_control_off` API:

.. code-block:: c

   #if DT_ANY_INST_HAS_PROP_STATUS_OKAY(clocks)
       if (!device_is_ready(timer_config->clk_dev)) {
           LOG_ERR("clock controller device not ready");
           return -ENODEV;
       }

       ret = clock_control_off(timer_config->clk_dev, timer_config->clkid);
       if (ret != 0) {
           LOG_ERR("ERROR in clock disable");
           return ret;
       }
   #endif

Getting Clock Status
--------------------

To get clock status, use the `alif_clock_control_get_status` API:

.. code-block:: c

   #if DT_ANY_INST_HAS_PROP_STATUS_OKAY(clocks)
       if (!device_is_ready(timer_config->clk_dev)) {
           LOG_ERR("clock controller device not ready");
           return -ENODEV;
       }

       ret = alif_clock_control_get_status(timer_config->clk_dev, timer_config->clkid);
       if (ret != 0) {
           LOG_ERR("ERROR in getting clock status");
           return ret;
       }
   #endif

Setting Clock Frequency
-----------------------

To set clock frequency, use the `alif_clock_control_set_rate` API:

.. code-block:: c

   #if DT_ANY_INST_HAS_PROP_STATUS_OKAY(clocks)
       if (!device_is_ready(timer_config->clk_dev)) {
           LOG_ERR("clock controller device not ready");
           return -ENODEV;
       }

       ret = clock_control_set_rate(timer_config->clk_dev, timer_config->clkid, (clock_control_subsys_rate_t) rate);
       if (ret != 0) {
           LOG_ERR("ERROR in clock frequency setting");
           return ret;
       }
   #endif

To Get the Current Running Frequency
------------------------------------

To get the current running frequency, use the `clock_control_get_rate` API:

.. code-block:: c

   #if DT_ANY_INST_HAS_PROP_STATUS_OKAY(clocks)
       if (!device_is_ready(timer_config->clk_dev)) {
           LOG_ERR("clock controller device not ready");
           return -ENODEV;
       }

       ret = clock_control_get_rate(timer_config->clk_dev, timer_config->clkid, &frequency);
       if (ret != 0) {
           LOG_ERR("ERROR in getting clock frequency");
           return ret;
       }
   #endif

To Configure Clock Source
-------------------------

To configure the clock source, use the `clock_control_configure` API:

.. code-block:: c

   #if DT_ANY_INST_HAS_PROP_STATUS_OKAY(clocks)
       if (!device_is_ready(timer_config->clk_dev)) {
           LOG_ERR("clock controller device not ready");
           return -ENODEV;
       }

       ret = clock_control_configure(timer_config->clk_dev, timer_config->clkid, NULL);
       if (ret != 0) {
           LOG_ERR("ERROR in clock source configuration");
           return ret;
       }
   #endif