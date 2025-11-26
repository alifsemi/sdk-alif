.. Pin Configuration

Pin Configuration for Ensemble
==============================

This document provides a comprehensive guide to pinmux configuration and hardware setup for peripherals on Alif Semiconductor Ensemble DevKit (E7, E8, and E1C variants). Organized by peripheral, it details pin assignments, hardware connections, and setup instructions in tabulated format for clarity.

.. note::

     - **Variant Suffixes**: A, B, C, D indicate different pinmux configurations for the same peripheral instance.

GPIO Pin Setup
--------------

All pins can be configured as GPIO unless assigned to a specific peripheral.

.. list-table:: GPIO Pins
   :widths: 25 75
   :header-rows: 1
   :align: left

   * - Port
     - Pins
   * - P0
     - P0_0 to P0_7
   * - P1
     - P1_0 to P1_7
   * - P2
     - P2_0 to P2_7
   * - P3
     - P3_0 to P3_7
   * - P4
     - P4_0 to P4_7
   * - P5
     - P5_0 to P5_7
   * - P6
     - P6_0 to P6_7
   * - P7
     - P7_0 to P7_7
   * - P8
     - P8_0 to P8_7
   * - P9
     - P9_0 to P9_7
   * - P10
     - P10_0 to P10_7
   * - P11
     - P11_0 to P11_7
   * - P12
     - P12_0 to P12_7
   * - P13
     - P13_0 to P13_7
   * - P14
     - P14_0 to P14_7
   * - P15 (LPGPIO)
     - P15_0 to P15_7

.. note::

      **LPGPIO Port**: P15 supports only LPGPIO functions (P15_0 to P15_7).

      **Pinmux Encoding**: Bits 0-2 denote the pin function (0 to 7), bits 3-9 denote the port number. The ``PINMUX_PIN`` macro combines these.


CAN Pin Setup
--------------

**Pin Configuration (CAN)**

.. list-table:: CAN Pins
   :widths: 25 25 25 25
   :header-rows: 1
   :align: left

   * - Instance
     - RXD
     - TXD
     - STBY
   * - CAN0
     - P7_0 (A), P0_4 (B), P12_4 (C)
     - P7_1 (A), P0_5 (B), P12_5 (C)
     - P7_3 (A), P0_6 (B), P12_6 (C)

**Connector Mappings (CAN)**

.. list-table:: CAN Connector Mappings
   :widths: 25 25 25 25
   :header-rows: 1
   :align: left

   * - Instance
     - Tx
     - Rx
     - Standby
   * - CAN0
     - J12-12
     - J12-14
     - J15-5

.. note::
   No pin connections or transceiver are required for the loopback test.


**Testing Connections (CAN)**

.. list-table:: CMP Testing Connections
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - Connection
     - Pins
   * - Positive Input
     - P12_3 to P0_0
   * - Output Observation
     - P14_7 (Logic Analyzer)

DAC and I2C Pin Setup
-----------------------

**Pin Configuration (DAC)**

.. list-table:: DAC Output Configuration
   :widths: 33 33 34
   :header-rows: 1
   :align: left

   * - Board Variant
     - DAC0 Pin
     - DAC1 Pin
   * - Ensemble E7
     - P2_2
     - P2_3
   * - Ensemble E1C
     - P2_3
     - -

**Hardware Setup (DAC)**

.. list-table:: DAC Hardware Setup
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - Channel
     - Setup
   * - DAC0
     - Connect logic analyzer to P2_2
   * - DAC1
     - Connect logic analyzer to P2_3

.. note::
   - **Input**: Provided to DAC registers.
   - **Resolution**: 12-bit, accepting values up to 4095.
   - **Operation**: Input of 0 gives 0 output; increments by 1000 are observable; inputs exceeding 4095 reset to 0.

**Pin Configuration (I2C)**

