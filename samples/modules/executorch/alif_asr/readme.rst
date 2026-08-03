.. _executorch-alif-asr-sample:

Machine Learning Automatic Speech Recognition Sample
####################################################

Overview
********

This sample is a Zephyr port of the `Alif ML Embedded Evaluation Kit <https://github.com/alifsemi/alif_ml-embedded-evaluation-kit>`_
for the automatic speech recognition (ASR) use case.


Requirements
************

- Alif Ensemble Gen 2 (AE822FA0E5597XX0) development kit

Building and Running
********************

This sample is located at :zephyr_file:`samples/modules/executorch/alif_asr` in the sdk-alif tree.

To build the sample, you first need to pull in the optional dependencies and set up MLEK resources:

.. code-block:: console

   west config manifest.group-filter -- +optional
   west config manifest.project-filter -- +alif-mlek
   west config manifest.project-filter -- +executorch
   west update
   west executorch-setup

   cd modules/alif-mlek
   python3 set_up_default_resources.py --ml-frameworks executorch --executorch-path {add your path}/modules/lib/executorch

.. note:: set_up_default_resources.py will take a while to run, as it downloads the ML models and compiles them with Vela for the Ethos-U NPU.

If set_up_default_resources.py fails, delete the :zephyr_file:`modules/alif-mlek/resources_downloaded` directory and try again.

The last command downloads the ML models and compiles them with Vela for the Ethos-U NPU.
The generated model and labels source code is produced automatically at CMake configure time.


Build the sample with the following command:
.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he alif/samples/modules/executorch/alif_asr -S ethos-u85-enable -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256 -DML_FWK_TMP_MEM_SIZE=0x002C0000
