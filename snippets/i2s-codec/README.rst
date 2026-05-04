.. _snippet-i2s-codec:

I2S Codec Application
######################

Overview
********

This snippet selects the appropriate overlay fragment to enable the I2S controller and WM8904 codec for the
`i2s_codec` sample.

The sample entry point is :zephyr_file:`samples/drivers/i2s_codec/src/main.c`. It:

* configures the WM8904 codec for I2S playback,
* configures the I2S TX path with a small static memory slab,
* generates a simple 1 kHz tone in software instead of storing a PCM file in flash,
* streams stereo 24-bit samples using 32-bit containers, and
* plays the tone for a short, fixed duration.

Building and Running
********************

Example command to build:

.. code-block:: console

   west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he -S i2s-codec ../alif/samples/drivers/i2s_codec -p
   OR
   west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he ../alif/samples/drivers/i2s_codec -p -- -DSNIPPET=i2s-codec

The application can be found under :zephyr_file:`samples/drivers/i2s_codec` in the Zephyr tree.

Application Output
*******************

Expected console output is similar to:

.. code-block:: console

   Playing tone sample at 48000 Hz
   Tone sample playback complete
