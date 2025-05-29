.. _appnote-zephyr-crc:

===
CRC
===

Introduction
============

This application note provides an overview of the Cyclic Redundancy Check (CRC), a powerful method used for error detection and correction. Here are the key features:

- **CRC Algorithm Support**: Supports CRC-8-CCITT, CRC-16-CCITT, CRC-32, and CRC-32C standards.
- **Data Processing**: Processes data in 8-bit or 32-bit chunks.
- **Automatic Byte and Bit Swapping**: Optionally performs byte and bit swapping of data.
- **Customized Polynomials**: Allows customization of polynomials for specific applications.
- **AHB Interface**: Interfaces with the Advanced High-performance Bus (AHB).

CRC Operation
=============

The following steps are involved in CRC operation:

Configuration
-------------

Write to the control register (0x00) to set up the operation:

- Choose the desired CRC algorithm and size (8, 16, or 32 bits).
- Optionally program BIT or BYTE swapping.
- Optionally program Reflect CRC.
- Provide a seed value for the selected algorithm.

Initialization
--------------

- Set the init[0] bit to load the CRC accumulation register with the seed value.
- Write the custom polynomial to the poly_custom register (0x14).

Calculation
-----------

The CRC output is independent of the input size. If the user selects the 8-bit CRC algorithm, the output will be 8 bits. For the 16-bit CRC algorithm, the output will be 16 bits, and for the 32-bit CRC algorithm, the output will be 32 bits.

Reading the Result
------------------

Once all data has been processed, read the CRC result from the crc_out register (0x18).

Hardware Requirements
=====================

- **Alif Ensemble DevKit (Flat Board)**
- **Debugger (ULinkpro or JLink)**

Building the CRC Application
============================

Follow these steps to create your Zephyr-based CRC application using the GCC compiler and the Alif Zephyr SDK:

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

2. Remove the existing build directory and build the application:

.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk_rtss_hp ../alif/samples/drivers/crc/

.. note::
   The build command targets the M55-HP core. To build for the M55-HE core, use ``alif_e7_dk_rtss_he`` instead (e.g., ``west build -b alif_e7_dk_rtss_he ../alif/samples/drivers/crc/``).

Verifying CRC Algorithm in a Web CRC Calculator
===============================================

