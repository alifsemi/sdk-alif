# BERT-Tiny Transformer on Ethos-U85 NPU

A TensorFlow Lite Micro application demonstrating BERT-Tiny transformer inference on Alif Ensemble E8 using the ARM Ethos-U85 NPU with Zephyr RTOS 4.1.

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

## Build Notes

- Supported boards: `alif_e8_dk/ae822fa0e5597xx0/rtss_hp` and `rtss_he`
- NPU configuration: `ethos-u85-256`
- Required overlays: Board-specific overlay + `enable_ethosu85.overlay`
- Refer to the SDK User Guide for detailed build and flash instructions

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

## Documentation

Refer to the SDK User Guide for:
- Build and flash instructions
- Troubleshooting common issues
- Memory profiling and optimization techniques

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
