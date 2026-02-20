# Keyword Spotting (KWS) Model for ExecutorTorch

This directory contains the KWS model definition for use with ExecutorTorch on Alif Ensemble devices with Ethos-U NPU.

## Model Architecture

- **Architecture**: DS-CNN (Depthwise Separable CNN) Small
- **Input**: 49 time steps × 10 MFCC features (490 elements, int8 quantized)
- **Output**: 12 classes (silence, unknown, yes, no, up, down, left, right, on, off, stop, go)

## Generating the Model

To generate the `.pte` model file for use with the kws_ethosu sample:

```bash
cd /home/srinivas/executorch/modules/lib/executorch

# For Ethos-U55 with 256 MACs
python -m executorch.examples.arm.aot_arm_compiler \
    --model_name kws \
    --quantize \
    --delegate \
    -t ethos-u55-256 \
    --system_config Ethos_U55_High_End_Embedded \
    --memory_mode Shared_Sram \
    --output kws_u55_256.pte

# For Ethos-U55 with 128 MACs
python -m executorch.examples.arm.aot_arm_compiler \
    --model_name kws \
    --quantize \
    --delegate \
    -t ethos-u55-128 \
    --system_config Ethos_U55_High_End_Embedded \
    --memory_mode Shared_Sram \
    --output kws_u55_128.pte
```

## Using the Generated Model

Copy the generated `.pte` file to the kws_ethosu sample directory and build:

```bash
# Copy the model
cp kws_u55_256.pte /home/srinivas/executorch/alif/samples/modules/executorch/kws_ethosu/

# Build the application
cd /home/srinivas/executorch
west build -b ensemble_e7_devkit_sk alif/samples/modules/executorch/kws_ethosu \
    -- -DET_PTE_FILE_PATH=kws_u55_256.pte \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256
```

## Model Classes

The model recognizes 12 keywords:
1. silence
2. unknown
3. yes
4. no
5. up
6. down
7. left
8. right
9. on
10. off
11. stop
12. go

## References

- [ARM ML Zoo - Keyword Spotting](https://github.com/ARM-software/ML-zoo/tree/master/models/keyword_spotting)
- [Google Speech Commands Dataset](https://arxiv.org/abs/1804.03209)
- [DS-CNN Paper](https://arxiv.org/abs/1711.07128)
