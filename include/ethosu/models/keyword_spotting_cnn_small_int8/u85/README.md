# Ethos-U85 Model Files for Keyword Spotting

This directory should contain the Ethos-U85 optimized model files for the keyword spotting application.

## Required Files

1. **input.h** - Sample input data
2. **output.h** - Expected output data  
3. **model_u85_256.h** - Model optimized for Ethos-U85 with 256 MACs

## Generating U85 Model Files

To generate the U85 model files from a TensorFlow Lite model:

```bash
# Use Vela optimizer to convert for Ethos-U85-256
vela model.tflite \
  --accelerator-config=ethos-u85-256 \
  --system-config=Ethos_U85_SRAM_MRAM \
  --output-dir=./output

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

### model_u85_256.h
```cpp
__attribute__((aligned(16), section(".rodata.tflm_model")))
const unsigned char networkModelData[] = { /* model bytes */ };
const char modelName[] = "keyword_spotting_cnn_small_int8";
const int tensorArenaSize = <size_in_bytes>;
```

## Note

Currently, this directory contains placeholder files. You can:
1. Copy and convert the U55 model to U85 format
2. Use the same U55 model files temporarily (copy from u55/ directory)
3. Generate proper U85-optimized models using Vela

For now, you can copy the U55 files to test the build:
```bash
cp u55/input.h u85/
cp u55/output.h u85/
# Convert u55 model to u85 or use as placeholder
```
