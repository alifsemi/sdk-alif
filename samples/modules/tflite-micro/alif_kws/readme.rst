.. _tflite-micro-alif-kws-sample:

Machine Learning Keyword Spotting Sample
########################################

Overview
********

This sample is a Zephyr port of the `Alif ML Embedded Evaluation Kit <https://github.com/alifsemi/alif_ml-embedded-evaluation-kit>`_
for the keyword spotting (KWS) use case.

Requirements
************

- Alif Ensemble or Balletto Development Kit

Building and Running
********************

This sample is located at :zephyr_file:`samples/modules/tflite-micro/alif_kws` in the sdk-alif tree.

To build the sample, you first need to pull in the optional dependencies and set up MLEK resources:

.. code-block:: console

   west config manifest.group-filter -- +optional
   west config manifest.project-filter -- +alif-mlek
   west update
   python3 modules/alif-mlek/set_up_default_resources.py

The last command downloads the ML models and compiles them with Vela for the Ethos-U NPU.
The generated model and labels source code is produced automatically at CMake configure time.