.. list-table:: I2C Pins
   :widths: 25 25 25 25
   :header-rows: 1
   :align: left

   * - Instance
     - SDA
     - SCL
     - Variant
   * - I2C0
     - P0_2 (A), P3_5 (B), P7_0 (C), P10_4 (D)
     - P0_3 (A), P3_4 (B), P7_1 (C), P10_5 (D)
     - Standard
   * - I2C1
     - P0_4 (A), P3_6 (B), P7_2 (C), P10_6 (D)
     - P0_5 (A), P3_7 (B), P7_3 (C), P10_7 (D)
     - Standard
   * - I2C2
     - P0_7 (A), P5_0 (B), P5_7 (C)
     - P0_6 (A), P5_1 (B), P5_6 (C)
     - Standard
   * - I2C3
     - P1_0 (A), P9_6 (B), P9_4 (C)
     - P1_1 (A), P9_7 (B), P9_5 (C)
     - Standard
   * - LPI2C
     - P7_5 (A), P5_3 (B)
     - P7_4 (A), P5_2 (B)
     - Low-Power
   * - I3C
     - P0_0 (A), P1_2 (B), P3_2 (C), P7_6 (D)
     - P0_1 (A), P1_3 (B), P3_3 (C), P7_7 (D)
     - Standard

**Hardware Connections (I2C)**

.. list-table:: I2C Hardware Connections
   :widths: 25 25 25 25
   :header-rows: 1
   :align: left

   * - Signal
     - I2C1 Pin
     - I2C0 Pin
     - Connection
   * - SDA
     - P7_2 (J15-3)
     - P3_5 (J11-29)
     - J15-3 to J11-29
   * - SCL
     - P7_3 (J15-5)
     - P3_4 (J11-27)
     - J15-5 to J11-27

**LPI2C Connections**

.. list-table:: LPI2C Connections
   :widths: 25 25 25 25
   :header-rows: 1
   :align: left

   * - Signal
     - I2C0 Pin
     - LPI2C Pin
     - Connection
   * - SDA
     - P3_5 (J11-29)
     - P5_3 (J14_5)
     - J11-29 to J14_5
   * - SCL
     - P3_4 (J11-27)
     - P5_2 (J12_17)
     - J11-27 to J12_17

**I3C Connector Mappings**

.. list-table:: I3C Pin Connections
   :widths: 33 33 34
   :header-rows: 1
   :align: left

   * - Instance
     - SDA
     - SCL
   * - I3C-0
     - J15-8
     - J15-10

I2S and LPI2S Pin Setup
-----------------------

I2S interfaces include I2S0 to I2S3 and LPI2S.

Pin Configuration
~~~~~~~~~~~~~~~~~

.. list-table:: I2S and LPI2S Pins
   :widths: 16 16 16 16 16 20
   :header-rows: 1
   :align: left

   * - Instance
     - SDI
     - SDO
     - SCLK
     - WS
     - Variant
   * - I2S0
     - P1_6 (A), P4_1 (B)
     - P1_7 (A), P4_2 (B)
     - P3_0 (A), P4_3 (B)
     - P3_1 (A), P4_4 (B)
     - Standard
   * - I2S1
     - P3_2 (A), P12_0 (B)
     - P3_3 (A), P12_1 (B)
     - P3_4 (A), P12_2 (B)
     - P4_0 (A), P12_3 (B)
     - Standard
   * - I2S2
     - P8_1 (A), P10_5 (B)
     - P8_2 (A), P10_6 (B)
     - P8_3 (A), P10_7 (B)
     - P8_4 (A), P11_0 (B)
     - Standard
   * - I2S3
     - P9_2 (A), P9_0 (B)
     - P9_3 (A), P9_1 (B)
     - P9_4 (A), P8_6 (B)
     - P9_5 (A), P8_7 (B)
     - Standard
   * - LPI2S
     - P2_4 (A), P10_1 (B), P13_4 (C)
     - P2_5 (A), P10_2 (B), P13_5 (C)
     - P2_6 (A), P10_3 (B), P13_6 (C)
     - P2_7 (A), P10_4 (B), P13_7 (C)
     - Low-Power

Hardware Connections
~~~~~~~~~~~~~~~~~~~~

.. list-table:: I2S and LPI2S Pin Connections
   :widths: 14 14 14 14 14 14 16
   :header-rows: 1
   :align: left

   * - Interface
     - WS
     - Clock
     - SDO
     - VA
     - VB
     - GND
   * - LPI2S
     - J14_40
     - J14_38
     - J14_39
     - 1.8
     - 3.3
     - GND
   * - I2S
     - J11_32
     - J11_27
     - J11_25
     - 1.8
     - 3.3
     - GND

.. note::
   The Ensemble DevKit includes two I2S microphones for stereo audio but lacks a headphone jack or speaker. Connect an external speaker using a 3.3V level shifter circuit.

PDM and LPPDM Pin Setup
-----------------------

PDM and LPPDM interfaces support data and clock signals.

Pin Configuration
~~~~~~~~~~~~~~~~~

