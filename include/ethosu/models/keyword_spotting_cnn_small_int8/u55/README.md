# Ethos-U55 Model Files for Keyword Spotting

This directory contains the Ethos-U55 optimized model files for the keyword spotting application.

## Required Files

1. **input.h** - Sample input data
2. **output.h** - Expected output data  
3. **model_u55_128.h** - Model optimized for Ethos-U55 with 128 MACs
4. **model_u55_256.h** - Model optimized for Ethos-U55 with 256 MACs

## Generating U55 Model Files

To generate the U55 model files from a TensorFlow Lite model:

```bash
# For U55-128 MAC configuration
vela model.tflite \
  --accelerator-config=ethos-u55-128 \
  --system-config=Ethos_U55_High_End_Embedded \
  --memory-mode=Shared_Sram \
  --output-dir=./output_u55_128

# For U55-256 MAC configuration
vela model.tflite \
  --accelerator-config=ethos-u55-256 \
  --system-config=Ethos_U55_High_End_Embedded \
  --memory-mode=Shared_Sram \
  --output-dir=./output_u55_256

# Convert the optimized .tflite to C array headers
# (Use your preferred conversion tool)
```

## File Format

All files must follow this format:

### input.h
```cpp
__attribute__((aligned(16), section(".rodata.tflm_input")))
const unsigned char inputData[] = { /* input bytes */ };
```

### output.h
```cpp
__attribute__((aligned(16), section(".rodata.tflm_output")))
const unsigned char expectedOutputData[] = { /* output bytes */ };
```

### model_u55_*.h
```cpp
#define TENSOR_ARENA_SIZE 50000
const size_t tensorArenaSize = TENSOR_ARENA_SIZE;

__attribute__((aligned(16), section(".rodata.tflm_model")))
const unsigned char networkModelData[] = { /* model bytes */ };
const char *modelName = "keyword_spotting_cnn_small_int8";
```

## Board Support

- **U55-128**: Used on RTSS_HE cores (B1, E1C, E3, E7 HE cores)
- **U55-256**: Used on RTSS_HP cores (E3, E7 HP cores)

The build system automatically selects the appropriate model based on the target board's core type.