You can verify that the CRC output in a web CRC calculator matches the devKit output. Refer to a web CRC calculator (e.g., https://crccalc.com/) for validation.

CRC-8-CCITT
-----------

To verify the output in a web CRC calculator:

1. Select CRC8 and enter the CRC input data.
2. Calculate the CRC output.

To verify the CRC-8-CCITT algorithm in the Alif CRC Hardware module:

1. Navigate to the ``ensemble_rtss_common.dtsi`` file and set ``crc_algo = "CRC_8_BIT_SIZE"``.
2. Provide the input data in the CRC application.

Example code:

.. code-block:: c

   uint8_t arr[] = {0x67, 0x3F, 0x90, 0xC9, 0x25, 0xF0, 0x4A, 0xB1, 0x12}; /* CRC8 unaligned input data */
   uint32_t seed_value = 0x00000000; /* Seed value for 8 bit */
   /* Output: CRC8 output value: 0xCD */


.. figure:: _static/web_crc8_output.png
    :alt: Web CRC 8 Output
    :align: center

    Web CRC 8 Output

.. figure:: _static/flatboard_crc8_output.png
    :alt: Flatboard CRC 8 Output
    :align: center

    Flatboard CRC 8 Output

CRC-16-CCITT
------------

To verify the output in a web CRC calculator:

1. Select CRC16_CCITT_ZERO and enter the CRC input data.
2. Calculate the CRC output.

To verify the CRC-16-CCITT algorithm in the Alif CRC Hardware module:

1. Navigate to the ``ensemble_rtss_common.dtsi`` file and set ``crc_algo = "CRC_16_BIT_SIZE"``.
2. Provide the input data in the CRC application.

Example code:

.. code-block:: c

   uint8_t arr[] = {0x67, 0x3F, 0x90, 0xC9, 0x25, 0xF0, 0x4A, 0xB1, 0x12}; /* CRC unaligned input data */
   uint32_t seed_value = 0x00000000; /* Seed value for 16 bit */
   /* Output: CRC-16-CCITT output value: 0xCD4D */

.. figure:: _static/web_crc16_output.png
    :alt: Web CRC 16 Output
    :align: center

    Web CRC 16 Output

.. figure:: _static/flatboard_crc16_output.png
    :alt: Flatboard CRC 16 Output
    :align: center

    Flatboard CRC 16 Output

CRC-32
------

To verify the output in a web CRC calculator:

1. Select CRC32 and enter the CRC input data.
2. Enable Input reflected and Result reflected.
3. Set the initial value to 0xFFFFFFFF and the final XOR value to 0xFFFFFFFF.
4. Calculate the CRC output.

To verify the CRC32 algorithm in the Alif CRC Hardware module:

1. Navigate to the ``ensemble_rtss_common.dtsi`` file and set ``crc_algo = "CRC_32_BIT_SIZE"``.
2. In the application code, enable the bit_swap, byte_swap, reflect, and invert options.
3. Provide the input data in the CRC application.

Example code:

.. code-block:: c

   uint8_t input_value[] = {0x67, 0x3F, 0x90, 0xC9, 0x25, 0xF0, 0x4A, 0xB1, 0x12}; /* CRC unaligned input data */
   uint32_t seed_value = 0xFFFFFFFF; /* Seed value for 32 bit */

.. figure:: _static/sample_code_crc32.png
    :alt: CRC 32 Sample Code
    :align: center

    CRC 32 Sample Code

.. figure:: _static/web_crc32_output.png
    :alt: Web CRC 16 Output
    :align: center

    Web CRC 32 Output

.. figure:: _static/flatboard_crc32_output.png
    :alt: Flatboard CRC 32 Output
    :align: center

    Flatboard CRC 32 Output

Custom CRC32C
-------------

To verify the output in a web CRC calculator:

1. Select the CRC32 custom option.
2. Enter your CRC input data.
3. Change the polynomial to match the application code.
4. Enable Input reflected.
5. Set the initial value to 0xFFFFFFFF and the final XOR value to 0xFFFFFFFF.
6. Calculate the CRC output.

To verify the CRC32C algorithm in the Alif CRC Hardware module:

1. Navigate to the ``ensemble_rtss_common.dtsi`` file and set ``crc_algo = "CRC_32_BIT_SIZE"``.
2. In the application code, enable the bit_swap, byte_swap, reflect, invert, and custom_poly options.
3. Set the polynomial value to 0x2CEEA6C8 and provide the input data in the CRC application.
4. Call the ``crc_polycustom`` function (declared in ``include/zephyr/drivers/crc/alif_crc.h``) in your application to use the CRC32C algorithm, passing the polynomial variable as a function parameter.

Example code:

.. code-block:: c

   uint8_t arr[] = {0x67, 0x3F, 0x90, 0xC9, 0x25, 0xF0, 0x4A, 0xB1, 0x12}; /* CRC unaligned input data */
   uint32_t seed_value = 0xFFFFFFFF; /* Seed value for custom CRC32 */
   uint32_t polynomial = 0x2CEEA6C8; /* Polynomial value for custom CRC32 */

.. figure:: _static/sample_code_custom_crc32.png
    :alt: CRC 32 Sample Code
    :align: center

    Custom CRC 32 Sample Code

.. figure:: _static/sample_code_custom_crc32_cont.png
    :alt: CRC 32 Sample Code Continued
    :align: center

    Custom CRC 32 Sample Code Continued

.. figure:: _static/web_custom_crc32_output.png
    :alt: Web Custom CRC 16 Output
    :align: center

    Web Custom CRC 32 Output

.. figure:: _static/flatboard_custom_crc32_output.png
    :alt: Flatboard Custom CRC 32 Output
    :align: center

    Flatboard Custom CRC 32 Output

Executing Binary on the DevKit
==============================

To execute binaries on the DevKit board, follow these steps:

1. Open the **Debug Configuration** window with *Create, manage, and run configurations*.

   .. figure:: _static/debug_config_window.png
      :alt: Debug Configuration Window
      :align: center

      Debug Configuration Window

2. In the **Connection** tab, ensure the correct Core and ULINKpro selections are made. In the **Select Target** section, choose:

   - ``Cortex-M55_0`` for M55-HP core
   - ``Cortex-M55_1`` for M55-HE core

   .. figure:: _static/connections_tab.png
      :alt: Connection Tab Settings
      :align: center

      Connection Tab Settings

3. In the **Debugger** tab:

   - Select **Connect Only**.
   - Use the ``loadfile`` command to specify the path to the application’s ``.elf`` file.
   - Click the **Debug** symbol to load debugging information.
   - Click **Apply** and then **Debug** to start the debugging process.

   .. figure:: _static/debugger_tab.png
      :alt: Debugger Tab Settings
      :align: center

      Debugger Tab Settings