.. list-table:: PDM and LPPDM Pins
   :widths: 25 25 25 25
   :header-rows: 1
   :align: left

   * - Instance
     - Data
     - Clock
     - Variant
   * - PDM
     - P0_4 (A), P0_6 (A), P5_0 (A), P5_1 (A), P6_0 (C), P6_2 (C), P3_0 (B), P3_2 (B), P5_4 (B), P5_5 (B)
     - P0_5 (A), P0_7 (A), P5_2 (A), P6_1 (C), P6_3 (C), P6_7 (A), P11_4 (B), P11_5 (B), P3_1 (B), P3_3 (B)
     - Standard
   * - LPPDM
     - P2_0 (A), P2_2 (A), P7_5 (A), P7_7 (A), P3_5 (B), P3_7 (B), P11_6 (B), P11_7 (B)
     - P2_1 (A), P2_3 (A), P3_4 (B), P7_4 (A), P7_6 (A), P11_2 (B), P11_3 (B)
     - Low-Power

SPI Pin Setup
-------------

SPI interfaces include SPI0 to SPI3 and LPSPI.

Pin Configuration
~~~~~~~~~~~~~~~~~

.. list-table:: SPI Pins
   :widths: 20 20 20 20 20
   :header-rows: 1
   :align: left

   * - Instance
     - MISO
     - MOSI
     - SCLK
     - SS
   * - SPI0
     - P1_0 (A), P5_0 (B), P7_0 (C)
     - P1_1 (A), P5_1 (B), P7_1 (C)
     - P1_2 (A), P5_3 (B), P7_2 (C)
     - P1_3 (A), P5_2 (B), P7_3 (C), P1_4 (A), P1_5 (A), P5_4 (A), P8_2 (B)
   * - SPI1
     - P2_4 (A), P8_3 (B), P14_4 (C)
     - P2_5 (A), P8_4 (B), P14_5 (C)
     - P2_6 (A), P8_5 (B), P14_6 (C)
     - P2_7 (A), P14_7 (C), P3_7 (A), P4_0 (A), P4_1 (A), P4_6 (A), P6_4 (B), P6_5 (B), P6_6 (B), P6_7 (B)
   * - SPI2
     - P4_2 (A), P9_2 (B)
     - P4_3 (A), P9_3 (B)
     - P4_4 (A), P9_4 (B)
     - P4_5 (A), P9_5 (B), P13_3 (A), P4_6 (A), P4_7 (A), P10_0 (B), P9_6 (B), P9_7 (B)
   * - SPI3
     - P12_4 (A), P10_5 (B)
     - P12_5 (A), P10_6 (B)
     - P12_6 (A), P10_7 (B)
     - P12_7 (A), P13_0 (A), P13_1 (A), P13_2 (A), P11_0 (B), P11_1 (B), P11_2 (B), P11_3 (B)
   * - LPSPI
     - P7_4 (A), P11_4 (B)
     - P7_5 (A), P11_5 (B)
     - P7_6 (A), P11_6 (B)
     - P7_7 (A), P11_7 (B)

LPSPI and SPI0 Demo Connections
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For data transmission between LPSPI (master) and SPI0 (slave) with DMA, connect using jumper wires:

.. list-table:: LPSPI and SPI0 Pin Connections
   :widths: 25 25 25 25
   :header-rows: 1
   :align: left

   * - Signal
     - LPSPI Pin
     - SPI0 Pin
     - Connection
   * - MISO
     - P7_4
     - P5_0
     - P7_4 (J12-27) to P5_0 (J12-13)
   * - MOSI
     - P7_5
     - P5_1
     - P7_5 (J15-9) to P5_1 (J12-15)
   * - SCLK
     - P7_6
     - P5_3
     - P7_6 (J15-8) to P5_3 (J14_5)
   * - SS
     - P7_7
     - P5_2
     - P7_7 (J15-10) to P5_2 (J12-17)

SPI0 and SPI1 Demo Connections
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For data transmission between SPI0 (master) and SPI1 (slave), connect using jumper wires:

.. list-table:: SPI0 and SPI1 Pin Connections
   :widths: 20 20 20 20 20
   :header-rows: 1
   :align: left

   * - Signal
     - SPI0 Pin
     - SPI0 Pin Header
     - SPI1 Pin
     - SPI1 Pin Header
   * - MISO
     - P5_0
     - J12-13
     - P8_3
     - J14-15
   * - MOSI
     - P5_1
     - J12-15
     - P8_4
     - J14-17
   * - SCLK
     - P5_3
     - J14_5
     - P8_5
     - J14-19
   * - SS
     - P5_2
     - J12-17
     - P6_4
     - J12-22


