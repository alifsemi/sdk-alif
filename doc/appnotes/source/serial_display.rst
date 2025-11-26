==============
Serial Display
==============

Introduction
============

This application note describes how to flush continuous video frames to a serial display using the CDC200 device and an LCD panel. The ILI9806E panel is used for the demo application. The components in the display pipeline for Alif SoCs are:

1. CDC200 DPI-Display Controller
2. MIPI DSI Controller (with integrated DPLL/DPHY)
3. Display Panel

.. figure:: _static/mipi_dsi_block_diagram.png
   :alt: MIPI DSI Block Diagram
   :align: center

   MIPI DSI Block Diagram

The MIPI Display Serial Interface (DSI) facilitates communication and data transfer from a processor to a DSI-compliant display panel. The interface is based on a DSI host controller, which implements the protocol functions defined in the MIPI DSI specification, while a D-PHY module acts as the physical layer. The Configurable DPI controller (CDC200) within the processor generates a video stream for the DSI controller to use. The CDC200 IP reads images (layers) from memory, combines them on the fly (e.g., by blending, cropping, and windowing operations), and generates a video output stream of the combined image. The CDC200 has two output interfaces:

1. Parallel Display Interface contains VSync, HSync, Pixel clk, DE signals, 24 bits for RGB channels, and GND.
2. Serial Display Interface connects internally to the MIPI DSI controller IP module to convert the RGB signals.

.. figure:: _static/mipi_dsi_flow_diagram.png
   :alt: MIPI DSI Flow Diagram
   :align: center

   MIPI DSI Flow Diagram

The Display Panel can be either parallel or serial. The serial display panel connects to the MIPI DSI controller IP, which is in turn connected to the CDC200.

.. figure:: _static/dpi_internal_connection.png
   :alt: DPI Internal Connection
   :align: center

   DPI Internal Connection

Required Software Drivers
=========================

- MIPI DSI controller driver
- Display controller driver (CDC200)
- Panel Driver (ILI9806E)

CDC200 Blending Pipeline
========================

The CDC200 has two layers and a background layer color that are different parameters that can be controlled in a display system.

.. figure:: _static/cdc200_blending_pipeline.png
   :alt: CDC200 Blending Pipeline
   :align: center

   CDC200 Blending Pipeline

The CDC (Color Display Controller) includes blending logic that combines the color values (RGB) of one layer with the blended results of sub-adjacent layers. Here's how it works:

1. **Blending Process**
   - To calculate a blended pixel, as many steps as there are layers are needed.
   - Internally, this process is achieved through the Alpha Blending Pipeline (ABP).
   - The ABP delivers one pixel per clock cycle.
   - The length of the ABP corresponds to the number of blending steps required (i.e., the number of layers).

