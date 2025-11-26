.. _pdm:

===
PDM
===

Introduction
============

This document explains how to create, compile, and run a demo application for the Pulse Density Modulation (PDM) controller IP provided by Alif Semiconductor" and integrated into Ensemble" devices. Key features include:

- A PDM (Pulse Density Modulation) microphone produces 1-bit digital data streams in Pulse Density Modulated format.
- The PDM Audio module supports eight audio channels, with each channel connecting to one microphone.
- PDM Audio converts each 1-bit PDM audio input into 16-bit PCM (Pulse Code Modulated) format.
- The host reads the 16-bit PCM data for each microphone through a set of APB registers.

.. figure:: _static/pdm_hardware.png
   :alt: PDM Hardware
   :align: center

   PDM Hardware

.. include:: prerequisites.rst

Hardware Connections and Setup for PDM
--------------------------------------

The DevKit board has internal PDM microphones connected to channels 4 and 5, requiring no additional hardware connections for these channels.

.. figure:: _static/pdm_microphone_b0.png
   :alt: PDM Microphone on B0 Flat Board
   :align: center

   PDM Microphone on B0 Flat Board

**For Channels 4 and 5**:

- **Data Line Configuration**:
  - Connect the data line from `PDM_D2_B` on the flat board to `P5_4`.
- **Clock Line Configuration**:
  - Connect the clock line from `PDM_C2_A` on the flat board to `P6_7`.

**For Channels 0 and 1**:

- **Data Line Configuration**:
  - Connect pin `P5_4` on Flat board J14 to pin `P0_4` on Flat board J11.
- **Clock Line Configuration**:
  - Connect pin `P6_7` on Flat board J15 to pin `P0_5` on Flat board J11.

**For Channels 2 and 3**:

- **Data Line Configuration**:
  - Connect pin `P5_4` on Flat board J14 to pin `P0_6` on Flat board J11.
- **Clock Line Configuration**:
  - Connect pin `P6_7` on Flat board J15 to pin `P0_7` on Flat board J11.

**For Channels 6 and 7**:

- **Data Line Configuration**:
  - Connect pin `P5_4` on Flat board J14 to pin `P5_1` on Flat board J12.
- **Clock Line Configuration**:
  - Connect pin `P6_7` on Flat board J15 to pin `P5_2` on Flat board J12.

**PDM Channel Configurations**:

Use the following channel configurations in the application before building:

.. code-block:: c

   /* PDM Channel 0 configurations */
   #define CH0_PHASE             0x00000003
   #define CH0_GAIN              0x00000013
   #define CH0_PEAK_DETECT_TH    0x00060002
   #define CH0_PEAK_DETECT_ITV   0x00020027

   /* PDM Channel 1 configurations */
   #define CH1_PHASE             0x0000001F
   #define CH1_GAIN              0x0000000D
   #define CH1_PEAK_DETECT_TH    0x00060002
   #define CH1_PEAK_DETECT_ITV   0x0004002D

   /* PDM Channel 2 configurations */
   #define CH2_PHASE             0x00000003
   #define CH2_GAIN              0x00000013
   #define CH2_PEAK_DETECT_TH    0x00060002
   #define CH2_PEAK_DETECT_ITV   0x00020027

   /* PDM Channel 3 configurations */
   #define CH3_PHASE             0x0000001F
   #define CH3_GAIN              0x0000000D
   #define CH3_PEAK_DETECT_TH    0x00060002
   #define CH3_PEAK_DETECT_ITV   0x0004002D

   /* PDM Channel 4 configurations */
   #define CH4_PHASE             0x0000001F
   #define CH4_GAIN              0x0000000D
   #define CH4_PEAK_DETECT_TH    0x00060002
   #define CH4_PEAK_DETECT_ITV   0x0004002D

   /* PDM Channel 5 configurations */
   #define CH5_PHASE             0x00000003
   #define CH5_GAIN              0x00000013
   #define CH5_PEAK_DETECT_TH    0x00060002
   #define CH5_PEAK_DETECT_ITV   0x00020027

   /* PDM Channel 6 configurations */
   #define CH6_PHASE             0x0000001F
   #define CH6_GAIN              0x0000000D
   #define CH6_PEAK_DETECT_TH    0x00060002
   #define CH6_PEAK_DETECT_ITV   0x0004002D

   /* PDM Channel 7 configurations */
   #define CH7_PHASE             0x00000003
   #define CH7_GAIN              0x00000013
   #define CH7_PEAK_DETECT_TH    0x00060002
   #define CH7_PEAK_DETECT_ITV   0x00020027

