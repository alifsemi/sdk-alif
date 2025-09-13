.. _cdc200:

================
Parallel Display
================

Introduction
============

This application note describes how to flush continuous video frames to a parallel display using the CDC200 device and LCD panel. The ILI6122 panel is used for the demo application. The components in the display pipeline for Alif SoCs are:

1. CDC200 Driver
2. Display Panel Configuration
   - Interface: Parallel
   - Resolution: 800 x 480

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
        - :math:`\text{Constant-}\alpha`
      * - 6
        - :math:`\text{Pixel-}\alpha \cdot \text{Constant-}\alpha`

   .. list-table:: Possible Values of :math:`f_2`
      :widths: 20 50
      :header-rows: 1

      * - Value
        - Significance
      * - 5
        - :math:`1 - \text{Constant-}\alpha`
      * - 6
        - :math:`1 - (\text{Pixel-}\alpha \cdot \text{Constant-}\alpha)`

   :math:`\text{Constant-}\alpha` for each layer can be configured using the ``CDC_Ln_CONST_ALPHA`` register or the ``const-alpha-l1/2`` parameter in the driver. The pixel alpha (:math:`\text{Pixel-}\alpha`) is determined by the pixel format used for the layer. In ARGB formats, there is a per-pixel alpha value provided, whereas in non-ARGB formats, the per-pixel values are elaborated to ARGB values, with a :math:`\text{Constant-}\alpha` value of 0 per pixel.

   Alpha (:math:`\alpha`) represents the opacity of a layer. The alpha value for a layer can range from 0 to 255 (0xff). For example, in the blending scenario described above:

   1. Consider the following settings:

      - Global background color (bg-color): 0x00ff00 (Green)

      - Layer 2: Disabled

      - Layer 1: Disabled

      - Default layer color for layer 1 (def-layer-color): 0xffff0000 (Red)

      - Blending factors for layer 1 (f1, f2): (4, 5)

      - Blending factors for layer 2 (f1, f2): (6, 7)

   2. If the constant alpha (:math:`\text{Constant-}\alpha`) for layer 1 is set to 0xff (255), only the color from the current layer will be used during blending. Any alpha value associated with the pixel will be ignored. In this case, the resulting color will be 0xffff0000 (Red).

   3. Similarly, if the constant alpha for the background layer (:math:`\text{Constant-}\alpha`) is set to 0xff (255), only the color from the background layer will be used during blending. The resulting color will be 0x00ff00 (Green).

.. figure:: _static/blending_process.png
   :alt: CDC200 Blending Process
   :align: center

   CDC200 Blending Process

Hardware Requirements
=====================

The parallel display configuration with the CDC200 and ILI6122 panel is supported on the following Alif Ensemble DevKits:

- E7
- E1C
- B1DK

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

ILI6122 Panel
-------------

ILI6122 is a 1200-channel output source driver with TTL interface timing controller (TCON). The interface follows digital 24-bit parallel RGB input format. The TCON generates (800 x 480) and (800 x 600) resolutions and provides horizontal and vertical control timing to source driver and gate driver. It also supports dithering feature, applying source driver and 6-bit DAC to perform 8-bit resolution 256 gray scales. It can be configured as dual-gate operation mode for reducing FPC amount and saving the cost. It has a wide range of supply voltages and many pin control features.

.. figure:: _static/ILI6122.png
   :alt: ILI6122 Panel
   :align: center

   ILI6122 Panel

The following are the features available with ILI6122 Panel:

**TCON**
- Supports display resolution 800x480 and 800x600
- Supports digital 24-bit parallel RGB input mode
- Supports configuring CABC block via 3-line SPI mode
- Source output with 8-bit resolution for 256 gray scales (2-bit dithering)
- Supports dual-gate operation mode
- Supports Stripe CF configuration
- Maximum Operating frequency: 50 MHz
- Provides flip and mirror scan mode by pin-control
- Supports stand-by mode for saving power consumption
- Operating Voltage level 3.0 V to 3.6 V
- Hardware Pin control CABC Mode selection

