.. _tflite-micro-alif-inference-sample:

Keyword Spotting using Generic Inference Runner
###############################################

Overview
********

This sample demonstrates how to use a generic inference runner to perform Keyword Spotting (KWS) on Alif devices.

Requirements
************

- Alif Ensemble or Balletto Development Kit

Building and Running
********************

This sample is located at :zephyr_file:`samples/modules/tflite-micro/alif_inference` in the sdk-alif tree.

To build the sample, you first need to pull in the optional dependencies by running the following commands:

.. code-block:: console

   west config manifest.group-filter -- +optional
   west update

To build the sample for the **U55** version:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S ethos-u55-enable samples/modules/tflite-micro/alif_inference

To build the sample for the **U85** version:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/modules/tflite-micro/alif_inference -S ethos-u85-enable -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

