.. _alif-audio-dmic-sample:

Alif Audio DMIC Demo
##################################

Overview
********

This sample demonstrates capturing audio data from a digital microphone using the PDM interface.
The recorded audio is converted into PCM samples and stored in a memory buffer.
A small portion of the captured PCM data is printed to the console as sample output.

Building and Running
********************

To build the sample for the ALIF supported boards:

.. zephyr-app-commands::
      west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/samples/drivers/audio/dmic_alif -S alif-pdm

Replace "alif_e7_dk/ae722f80f55d5xx/rtss_he" with the appropriate board name for other supported boards.

Sample Output
*************

.. code-block:: console

	*** Booting Zephyr OS build 2010a10268ff ***
	[00:00:00.000,000] <inf> PDM: PDM init okay

	[00:00:00.000,000] <inf> PDM: memslab: 0x200003d4

	[00:00:00.000,000] <inf> PDM: channel_map 30 block size: 7530

	Start Speaking or Play some Audio!
	Stop recording
	[00:00:01.876,000] <inf> PDM: PCM samples will be stored in 0x20000d24 address and size of buffer is 60000

	[00:00:01.876,000] <inf> PDM: Block freed at address: 0x20017d00

	[00:00:01.876,000] <inf> PDM: pcm data : 0x20000d24

	[00:00:01.876,000] <inf> PDM:  0 0 0 0 0 0 0 0

	[00:00:01.876,000] <inf> PDM:  0 0 0 0 0 0 0 0

	[00:00:01.876,000] <inf> PDM:  0 0 0 0 0 0 0 0

	[00:00:01.876,000] <inf> PDM:  0 0 0 0 ff ff 0 0

	[00:00:01.876,000] <inf> PDM:  0 0 0 0 8 0 fb ff

	[00:00:01.876,000] <inf> PDM:  1a 0 fb ff fc ff f 0

	[00:00:01.876,000] <inf> PDM:  6 0 22 0 de ff c 0

	[00:00:01.876,000] <inf> PDM:  b4 ff d 0 a2 ff f8 ff

	[00:00:01.876,000] <inf> PDM:  1f ff 6a ff 11 ff 6f ff

	[00:00:01.876,000] <inf> PDM:  67 fe 4d 0 ef fd 6d 0
