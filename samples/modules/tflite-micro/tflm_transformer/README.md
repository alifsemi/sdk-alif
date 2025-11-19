# BERT-Tiny Transformer on Ethos-U85 NPU

A TensorFlow Lite Micro application demonstrating BERT-Tiny transformer inference on Alif Ensemble E8 using the ARM Ethos-U85 NPU with Zephyr RTOS 4.1.

## Quick Start

### For RTSS_HP Core
```bash
cd ${SDK_ALIF_DIR}/zephyr

west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
  ../alif/samples/modules/tflite-micro/tflm_transformer/ \
  -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256 \
  -DEXTRA_DTC_OVERLAY_FILE="boards/alif_e8_dk_rtss_hp.overlay;boards/enable_ethosu85.overlay"

west flash
minicom -D /dev/ttyACM0 -b 115200
```

### For RTSS_HE Core
```bash
cd ${SDK_ALIF_DIR}/zephyr

west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
  ../alif/samples/modules/tflite-micro/tflm_transformer/ \
  -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256 \
  -DEXTRA_DTC_OVERLAY_FILE="boards/alif_e8_dk_rtss_he.overlay;boards/enable_ethosu85.overlay"

west flash
minicom -D /dev/ttyACM1 -b 115200
```

## Overview

This application demonstrates real-time transformer model inference using:

- **Model**: BERT-Tiny (2 layers, 96 hidden, 869KB INT8 quantized)
- **NPU**: ARM Ethos-U85 with 256 MACs
- **RTOS**: Zephyr 4.1 with multi-threaded inference pipeline
- **Board**: Alif Ensemble E8 Development Kit
- **Cores**: Support for both RTSS_HP (1MB DTCM) and RTSS_HE (256KB DTCM)

### Key Features

✅ **Dual-Core Support**: Optimized configurations for both HP and HE cores  
✅ **NPU Acceleration**: Leverages Ethos-U85 for matrix operations  
✅ **Memory Optimized**: Core-specific tensor arena sizing  
✅ **Multi-Threaded**: Parallel inference job processing  
✅ **Production Ready**: Successfully ported from Zephyr 3.6 to 4.1  

## Architecture

### Model Specifications

| Property | Value |
|----------|-------|
| Architecture | BERT-Tiny (transformer) |
| Layers | 2 |
| Hidden Size | 96 |
| Attention Heads | 4 |
| Sequence Length | 32 tokens |
| Model Size | 869KB (INT8 quantized) |
| Optimization | Vela for Ethos-U85-256 |

### Memory Configuration

| Core | DTCM | Tensor Arena | Stacks | Heap | Margin |
|------|------|--------------|--------|------|--------|
| RTSS_HP | 1MB | 700KB | 16KB | 24KB | ~282KB |
| RTSS_HE | 256KB | 128KB | 16KB | 24KB | ~86KB |

### Threading Model

```
┌─────────────┐
│   main()    │
└──────┬──────┘
       │
       ├──> Sender Task 0 (2KB stack) ──┐
       ├──> Sender Task 1 (2KB stack) ──┤
       │                                 │
       │                            ┌────▼────┐
       │                            │  Queue  │
       │                            └────┬────┘
       │                                 │
       └──> Runner Task 0 (8KB stack) <─┘
                    │
                    ├──> TFLite Micro Interpreter
                    ├──> Ethos-U85 NPU Delegate
                    └──> Inference Execution
```

## Hardware Setup

### Board: Alif Ensemble E8 Development Kit

- **Processor**: Dual Cortex-M55 (RTSS_HP + RTSS_HE)
- **NPU**: ARM Ethos-U85 (256 MACs)
- **RAM**: 
  - RTSS_HP: 1MB DTCM
  - RTSS_HE: 256KB DTCM
  - Shared: 4MB SRAM1
- **Flash**: 128MB OSPI (XIP capable)

### Console Connections

| Core | UART | Device (Linux) | Baud Rate |
|------|------|----------------|-----------|
| RTSS_HP | UART4 | /dev/ttyACM0 | 115200 |
| RTSS_HE | UART2 | /dev/ttyACM1 | 115200 |

## Build Configuration

### CMake Parameters

| Parameter | Value | Purpose |
|-----------|-------|---------|
| Board | `alif_e8_dk/ae822fa0e5597xx0/rtss_hp` or `rtss_he` | Target core selection |
| NPU Config | `ethos-u85-256` | NPU MAC configuration |
| Overlays | Board + NPU overlays | Device tree customization |

## Expected Output

### Successful Inference (HP Core)

