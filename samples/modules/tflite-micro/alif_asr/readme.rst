.. _tflite-micro-alif-asr-sample:

Machine Learning Automatic Speech Recognition Sample
########################################

Overview
********

This sample is a Zephyr port of the `Alif ML Embedded Evaluation Kit <https://github.com/alifsemi/alif_ml-embedded-evaluation-kit>`_
for the automatic speech recognition (ASR) use case.

Use case has also push-to-talk feature which is enabled by default for the project.

Push-to-talk:
1. app starts by going to STOP mode
2. wakeup via joystick button, keep pressed to record voice. Once button released/max time (~10s)
   inference is run to recorded speech and printed to console
3. app goes back to STOP mode

CONFIG_OUTPUT_TO_LINE_OUT can used to listen the recorded voice.
When enabled, after speech record is finished and before inference is run, whole record is forwarded to line out.

Requirements
************

- Alif Ensemble or Balletto Development Kit
- For Balletto Development Kit, some modifications are required to be done.
	- remove resistor R64
	- add jumper wire between SW5 (P5_7) and P15_0 to use button as Push-to-talk button

Building and Running
********************

This sample is located at :zephyr_file:`samples/modules/tflite-micro/alif_asr` in the sdk-alif tree.

To build the sample, you first need to pull in the optional dependencies by running the following commands:

.. code-block:: console

   west config manifest.group-filter -- +optional
   west update