2. **Layer Blending Order**
   - If two layers are enabled:

     - First, layer 1 is blended with the background (BG) color.

     - Then, layer 2 is blended with the result of the blended color from layer 1 and the background.

   .. math::
      C' = f_1 \cdot c + f_2 \cdot c_s

   - :math:`C'` = blended color
   - :math:`f_1` = blend factor for current layer pixels
   - :math:`c` = current layer color
   - :math:`f_2` = blend factor for sub-adjacent color
   - :math:`c_s` = sub-adjacent layer color

   .. list-table:: Possible Values of :math:`f_1`
      :widths: 20 50
      :header-rows: 1

      * - Value
        - Significance
      * - 4
        - Constant-:math:`\alpha`
      * - 6
        - Pixel-:math:`\alpha` * Constant-:math:`\alpha`

   .. list-table:: Possible Values of :math:`f_2`
      :widths: 20 50
      :header-rows: 1

      * - Value
        - Significance
      * - 5
        - 1 - Constant-:math:`\alpha`
      * - 6
        - 1 - (Pixel-:math:`\alpha` * Constant-:math:`\alpha`)

   Constant-:math:`\alpha` for each layer can be configured using the ``CDC_Ln_CONST_ALPHA`` register or the ``const-alpha-l1/2`` parameter in the driver. The pixel alpha (Pixel-:math:`\alpha`) is determined by the pixel format used for the layer. In ARGB formats, there is a per-pixel alpha value provided, whereas in non-ARGB formats, the per-pixel values are elaborated to ARGB values, with an:math:`\alpha` value of 0 per pixel.

   Alpha (:math:`\alpha`) represents the opacity of a layer. The alpha value for a layer can range from 0 to 255 (0xff). For example, in the blending scenario described above:

   1. Consider the following settings:
      - Global background color (bg-color): 0x00ff00 (Green)
      - Layer 2: Disabled
      - Layer 1: Disabled
      - Default layer color for layer 1 (def-layer-color): 0xffff0000 (Red)
      - Blending factors for layer 1 (f1, f2): (4, 5)
      - Blending factors for layer 2 (f1, f2): (6, 7)

   2. If the constant alpha (const(:math:`\alpha`) for layer 1 is set to 0xff (255), only the color from the current layer will be used during blending. Any alpha value associated with the pixel will be ignored. In this case, the resulting color will be 0xffff0000 (Red).

   3. Similarly, if the constant alpha for the background layer (const-:math:`\alpha`) is set to 0xff (255), only the color from the background layer will be used during blending. The resulting color will be 0x00ff00 (Green).

.. figure:: _static/blending_process.png
   :alt: CDC200 Blending Process
   :align: center

   CDC200 Blending Process

Hardware Requirements
=====================

- Alif Devkit

CDC200 Controller
-----------------

The Customizable Display Controller (CDC) from TES is a fully configurable VHDL IP to drive pixel displays. It supports the following main features:

- Configurable resolution and refresh rate
- 2 display layers
- Programmable background color
- Color Look-up Table (CLUT) with 256 x 24 entries per layer for indexed pixel formats
- Flexible blending between the layers using alpha value (const alpha or Pixel alpha)
- Color Keying: defining transparent color for pixel formats without alpha channel
- Windowing: blending a programmable rectangular area of one layer into the other
- Default Color Programmable: per layer default color that is to be used in case of windowing or disabled layer
- Gamma Correction: map incoming RGB to different RGB values
- Dithering (2 bits per color Component): providing softer color transitions for displays with less color depth
- Multiple input pixel formats selectable per layer:
  - ARGB8888, RGBA8888, RGB888, RGB565, ARGB1555, ARGB4444
  - AL44, L8
- RGB888 output pixel format

Focus LCDs Panel Specifications
-------------------------------

Two example displays have been brought up on Alif Development Kit hardware. There is a 5" non-touch panel and a 4.3" capacitive touch panel included with the development kit depending on availability. They share the ILI9806E TFT controller, for which a driver is provided. To support additional displays using this same controller, simply change the resolution, timing values, and register sequence in the driver example.

.. note::
   For serial display: 2-lane display ILI9806E support is available only on the Alif Ensemble E7 DevKit.

.. list-table:: Key Features
   :widths: 30 30 30
   :header-rows: 1

   * - Key Features
     - E50RA-I-MW550-N
     - E43GB-I-MW405-C
   * - Diagonal Size
     - 5.0" (127 mm)
     - 4.3" (109 mm)
   * - Display Resolution
     - 480x854
     - 480x800
   * - Display Colors
     - 24-bit RGB
     - 24-bit RGB
   * - Display Brightness
     - 550 nits
     - 405 nits
   * - Display Controller
     - ILI9806E
     - ILI9806E
   * - Display Interface
     - 2-lane DSI
     - 2-lane DSI
   * - Touch Controller
     - N/A
     - GT911
   * - Touch Interface
     - N/A
     - I2C

.. list-table:: Timing Specifications
   :widths: 30 30 30
   :header-rows: 1

   * - Feature
     - E50RA-I-MW550-N
     - E43GB-I-MW405-C
   * - FPS
     - 60
     - 60
   * - Width
     - 480
     - 480
   * - Height
     - 854
     - 800
   * - HSYNC
     - 4
     - 4
   * - HBP
     - 30
     - 5
   * - HFP
     - 18
     - 5
   * - VSYNC
     - 4
     - 2
   * - VBP
     - 30
     - 10
   * - VFP
     - 20
     - 10

MIPI DSI
========

The MIPI Display Serial Interface (DSI) supports the following features:

- Supports one data lane (maximum speed 850 Mbps) or two data lanes (maximum speed 500 Mbps)
- Supports DSI version 1.02.00
- Supports D-PHY version 1.00.00
- Supports DCS version 1.02.00
- MIPI-DPI (Display Pixel Interface) interface:
  - 16 bit/pixel (R: 5-bit, G: 6-bit, B: 5-bit)
  - 18 bit/pixel (R: 6-bit, G: 6-bit, B: 6-bit)
  - 24 bit/pixel (R: 8-bit, G: 8-bit, B: 8-bit)
- 3-line 9-bit SPI (Serial Peripheral Interface) interface for touch sensor
- 2 TX data lanes on D-PHY
- Up to 2.5 Gbps per lane in D-PHY
- Supports ULPS (Ultra Low Power State) with PLL disabled
- Bi-directional communication and escape mode support through Data Lane 0
- ECC and Checksum capabilities
- Supports End of Transmission Packet (EoTp)
- Fault recovery schemes
- DPI interface features:
  - DPI interface color coding mappings into 30-bit interface:


    - 16-bit RGB, configuration 2

    - 18-bit RGB, configuration 2

    - 24-bit RGB

  - Programmable polarity for all DPI interface signals

- Video Mode Pattern Generator with the following capabilities:
  - Vertical/Horizontal color bar generation without DPI stimuli
  - PHY Bit-Error Rate (BER) pattern without DPI stimuli

Hardware Connection & Setup
===========================

The Alif DevKit connects to the ILI9806E panel via the MIPI DSI interface. The MIPI DSI controller on the DevKit Board interfaces with the ILI9806E panel using differential signaling pairs (D0P/N, D1P/N, CLKP/N) for high-speed data transmission. No external jumper wires are required, as the connections are internal to the board's MIPI DSI connector.

.. figure:: _static/ILI9806E.png
   :alt: ILI9806E Panel
   :align: center

   ILI9806E Panel

.. figure:: _static/serial_display_setup.png
   :alt: Hardware Setup
   :align: center

   Hardware Setup

Build CDC200 Application in Zephyr
====================================

Follow these steps to build CDC200 application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.


2. Build commands for HE application for TCM memory:

   .. code-block:: bash

      west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/display/ -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/display/boards/serial_display_2lane.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/display/boards/serial_display.conf"

3. Build commands for HP application for TCM memory:

   .. code-block:: bash

      west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/display/ -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/display/boards/serial_display_2lane.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/display/boards/serial_display.conf"


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Required Config Features
========================

The following config features are necessary to test the application:

- ``CONFIG_HEAP_MEM_POOL_SIZE=81920``
- ``CONFIG_LOG=y``
- ``CONFIG_DISPLAY=y``
- ``CONFIG_MIPI_DSI=y``
- ``CONFIG_DISPLAY_LOG_LEVEL_DBG=y`` (to enable display driver debug logs)

These config features are already selected when building the test application.

DTS Properties
==============

CDC200 DTS Properties
---------------------

The DTS entry for the CDC200 in Zephyr has the following tweakable properties that allow testing various features:

.. list-table:: CDC200 DTS Properties
   :widths: 20 50 20
   :header-rows: 1

   * - DTS Property
     - Significance
     - Default Value
   * - width
     - Width of Panel in pixels (for FW-405 panel)
     - 480
   * - height
     - Height of Panel in pixels (for FW-405 panel)
     - 800
   * - hfront-porch
     - Horizontal Front Porch time in pixel clocks (for FW-405 panel)
     - 5
   * - hback-porch
     - Horizontal Back Porch time in pixel clocks (for FW-405 panel)
     - 5
   * - hsync-len
     - Length of horizontal sync pulse in pixel clock (for FW-405 panel)
     - 4
   * - vfront-porch
     - Vertical Front Porch time in pixel clocks (for FW-405 panel)
     - 10
   * - vback-porch
     - Vertical Back Porch time in pixel clocks (for FW-405 panel)
     - 10
   * - vsync-len
     - Length of vertical sync pulse in number of lines (for FW-405 panel)
     - 2
   * - hsync-active
     - Polarity of H-Sync Pulse. 0 - Active low, 1 - Active high
     - 0
   * - vsync-active
     - Polarity of V-Sync Pulse. 0 - Active low, 1 - Active high
     - 0
   * - de-active
     - Polarity of Data Enable (DE) signal. 0 - Active low, 1 - Active high
     - 1
   * - pixelclk-active
     - Polarity of H-Sync Pulse. 0 - Active low, 1 - Active high
     - 0
   * - bg-color
     - Background Layer color (24-bit in size)
     - 0
   * - enable-l1/l2
     - Enable Layer 1/2. 0 - Disable, 1 - Enable
     - 1 - l1, 0 - l2
   * - pixel-fmt-l1/l2
     - Pixel format for Layer 1/2. Possible values: "argb-8888" (Tested + supported by app), "rgb-888", "rgb-565" (Tested + supported by app), "rgba-8888", "al-44", "l-8", "argb-1555", "argb-4444"
     - "rgb-888" - l1, "undefined" - l2
   * - def-back-color-l1/l2
     - Default Color for layer 1/2 (32-bit value)
     - 0 - l1, No value specified for l2
   * - win-x0-l1/l2
     - Starting x value for layer 1/2. Its value should be between 0 to width (inclusive), but less than win-x1-l1/l2 respectively
     - 0 - l1, 0 - l2
   * - win-x1-l1/l2
     - Ending x value for layer 1/2. Its value should be between 0 to width (inclusive), but greater than win-x0-l1/l2 respectively
     - 480 - l1, 0 - l2
   * - win-y0-l1/l2
     - Starting y value of layer 1/2. Its value should be between 0 to height (inclusive), but less than win-y1-l1/l2 respectively
     - 0 - l1 and l2
   * - win-y1-l1/l2
     - Ending value of layer 1/2. Its value should be between 0 to height (inclusive), but greater than win-y0-l1/l2 respectively
     - 800 - l1, 0 - l2
   * - blend-factor1-l1/l2
     - Current layer blending factor - f1. When layer-wise blending is enabled, the pixel from current layer will be weighted based on this factor. Possible values: 4 - constant alpha used for weighting, if Pixel has alpha - it is ignored; 6 - (constant alpha * pixel alpha) used for weighing
     - 4 - l1, 6 - l2
   * - blend-factor2-l1/l2
     - Subjacent layer blending factor - f2. When layer-wise blending is enabled, this factor will determine how the cumulative pixels from lower layers are to be blended. Possible values: 5 - (1 - constant alpha) used for weight; 7 - (1 - (pixel_alpha * constant_alpha)) used for weight
     - 5 - l1, 7 - l2
   * - const-alpha-l1/l2
     - Value of constant alpha to be used for a given layer
     - 0xff - l1, 0xff - l2

MIPI-DSI DTS Properties
-----------------------

The DTS entry for MIPI-DSI has the following tweakable properties:

.. list-table:: MIPI-DSI DTS Properties
   :widths: 20 40 20 20
   :header-rows: 1

   * - DTS Property
     - Significance
     - Default Value
     - Notes
   * - eotp-lp-tx-en
     - Automatically insert the EoTp short packet at the end of LP transmission from the host to peripheral
     - False
     -
   * - eotp-rx-en
     - Enable reception of the EoTp short packet at the end of LPDT reception from the peripheral
     - False
     - FW-405 does not support transmission of EoTp short packets
   * - ecc-recv-en
     - Support for ECC reception, error correction, and reporting
     - True
     -
   * - crc-recv-en
     - Support for CRC reception and error reporting
     - True
     -
   * - frame-ack-en
     - Support Peripheral ACK at the end of frame
     - True
     -
   * - dpi-colorm-active
     - Polarity of ColorM pin. 0 - Active low, 1 - Active high
     - 1
     -
   * - dpi-shutdn-active
     - Polarity of Shut-down pin. 0 - Active low, 1 - Active high
     - 1
     -
   * - vid-pkt-size
     - Size of video packet in pixel stream
     - 480
     - Only useful in non-burst mode of operation
   * - dpi-video-pattern-gen
     - Use DSI internal Video packet generator for driving display. CDC200 output will be overlooked when this property is used. Different modes available: vertical-color-bar, horizontal-color-bar, vertical-bit-error-rate
     - No value provided
     -

FW-405 Panel DTS Properties
---------------------------

The DTS entry for the FW-405 serial panel has the following tweakable properties:

.. list-table:: FW-405 Panel DTS Properties
   :widths: 20 50 20
   :header-rows: 1

   * - DTS Property
     - Significance
     - Default Value
   * - video-mode
     - Selects the mode of operation of Panel and Host DSI controller. Supported Modes: burst, nb-sync-pulse (Non-Burst with sync pulses), nb-sync-events (Non-Burst with sync events)
     - burst
   * - command-tx-mode
     - DSI Commands to be transmitted to the panel in either of the following modes: low-power, high-speed
     - high-speed
   * - data-lanes
     - Number of data-lanes. For now, only 2 lanes are supported, but possible values are 1 and 2
     - 2
   * - pixel-format
     - Supported Pixel format for the DSI interface. MIPI_DSI_PIXFMT_RGB888, MIPI_DSI_PIXFMT_RGB666, MIPI_DSI_PIXFMT_RGB666_PACKED, MIPI_DSI_PIXFMT_RGB565
     - MIPI_DSI_PIXFMT_RGB888
   * - width
     - Width of Panel. This should be same as width property of CDC200
     - 480
   * - height
     - Height of Panel. This should be same as height property of CDC200
     - 800

Validating CDC200
=================

The output screen is configured with the following layer settings:

- Panel dimensions (width, height) = (480, 800)
- Layer 1 dimensions (x resolution, y resolution) = (480, 800)
- Pixel where Layer 1 starts (x, y) = (0, 0)
- Pixel where Layer 1 ends (x, y) = (480, 800)
- Layer 2 dimensions (x resolution, y resolution) = (0, 0)
- Pixel where Layer 2 starts (x, y) = (0, 0)
- Pixel where Layer 2 ends (x, y) = (0, 0)

Output Logs
-----------

The following are the output logs observed on minicom:

.. code-block:: console

   [00:00:00.244,000] <inf> panel_mw405: MW-405 Configuration.

   [00:00:00.326,000] <inf> disp: Rotating the display by 180 degrees
   [00:00:00.327,000] <inf> disp: Enable Ensemble-DSI Device video mode.
   [00:00:00.327,000] <inf> disp: Panel Orientation - 2
   [00:00:00.327,000] <inf> disp: Display sample for cdc200@49031000
   [00:00:00.327,000] <inf> disp: Enabling CDC200 Device.
   [00:00:00.327,000] <inf> disp: Display Capabilities
   [00:00:00.327,000] <inf> disp: Panel resolution, supported formats - (480, 800), 25
   [00:00:00.327,000] <inf> disp: CDC200 orientation - 0
   [00:00:00.327,000] <inf> disp: Display Capabilities layer 1:
   [00:00:00.327,000] <inf> disp:  layer_enabled - 1
   [00:00:00.327,000] <inf> disp:  (x_res, y_res) - (480, 800)
   [00:00:00.327,000] <inf> disp:  curr_pix_fmt - 1
   [00:00:00.327,000] <inf> disp: Display Capabilities layer 2:
   [00:00:00.327,000] <inf> disp:  layer_enabled - 0
   [00:00:00.327,000] <inf> disp:  (x_res, y_res) - (0, 0)
   [00:00:00.327,000] <inf> disp:  curr_pix_fmt - 0
   [00:00:00.327,000] <inf> disp: FB0 - 0x02000000, size - 1152000

.. figure:: _static/serial_display_output.png
   :alt: Serial Display Output
   :align: center

   Serial Display Output

Known Issues
============

- **Zephyr CDC200 Driver**: The Zephyr device driver for the CDC200 currently supports only ARGB8888, RGB888, and RGB565 formats. This is a limitation of the Zephyr framework and may be addressed in future releases.

- **Demo Application (Layer 2)**: In the demo application, Layer 2 is designed to copy an image in ARGB8888 format directly from a C array to the framebuffer. Therefore, avoid using any format other than ARGB8888 for Layer 2. Layer 1 formats can be changed without issue.