**Source Driver**
- 1200 channels output source driver for TFT LCD panel
- Embedded custom-made Gamma table for special custom request
- Supports external V1~V14 pad for Gamma adjustment
- Output dynamic range: 0.1 ~ VDDA-0.1V
- Voltage deviation of outputs: :math:`\pm 20\,\text{mV}`

Hardware Setup
==============

.. figure:: _static/parallel_display_setup.png
   :alt: Hardware Setup
   :align: center

   Hardware Setup

Building the CDC200 Application
===============================

The application was tested for the HP core. Follow these steps to build your Zephyr-based CDC200 application using the GCC compiler and the Alif Zephyr SDK:

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

2. Remove the existing build directory and build the CDC200 application:

   .. code-block:: bash

      rm -rf build
      west build -b alif_e7_dk_rtss_hp ../alif/samples/drivers/display -p -- -G"Unix Makefiles"

3. Access the output and binary files: The binary file is available in the ``./build/zephyr/`` directory.

Required Config Features
========================

The following config features are necessary to test the application:

- ``CONFIG_HEAP_MEM_POOL_SIZE=81920``
- ``CONFIG_LOG=y``
- ``CONFIG_DISPLAY=y``
- ``CONFIG_DISPLAY_LOG_LEVEL_DBG=y`` (to enable display driver debug logs)

These config features are already selected when building the test application.

DTS Properties
==============

The DTS entry for the CDC200 in Zephyr has the following tweakable properties that allow testing various features:

.. list-table:: DTS Properties
   :widths: 20 50 20
   :header-rows: 1

   * - DTS Property
     - Significance
     - Default Value
   * - width
     - Width of Panel in pixels
     - 800
   * - height
     - Height of Panel in pixels
     - 480
   * - hfront-porch
     - Horizontal Front Porch time in pixel clocks
     - 210
   * - hback-porch
     - Horizontal Back Porch time in pixel clocks
     - 46
   * - hsync-len
     - Length of horizontal sync pulse in pixel clock
     - 1
   * - vfront-porch
     - Vertical Front Porch time in pixel clocks
     - 22
   * - vback-porch
     - Vertical Back Porch time in pixel clocks
     - 23
   * - vsync-len
     - Length of vertical sync pulse in number of lines
     - 1
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
     - 0x5a5a5a
   * - enable-l1/l2
     - Enable Layer 1/2. 0 - Disable, 1 - Enable
     - 1
   * - pixel-fmt-l1/l2
     - Pixel format for Layer 1/2. Possible values: "argb-8888" (Tested + supported by app), "rgb-888", "rgb-565" (Tested + supported by app), "rgba-8888", "al-44", "l-8", "argb-1555", "argb-4444"
     - "rgb-565" l1, "argb-8888" - l2
   * - def-back-color-l1/l2
     - Default Color for layer 1/2 (32-bit value)
     - 0x00ff00 - l1, No value specified for l2
   * - win-x0-l1/l2
     - Starting x value for layer 1/2. Its value should be between 0 to width (inclusive), but less than win-x1-l1/l2 respectively
     - 0 - l1, 500 - l2
   * - win-x1-l1/l2
     - Ending x value for layer 1/2. Its value should be between 0 to width (inclusive), but greater than win-x0-l1/l2 respectively
     - 300 - l1, 800 - l2
   * - win-y0-l1/l2
     - Starting y value of layer 1/2. Its value should be between 0 to height (inclusive), but less than win-y1-l1/l2 respectively
     - 0 - l1 and l2
   * - win-y1-l1/l2
     - Ending value of layer 1/2. Its value should be between 0 to height (inclusive), but greater than win-y0-l1/l2 respectively
     - 480 - l1, 68 - l2
   * - blend-factor1-l1/l2
     - Current layer blending factor - f1. When layer-wise blending is enabled, the pixel from current layer will be weighted based on this factor. Possible values: 4 - constant alpha used for weighting, if Pixel has alpha - it is ignored; 6 - (constant alpha * pixel alpha) used for weighing
     - 4 - l1 and l2
   * - blend-factor2-l1/l2
     - Subjacent layer blending factor - f2. When layer-wise blending is enabled, this factor will determine how the cumulative pixels from lower layers are to be blended. Possible values: 5 - (1 - constant alpha) used for weight; 7 - (1 - (pixel_alpha * constant_alpha)) used for weight
     - 5 - l1 and l2
   * - const-alpha-l1/l2
     - Value of constant alpha to be used for a given layer
     - 0x7f - l1, 0xaf - l2