SD Pin Setup
------------

SD interfaces support data, command, clock, and reset signals.

.. list-table:: SD Pins
   :widths: 20 20 20 20 20
   :header-rows: 1
   :align: left

   * - Instance
     - Data (D0-D7)
     - CMD
     - CLK
     - RST
   * - SD
     - P5_0 to P5_7 (A), P6_0 to P6_7 (D), P8_0 to P8_7 (C), P13_0 to P13_7 (B)
     - P7_0 (A), P9_0 (C), P14_0 (B)
     - P7_1 (A), P9_1 (C), P14_1 (B)
     - P7_2 (A), P9_2 (C), P14_2 (B)
   * - SD (Additional)
     - P4_1 (D), P4_2 (D)
     - -
     - -
     - P4_3 (D)

Analog Pin Setup
----------------

Analog signals (ANA_S0 to ANA_S23) are supported on specific pins.

.. list-table:: Analog Pins
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - Signal
     - Pin
   * - ANA_S0
     - P0_0
   * - ANA_S1
     - P0_1
   * - ANA_S2
     - P0_2
   * - ANA_S3
     - P0_3
   * - ANA_S4
     - P0_4
   * - ANA_S5
     - P0_5
   * - ANA_S6
     - P0_6
   * - ANA_S7
     - P0_7
   * - ANA_S8
     - P1_0
   * - ANA_S9
     - P1_1
   * - ANA_S10
     - P1_2
   * - ANA_S11
     - P1_3
   * - ANA_S12
     - P1_4
   * - ANA_S13
     - P1_5
   * - ANA_S14
     - P1_6
   * - ANA_S15
     - P1_7
   * - ANA_S16
     - P2_0
   * - ANA_S17
     - P2_1
   * - ANA_S18
     - P2_2
   * - ANA_S19
     - P2_3
   * - ANA_S20
     - P2_4
   * - ANA_S21
     - P2_5
   * - ANA_S22
     - P2_6
   * - ANA_S23
     - P2_7

QEC Pin Setup
-------------

Quadrature Encoder (QEC0 to QEC3) interfaces support X, Y, and Z signals.

.. list-table:: QEC Pins
   :widths: 25 25 25 25
   :header-rows: 1
   :align: left

   * - Instance
     - X
     - Y
     - Z
   * - QEC0
     - P3_0 (A), P8_4 (B), P13_0 (C)
     - P3_1 (A), P8_5 (B), P13_1 (C)
     - P3_2 (A), P8_6 (B), P13_2 (C)
   * - QEC1
     - P3_3 (A), P8_7 (B), P13_3 (C)
     - P3_4 (A), P9_0 (B), P13_4 (C)
     - P3_5 (A), P9_1 (B), P13_5 (C)
   * - QEC2
     - P3_6 (A), P9_2 (B), P13_6 (C)
     - P3_7 (A), P9_3 (B), P13_7 (C)
     - P4_0 (A), P9_4 (B), P14_0 (C)
   * - QEC3
     - P4_1 (A), P9_5 (B), P14_1 (C)
     - P4_2 (A), P9_6 (B), P14_2 (C)
     - P4_3 (A), P9_7 (B), P14_3 (C)

QDEC Pin Setup
--------------

Quadrature Decoder (QDEC) testing uses the Zephyr QDEC sample application emulator.

Hardware Connections
~~~~~~~~~~~~~~~~~~~~

.. list-table:: QDEC Jumper Connections
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - Board Variant
     - Jumper Connections
   * - E7 DevKit
     - Phase A: J11_3 to J11_2, Phase B: J11_5 to J11_4
   * - E1C DevKit
     - Phase A: J10_13 to J9_7, Phase B: J10_15 to J9_9

.. note::
   - Ensure the encoder's ground is connected to the DevKit's ground.
   - Verify the encoder's supply voltage matches the DevKit's I/O voltage (typically 3.3V).
   - Refer to the Alif SDK documentation for exact pinmux mappings to UTIMER channels.

Fault Pin Setup
---------------

Fault signals (FAULT0 to FAULT3) are supported on specific pins.