```
Starting BERT-Tiny Transformer Model Demo on Ethos-U85 NPU
Model: bert_tiny
Tensor arena size: 716800 bytes
Number of inference tasks: 1
Number of job tasks: 2
Number of jobs per task: 2
Total inferences: 4

Creating 2 sender threads
Creating 1 runner threads
Starting threads...

sender 0: Sending inference. job=0x20052ce0, name=bert_tiny
sender 1: Sending inference. job=0x20052d00, name=bert_tiny
runner 0: Received inference job. job=0x20052ce0
runner 0: Starting NPU inference...
runner 0: Inference complete
runner 0: Sending inference response. job=0x20052ce0
sender 0: Received job response. job=0x20052ce0, status=0
sender 0: Inference successful!
sender 0: Sending inference. job=0x20052ce0, name=bert_tiny
runner 0: Received inference job. job=0x20052d00
...
All inferences completed successfully!
```

## Performance

### Inference Execution

- **NPU Accelerated Operations**: Attention mechanisms, FC layers, layer norm
- **CPU Operations**: Model setup, I/O copying, scheduling
- **Typical Inference Time**: Varies based on sequence length and model complexity
- **Throughput**: Multiple concurrent inferences via job queue

### Resource Utilization

**RTSS_HP (1MB DTCM):**
- Tensor Arena: 700KB (68%)
- Application Code/Data: ~40KB
- Stacks: 16KB
- Heaps: 24KB
- **Available Margin**: ~282KB

**RTSS_HE (256KB DTCM):**
- Tensor Arena: 128KB (50%)
- Application Code/Data: ~40KB
- Stacks: 16KB
- Heaps: 24KB
- **Available Margin**: ~86KB

## Common Issues

### Build Fails with RAM Overflow

**Problem**: `region 'RAM' overflowed by X bytes`

**Solution**: Tensor arena too large for DTCM. Reduce size in `model_u85_256.h`:
- HP: Reduce from 700KB (current safe value)
- HE: Reduce from 128KB (current safe value)

### No Console Output

**Problem**: No prints appear after flashing

**Solution**: 
- HP uses UART4 → try `/dev/ttyACM0`
- HE uses UART2 → try `/dev/ttyACM1`
- Verify 115200 baud rate
- Check USB cable connection

### Memory Corruption at Runtime

**Problem**: Application crashes or accesses secure memory

**Solution**: Total RAM usage exceeds DTCM:
1. Check linker map: `arm-zephyr-eabi-size build/zephyr/zephyr.elf`
2. Reduce tensor arena size
3. Reduce stack sizes in `prj.conf`
4. Consider using SRAM1 instead of DTCM

### Inference Fails

**Problem**: `Failed to allocate tensors` error

**Solution**: Tensor arena too small:
1. Increase `TENSOR_ARENA_SIZE` (if DTCM allows)
2. Move to SRAM1 for more space (see BUILDING.md)

## Advanced Usage

### Customizing Inference Jobs

Edit `src/main.cpp`:

```cpp
#define NUM_INFERENCE_TASKS 1    // NPU worker threads
#define NUM_JOB_TASKS 2          // Job sender threads
#define NUM_JOBS_PER_TASK 2      // Jobs per sender
```

### Using Different Models

1. Optimize your model with Vela for Ethos-U85:
   ```bash
   vela --accelerator-config=ethos-u85-256 \
        --system-config=Ethos_U85_SRAM_MRAM \
        model.tflite
   ```

2. Convert to C arrays and update includes in `CMakeLists.txt`

3. Adjust `TENSOR_ARENA_SIZE` based on model requirements

### Memory Profiling

Check actual memory usage:

```bash
# Build with size optimization
west build ... -- -DCONFIG_SIZE_OPTIMIZATIONS=y

# Check ELF sections
arm-zephyr-eabi-size -A build/zephyr/zephyr.elf

# View memory regions
arm-zephyr-eabi-objdump -h build/zephyr/zephyr.elf
```

## Documentation

- **BUILDING.md**: Comprehensive build instructions and troubleshooting
- **README.rst**: ReStructuredText version for Zephyr docs
- **README.md**: This file - quick reference guide

## References

- [Alif Semiconductor](https://alifsemi.com/)
- [ARM Ethos-U85 Documentation](https://developer.arm.com/Processors/Ethos-U85)
- [Zephyr RTOS](https://docs.zephyrproject.org/)
- [TensorFlow Lite Micro](https://www.tensorflow.org/lite/microcontrollers)
- [BERT Paper](https://arxiv.org/abs/1810.04805)

## License

Copyright (C) 2025 Alif Semiconductor  
SPDX-License-Identifier: Apache-2.0

## Support

For issues or questions:
- Alif Developer Forum
- GitHub Issues (if using open-source repository)
- Technical Support: contact@alifsemi.com

---

**Status**: ✅ Tested and working on both RTSS_HP and RTSS_HE cores (Nov 2025)
