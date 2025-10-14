.. _i2s:

===
I2S
===

Introduction
============

This document explains how to create, compile, and run a demo application for the Inter-IC Sound (I2S) controller IP provided by Alif Semiconductor™ and integrated into Ensemble™ and Balletto devices.

Overview
--------

The Inter-IC Sound (I2S™) is an electrical serial bus interface standard used for connecting digital audio devices. It is used to communicate Pulse-Code Modulation (PCM) audio data between integrated circuits in an electronic device. The I2S bus separates clock and serial data signals, resulting in simpler receivers than those required for asynchronous communication systems that need to recover the clock from the data stream.

.. figure:: _static/i2s_hardware_connections.png
   :alt: I2S Hardware Connections
   :align: center

   I2S Hardware Connections

.. include:: prerequisites.rst


Hardware Setup I2S
------------------

The DevKit board includes two I2S microphones (one for the left channel and one for the right channel) for recording stereo audio. However, it does not have a built-in headphone jack or speaker.

For E7 DevKits, connect an external speaker to the board using a level shifter circuit to provide the appropriate 3.3V voltage level to the speaker to play the audio.

.. figure:: _static/level_shifter_for_i2s.png
   :alt: Level Shifter (Only for DevKit E7)
   :align: center

   Level Shifter

.. figure:: _static/speaker_for_i2s.png
   :alt: Speaker (Only for DevKit E7)
   :align: center

   Speaker

.. figure:: _static/board_setup_i2s.png
   :alt: Board Setup
   :align: center

   Board Setup

Building I2S Application in Zephyr
====================================

Follow these steps to build your Zephyr-based I2S application using the GCC compiler and the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the M55 HE core using the Ninja build command:

.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/i2s/echo

Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Observations
============
The echo application plays back (or echoes) the sound from the microphone through the connected speaker, enabling real-time audio feedback.

.. include:: west_debug.rst