.. list-table:: Fault Pins
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - Signal
     - Pin
   * - FAULT0
     - P4_4 (A), P8_0 (B), P14_4 (C)
   * - FAULT1
     - P4_5 (A), P8_1 (B), P14_5 (C)
   * - FAULT2
     - P4_6 (A), P8_2 (B), P14_6 (C)
   * - FAULT3
     - P4_7 (A), P8_3 (B), P14_7 (C)

JTAG Pin Setup
--------------

JTAG interfaces (JTAG0 and JTAG1) support trace, clock, and data signals.

.. list-table:: JTAG Pins
   :widths: 20 20 20 20 20
   :header-rows: 1
   :align: left

   * - Instance
     - TCK
     - TMS
     - TDI
     - TDO
   * - JTAG0
     - P4_4
     - P4_5
     - P4_6
     - P4_7
   * - JTAG1
     - P8_5
     - P8_6
     - P8_7
     - P9_0
   * - JTAG0 (Trace)
     - P3_7 (TRACECLK), P4_0 (TDATA0), P4_1 (TDATA1), P4_2 (TDATA2), P4_3 (TDATA3)
     - -
     - -
     - -


SCP Pin Setup
-------------

SCP signals (SCP0 to SCP3) are supported on specific pins.

.. list-table:: SCP Pins
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - Signal
     - Pin
   * - SCP0
     - P7_0 (A), P7_4 (C)
   * - SCP1
     - P7_1 (A), P7_5 (C)
   * - SCP2
     - P7_2 (A), P7_6 (C)
   * - SCP3
     - P7_3 (A), P7_7 (C)

Utility Timer (UT) Pin Setup
----------------------------

Utility Timer signals (UT0 to UT11) support T0 and T1 signals.

.. list-table:: Utility Timer Pins
   :widths: 33 33 34
   :header-rows: 1
   :align: left

   * - Instance
     - T0
     - T1
   * - UT0
     - P0_0 (A), P5_0 (B), P10_0 (C)
     - P0_1 (A), P5_1 (B), P10_1 (C)
   * - UT1
     - P0_2 (A), P5_2 (B), P10_2 (C)
     - P0_3 (A), P5_3 (B), P10_3 (C)
   * - UT2
     - P0_4 (A), P5_4 (B), P10_4 (C)
     - P0_5 (A), P5_5 (B biens), P10_5 (C)
   * - UT3
     - P0_6 (A), P5_6 (B), P10_6 (C)
     - P0_7 (A), P5_7 (B), P10_7 (C)
   * - UT4
     - P1_0 (A), P6_0 (B), P11_0 (C)
     - P1_1 (A), P6_1 (B), P11_1 (C)
   * - UT5
     - P1_2 (A), P6_2 (B), P11_2 (C)
     - P1_3 (A), P6_3 (B), P11_3 (C)
   * - UT6
     - P1_4 (A), P6_4 (B), P11_4 (C)
     - P1_5 (A), P6_5 (B), P11_5 (C)
   * - UT7
     - P1_6 (A), P6_6 (B), P11_6 (C)
     - P1_7 (A), P6_7 (B), P11_7 (C)
   * - UT8
     - P2_0 (A), P7_0 (B), P12_0 (C)
     - P2_1 (A), P7_1 (B), P12_1 (C)
   * - UT9
     - P2_2 (A), P7_2 (B), P12_2 (C)
     - P2_3 (A), P7_3 (B), P12_3 (C)
   * - UT10
     - P2_4 (A), P7_4 (B), P12_4 (C)
     - P2_5 (A), P7_5 (B), P12_5 (C)
   * - UT11
     - P2_6 (A), P7_6 (B), P12_6 (C)
     - P2_7 (A), P7_7 (B), P12_7 (C)

Debug and Miscellaneous Pin Setup
---------------------------------

Debug and miscellaneous signals include debug ports, clock outputs, and audio clocks.

.. list-table:: Debug and Miscellaneous Pins
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - Signal
     - Pin
   * - DEBUG_PORT0
     - P12_0
   * - DEBUG_PORT1
     - P12_1
   * - DEBUG_PORT2
     - P12_2
   * - DEBUG_PORT3
     - P12_3
   * - DEBUG_PORT4
     - P12_4
   * - DEBUG_PORT5
     - P12_5
   * - DEBUG_PORT6
     - P12_6
   * - DEBUG_PORT7
     - P12_7
   * - HFXO_OUT
     - P3_6 (A), P9_3 (B)
   * - AUDIO_CLK
     - P8_0 (A), P9_6 (B), P12_0 (C)