**FIR Coefficients**:

.. code-block:: c

   /* Channel 0 FIR coefficient */
   uint32_t ch0_fir[18] = {0x00000000, 0x000007FF, 0x00000000, 0x00000004, 0x00000004, 0x000007FC, 0x00000000, 0x000007FB, 0x000007E4,
                           0x00000000, 0x0000002B, 0x00000009, 0x00000016, 0x00000049, 0x00000793, 0x000006F8, 0x00000045, 0x00000178};

   /* Channel 1 FIR coefficient */
   uint32_t ch1_fir[18] = {0x00000001, 0x00000003, 0x00000003, 0x000007F4, 0x00000004, 0x000007ED, 0x000007F5, 0x000007F4, 0x000007D3,
                           0x000007FE, 0x000007BC, 0x000007E5, 0x000007D9, 0x00000793, 0x00000029, 0x0000072C, 0x00000072, 0x000002FD};

   /* Channel 2 FIR coefficient */
   uint32_t ch2_fir[18] = {0x00000000, 0x000007FF, 0x00000000, 0x00000004, 0x00000004, 0x000007FC, 0x00000000, 0x000007FB, 0x000007E4,
                           0x00000000, 0x0000002B, 0x00000009, 0x00000016, 0x00000049, 0x00000793, 0x000006F8, 0x00000045, 0x00000178};

   /* Channel 3 FIR coefficient */
   uint32_t ch3_fir[18] = {0x00000001, 0x00000003, 0x00000003, 0x000007F4, 0x00000004, 0x000007ED, 0x000007F5, 0x000007F4, 0x000007D3,
                           0x000007FE, 0x000007BC, 0x000007E5, 0x000007D9, 0x00000793, 0x00000029, 0x0000072C, 0x00000072, 0x000002FD};

   /* Channel 4 FIR coefficient */
   uint32_t ch4_fir[18] = {0x00000001, 0x00000003, 0x00000003, 0x000007F4, 0x00000004, 0x000007ED, 0x000007F5, 0x000007F4, 0x000007D3,
                           0x000007FE, 0x000007BC, 0x000007E5, 0x000007D9, 0x00000793, 0x00000029, 0x0000072C, 0x00000072, 0x000002FD};

   /* Channel 5 FIR coefficient */
   uint32_t ch5_fir[18] = {0x00000000, 0x000007FF, 0x00000000, 0x00000004, 0x00000004, 0x000007FC, 0x00000000, 0x000007FB, 0x000007E4,
                           0x00000000, 0x0000002B, 0x00000009, 0x00000016, 0x00000049, 0x00000793, 0x000006F8, 0x00000045, 0x00000178};

   /* Channel 6 FIR coefficient */
   uint32_t ch6_fir[18] = {0x00000001, 0x00000003, 0x00000003, 0x000007F4, 0x00000004, 0x000007ED, 0x000007F5, 0x000007F4, 0x000007D3,
                           0x000007FE, 0x000007BC, 0x000007E5, 0x000007D9, 0x00000793, 0x00000029, 0x0000072C, 0x00000072, 0x000002FD};

   /* Channel 7 FIR coefficient */
   uint32_t ch7_fir[18] = {0x00000000, 0x000007FF, 0x00000000, 0x00000004, 0x00000004, 0x000007FC, 0x00000000, 0x000007FB, 0x000007E4,
                           0x00000000, 0x0000002B, 0x00000009, 0x00000016, 0x00000049, 0x00000793, 0x000006F8, 0x00000045, 0x00000178};