Validating CDC200
=================

The output screen is divided among 2 layers and some area that is not covered by any layer.

- Panel dimensions (width, height) = (800, 480)
- Layer 1 dimensions (x resolution, y resolution) = (300, 480)
- Pixel where Layer 1 starts (x, y) = (0, 0)
- Pixel where Layer 1 ends (x, y) = (300, 480)
- Layer 2 dimensions (x resolution, y resolution) = (300, 480)
- Pixel where layer 2 starts (x, y) = (500, 0)
- Pixel where Layer 2 ends (x, y) = (800, 480)

Output Logs
-----------

Both Layers Enabled
~~~~~~~~~~~~~~~~~~~

The following are the output logs observed on minicom when both layers are enabled:

.. code-block:: console

   *** Booting Zephyr OS build zephyr-v3.3.0-86-ged667deb6ee9 ***
   [00:00:00.000,000] <inf> ensemble_disp: Display sample for cdc200@49031000
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities panel x_res - 800
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities panel y_res - 480
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities pix_fmt_supported - 25
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities orientation - 0
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities layer 1:
   [00:00:00.000,000] <inf> ensemble_disp:         layer_enabled - 1
   [00:00:00.000,000] <inf> ensemble_disp:         (x_res, y_res) - (300, 480)
   [00:00:00.000,000] <inf> ensemble_disp:         curr_pix_fmt - 16
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities layer 2:
   [00:00:00.000,000] <inf> ensemble_disp:         layer_enabled - 1
   [00:00:00.000,000] <inf> ensemble_disp:         (x_res, y_res) - (300, 68)
   [00:00:00.000,000] <inf> ensemble_disp:         curr_pix_fmt - 8

.. figure:: _static/output_both_layers.png
   :alt: Output with Both Layers Enabled
   :align: center

   Output with Both Layers Enabled

Only Layer 1
~~~~~~~~~~~~

.. code-block:: console

   *** Booting Zephyr OS build zephyr-v3.3.0-86-ged667deb6ee9 ***
   [00:00:00.000,000] <inf> ensemble_disp: Display sample for cdc200@49031000
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities panel x_res - 800
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities panel y_res - 480
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities pix_fmt_supported - 25
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities orientation - 0
   [00:00:00.000] <inf> ensemble_disp: Display Capabilities layer 1:
   [00:00:00.000,000] <inf> ensemble_disp:         layer_enabled - 1
   [00:00:00.000,000] <inf> ensemble_disp:         (x_res, y_res) - (300, 480)
   [00:00:00.000,000] <inf> ensemble_disp:         curr_pix_fmt - 16
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities layer 2:
   [00:00:00.000,000] <inf> ensemble_disp:         layer_enabled - 0
   [00:00:00.000,000] <inf> ensemble_disp:         (x_res, y_res) - (300, 68)
   [00:00:00.000,000] <inf> ensemble_disp:         curr_pix_fmt - 8

.. figure:: _static/output_layer1.png
   :alt: Output with Only Layer 1
   :align: center

   Output with Only Layer 1

Only Layer 2
~~~~~~~~~~~~

.. code-block:: console

   *** Booting Zephyr OS build zephyr-v3.3.0-86-ged667deb6ee9 ***
   [00:00:00.000,000] <inf> ensemble_disp: Display sample for cdc200@49031000
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities panel x_res - 800
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities panel y_res - 480
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities pix_fmt_supported - 25
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities orientation - 0
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities layer 1:
   [00:00:00.000,000] <inf> ensemble_disp:         layer_enabled - 0
   [00:00:00.000,000] <inf> ensemble_disp:         (x_res, y_res) - (300, 480)
   [00:00:00.000,000] <inf> ensemble_disp:         curr_pix_fmt - 16
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities layer 2:
   [00:00:00.000,000] <inf> ensemble_disp:         layer_enabled - 1
   [00:00:00.000,000] <inf> ensemble_disp:         (x_res, y_res) - (300, 68)
   [00:00:00.000,000] <inf> ensemble_disp:         curr_pix_fmt - 8

.. figure:: _static/output_layer2.png
   :alt: Output with Only Layer 2
   :align: center

   Output with Only Layer 2

No Layers Enabled
~~~~~~~~~~~~~~~~~

.. code-block:: console

   *** Booting Zephyr OS build zephyr-v3.3.0-86-ged667deb6ee9 ***
   [00:00:00.000,000] <inf> ensemble_disp: Display sample for cdc200@49031000
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities panel x_res - 800
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities panel y_res - 480
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities pix_fmt_supported - 25
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities orientation - 0
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities layer 1:
   [00:00:00.000,000] <inf> ensemble_disp:         layer_enabled - 0
   [00:00:00.000,000] <inf> ensemble_disp:         (x_res, y_res) - (300, 480)
   [00:00:00.000,000] <inf> ensemble_disp:         curr_pix_fmt - 16
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities layer 2:
   [00:00:00.000,000] <inf> ensemble_disp:         layer_enabled - 0
   [00:00:00.000,000] <inf> ensemble_disp:         (x_res, y_res) - (300, 68)
   [00:00:00.000,000] <inf> ensemble_disp:         curr_pix_fmt - 8

.. figure:: _static/output_no_layers.png
   :alt: Output with No Layers
   :align: center

   The Layer 1 default color (0x00ff00) blended with background layer color (0x5a5a5a) is observed.

No Layers and Default Color for Layer 1 Disabled
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: console

   *** Booting Zephyr OS build zephyr-v3.3.0-86-ged667deb6ee9 ***
   [00:00:00.000,000] <inf> ensemble_disp: Display sample for cdc200@49031000
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities panel x_res - 800
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities panel y_res - 480
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities pix_fmt_supported - 25
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities orientation - 0
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities layer 1:
   [00:00:00.000,000] <inf> ensemble_disp:         layer_enabled - 0
   [00:00:00.000,000] <inf> ensemble_disp:         (x_res, y_res) - (300, 480)
   [00:00:00.000,000] <inf> ensemble_disp:         curr_pix_fmt - 16
   [00:00:00.000,000] <inf> ensemble_disp: Display Capabilities layer 2:
   [00:00:00.000,000] <inf> ensemble_disp:         layer_enabled - 0
   [00:00:00.000,000] <inf> ensemble_disp:         (x_res, y_res) - (300, 68)
   [00:00:00.000,000] <inf> ensemble_disp:         curr_pix_fmt - 8

.. figure:: _static/output_no_layers_no_default.png
   :alt: Output with No Layers and Default Color for Layer 1 Disabled
   :align: center

   The Background Layer color 0x5a5a5a is observed when all layers and their default colors are disabled.

Known Issues
============

- **Zephyr CDC200 Driver**: The Zephyr device driver for the CDC200 currently supports only ARGB8888, RGB888, and RGB565 formats. This is a limitation of the Zephyr framework and may be addressed in future releases.
- **Demo Application (Layer 2)**: In the demo application, Layer 2 is designed to copy an image in ARGB8888 format directly from a C array to the framebuffer. Therefore, avoid using any format other than ARGB8888 for Layer 2. Layer 1 formats can be changed without issue. 