Hardware Setup for Multiple PDM Channels
----------------------------------------

The DevKit board has internal PDM microphones. To test PDM channels 0, 1, 2, and 3, two flat boards are required, as each board contains a pair of microphones sufficient for channels 0 and 1. For channels 2 and 3, an additional pair of microphones is needed, requiring another board with PDM microphones connected to channels 2 and 3. The same configuration applies to other channels.

**For Channels 0 and 1**:

- **Data Line Configuration**:
  - Connect pin `P5_4` on Flat board J14 to pin `P0_4` on Flat board J11.
- **Clock Line Configuration**:
  - Connect pin `P6_7` on Flat board J15 to pin `P0_5` on Flat board J11.

**For Channels 2 and 3**:

- **Data Line Configuration**:
  - Connect pin `P5_4` on Flat board J14 to pin `P0_6` on Flat board J11.
- **Clock Line Configuration**:
  - Connect pin `P6_7` on Flat board J15 to pin `P0_7` on Flat board J11.

Hardware Setup for Multiple LPPDM Channels
------------------------------------------

The DevKit has internal PDM microphones. To test LPPDM channels 0, 1, 2, and 3, two flat boards are required, as each board contains a pair of microphones sufficient for channels 0 and 1. For channels 2 and 3, an additional pair of microphones is needed, requiring another board with PDM microphones connected to channels 2 and 3. The same configuration applies to other channels.

**For Channels 0 and 1**:

- **Data Line (LPPDM_D0_B)**:
  - Connect pin `P5_4` on Flat board J14 to pin `P3_5` on Flat board J11.
- **Clock Line (LPPDM_C0_B)**:
  - Connect pin `P6_7` on Flat board J15 to pin `P3_4` on Flat board J11.

**For Channels 2 and 3**:

- **Data Line (LPPDM_D1_B)**:
  - Connect pin `P5_4` on Flat board J14 to pin `P3_7` on Flat board J11.
- **Clock Line (LPPDM_C1_B)**:
  - Connect pin `P6_7` on Flat board J15 to pin `P3_6` on Flat board J11.

Hardware Setup for Low-Power PDM (LPPDM)
----------------------------------------

The DevKit has internal PDM microphones. To test LPPDM channels 0 and 1, connect the PDM data line of the flat board's PDM microphone to the LPPDM data line of `P3_5` (J11 on the flat board).

**For Channels 0 and 1**:

- **Data Line (LPPDM_D0_B)**:
  - Connect pin `P5_4` on Flat board J14 to pin `P3_5` on Flat board J11.
- **Clock Line (LPPDM_C0_B)**:
  - Connect pin `P6_7` on Flat board J15 to pin `P3_4` on Flat board J11.

.. include:: note.rst

Building an PDM Application with Zephyr
========================================

Follow these steps to build the PDM application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the M55 HE core:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/audio/dmic_alif/

3. Build commands for applications on the M55 HP core:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/audio/dmic_alif/

Building an LPPDM Application with Zephyr
===========================================

Both PDM and LPPDM use similar applications. To build the LPPDM application, modify the PDM sample application code from `DT_NODELABEL(pdm)` to `DT_NODELABEL(lppdm)`.

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

.. code-block:: bash

   const struct device *pcmj_device = DEVICE_DT_NODELABEL(lppdm)

Follow these steps to build your Zephyr-based LPPDM application using the GCC compiler and the Alif Zephyr SDK:

1. Fetch the Alif Zephyr SDK source at the desired revision:

.. code-block:: bash

   mkdir sdk-alif
   cd sdk-alif
   west init -m https://github.com/alifsemi/sdk-alif.git --mr ${revision}
   west update

.. note::
   Replace ``${revision}`` with any SDK revision (branch/tag/commit SHA) you wish to achieve.
   This can be ``main`` if you want the latest state, or any commit SHA or tag (e.g., ``west init -m https://github.com/alifsemi/sdk-alif.git --mr v1.2.0``).

2. Navigate to the Zephyr directory:

.. code-block:: bash

   cd zephyr

3. Remove the existing build directory and build the application:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/audio/dmic_alif/

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Procedure to Test PDM and LPPDM
===============================

1. Select PDM channels 4 and 5 in the test application.

.. code-block:: c

    /**
     * The list of channels to test
     * The number of channels should match the PDM_CHANNELS
     */
    #define PDM_CHANNELS    PDM_MASK_CHANNEL_4 | PDM_MASK_CHANNEL_5


For multiple channels, consider enabling channels 0, 1, 2, and 3.

.. code-block:: c

    /**
     * The list of channels to test
     * The number of channels should match the PDM_CHANNELS
     */
    #define PDM_CHANNELS    PDM_MASK_CHANNEL_0 | PDM_MASK_CHANNEL_1 | PDM_MASK_CHANNEL_2 | PDM_MASK_CHANNEL_3

1. Specify the block size to store the PCM data.

.. code-block:: bash

   #define PCMJ BlockSize >> >>   50000

3. Specify the number of samples to store the captured PCM data.

.. code-block:: bash

    /* Number of blocks in the slab */
   #define MEM_SLAB_NUM_BLOCKS >> >>   1


- If `PCMJ_BLOCK_SIZE` is 50000 and `MEM_SLAB_NUM_BLOCKS` is 1, 50000 PCM samples can be stored in the `pcmj_data` buffer.

- If `PCMJ_BLOCK_SIZE` is 50000 and `MEM_SLAB_NUM_BLOCKS` is 2, 100000 PCM samples can be stored in the `pcmj_data` buffer.

4. Choose the mode of operation in the `pdm_mode` API.

5. Set the FIFO watermark value in the DTS file.

6. Store the channel status configuration values for each channel.

7. Build the project and flash the generated `.elf` file onto the target.

8. Once all hardware connections are completed, power on the DevKit.

9. Start playing audio or speak near the PDM microphone on the DevKit.

10. When the sample count reaches the maximum value, the IRQ will be disabled, and audio capture will stop.

11. Stop the application code.

12. The PCM samples will be stored in the `pcmj_data` buffer. Print the base address of the `pcmj_data` buffer.

The text below shows channels 4 and 5 enabled, with the buffer address at `0x20000c3c`. 50,000 PCM samples are stored in the `pcmj_data` buffer, and the stored PCM samples are being printed.

PCM Samples Buffer (Channels 4 and 5, Address 0x20000c3c)
---------------------------------------------------------

.. code-block:: text

   Start Speaking or Play some Audio!
   [00:00:00.000,000] <inf> alif_pdm: Memory block allocated : 0x2000dfd8
   [00:00:01.563,000] <inf> PDM: Block freed at address: 0x2000dfd8

   stop recording
   [00:00:01.563,000] <inf> PDM: PCM samples will be stored in 0x20000c3c address and size of buffer is 50000
   [00:00:01.563,000] <inf> PDM: pcm data : 0x20000c3c

   [00:00:01.563,000] <inf> PDM:   0 0 0 0 0 0 0 0
   [00:00:01.563,000] <inf> PDM:   0 0 0 0 0 0 0 0
   [00:00:01.563,000] <inf> PDM:   0 0 0 0 0 0 0 0
   [00:00:01.563,000] <inf> PDM:   0 0 0 0 0 0 0 0

   [00:00:01.563,000] <inf> PDM:   ff ff 0 0 fb ff 0 0
   [00:00:01.563,000] <inf> PDM:   f2 ff 5 0 ee ff 2 0
   [00:00:01.563,000] <inf> PDM:   14 0 ee ff b 0 e0 ff
   [00:00:01.563,000] <inf> PDM:   49 0 f7 ff 68 0 fa ff
   [00:00:01.563,000] <inf> PDM:   91 0 1d 0 15 1 94 0
   [00:00:01.564,000] <inf> PDM:   16 1 71 0 e0 1 a2 ff


13. Copy the buffer address for channels 4 and 5.

14. Open the memory section, paste the buffer address, and press Enter.

15. In the buffer memory section, view the converted PCM samples stored in the buffer.

    The screenshot below shows the PCM samples stored at the specified buffer address.

    .. figure:: _static/pdm_memory_section.png
       :alt: PCM Samples in Memory Section
       :align: center

       PCM Samples in Memory Section

16. To export the memory:
    - Go to the right-most corner.
    - Click on the **View** menu.
    - Select the **Export Memory** option.

    .. figure:: _static/pdm_memory_export.png
       :alt: Export Memory Option
       :align: center

       Export Memory Option

17. By default, the start and end address will be present in the memory bounds.

18. Specify the length in bytes, corresponding to the buffer size.
    For example, if the number of samples is 50,000, the length in bytes will be 50,000 bytes.

19. Store the PCM samples in binary format with the filename `memory.bin`.

    .. figure:: _static/storing_pcm_samples.png
       :alt: Storing PCM Samples
       :align: center

       Storing PCM Samples


20. To play the PCM data, use the `pcmplay.c` file, which includes resources such as the `memory.bin` file, channel number, data types of the memory buffer, sampling rate, frequency, and generates the `pcm_samples.pcm` audio file.

21. Compile the `pcmplay.c` file using `gcc` to generate the `pcm_samples.pcm` file in the same directory.

22. Use the `ffplay` command to play the audio:

    .. code-block:: bash

       ffplay -f s16le -ac 2 -ar 8k pcm_samples.pcm

    Where:
    - `s16le`: 16-bit little-endian system.
    - `-ac`: Specifies the number of channels.
    - `-ar`: Specifies the frequency.

23. After entering the command, press Enter and connect the microphone to the NUC. The user will hear recorded audio (approximately 3-4 seconds) using any speaker.
For better quality, use earphones.

    .. figure:: _static/playing_audio_pdm.png
       :alt: Playing Audio
       :align: center

       Playing Audio

24. For LPPDM, follow the same procedure, selecting channels 0 and 1 instead of channels 4 and 5.

25. Select the mode of operation in the `pdm_mode` API.

26. Store the channel status configuration values for channels 0 and 1.

27. Build the project and flash the generated `.elf` file onto the target.

28. Follow the same PDM procedure to obtain PCM samples for LPPDM and play the recorded audio using the `ffplay` command.

Alternative Method Using Audacity Player
========================================

An alternative to `ffplay` is to use the Audacity player. Download and install Audacity, then follow these steps:

1. Go to **File** -> **Import** -> **Raw Data** and load the `memory.bin` file.

    .. figure:: _static/using_audacity_player.png
       :alt: Using Audacity Player
       :align: center

       Using Audacity Player


2. Select 16-bit PCM with little-endian byte order.

3. Specify the number of channels used by your application (e.g., 2 channels for the sample application; select 4 channels if using 4 channels).

4. Set the sample rate according to the PDM mode used in your application:

   - For standard voice with a baseband sampling rate of 8 kHz, use 8000.

   - For high-quality voice with a 512 PDM clock frequency, use 16000.

   .. figure:: _static/audacity_settings.png
      :alt: Audacity Settings for PDM
      :align: center

      Audacity Settings for PDM

PDM Modes
=========

   .. figure:: _static/pdm_modes.png
      :alt: PDM Modes
      :align: center

      PDM Modes

