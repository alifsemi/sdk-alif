/*
 * Copyright 2025 Alif Semiconductor
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * Copyright 2025 Arm Limited and/or its affiliates.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Keyword Spotting (KWS) Application using ExecutorTorch with Ethos-U NPU
 *
 * This application demonstrates running a keyword spotting CNN model
 * on the Ethos-U NPU using ExecutorTorch runtime with static input data.
 */

#include <errno.h>
#include <executorch/examples/arm/executor_runner/arm_memory_allocator.h>
#include <executorch/extension/data_loader/buffer_data_loader.h>
#include <executorch/extension/runner_util/inputs.h>
#include <executorch/runtime/core/memory_allocator.h>
#include <executorch/runtime/executor/program.h>
#include <executorch/runtime/platform/log.h>
#include <executorch/runtime/platform/platform.h>
#include <executorch/runtime/platform/runtime.h>
#include <math.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <cstring>
#include <memory>
#include <vector>

#include "model_pte.h"
#include "kws_input.h"
#include "kws_output.h"

using executorch::aten::ScalarType;
using executorch::aten::Tensor;
using executorch::aten::TensorImpl;
using executorch::extension::BufferCleanup;
using executorch::extension::BufferDataLoader;
using executorch::runtime::Error;
using executorch::runtime::EValue;
using executorch::runtime::HierarchicalAllocator;
using executorch::runtime::MemoryAllocator;
using executorch::runtime::MemoryManager;
using executorch::runtime::Method;
using executorch::runtime::MethodMeta;
using executorch::runtime::Program;
using executorch::runtime::Result;
using executorch::runtime::Span;
using executorch::runtime::Tag;
using executorch::runtime::TensorInfo;

#if defined(CONFIG_ARM_ETHOS_U)
extern "C" executorch::runtime::Error
executorch_delegate_EthosUBackend_registered(void);
#endif

#if !defined(ET_ARM_METHOD_ALLOCATOR_POOL_SIZE)
#define ET_ARM_METHOD_ALLOCATOR_POOL_SIZE (1572864)
#endif
const size_t method_allocation_pool_size = ET_ARM_METHOD_ALLOCATOR_POOL_SIZE;
unsigned char __attribute__((
    section(".alif_sram0.tensor_arena"),
    aligned(16))) method_allocation_pool[method_allocation_pool_size];

#if !defined(ET_ARM_BAREMETAL_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE)
#define ET_ARM_BAREMETAL_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE (1572864)
#endif

#if !defined(ET_ARM_BAREMETAL_FAST_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE)
#define ET_ARM_BAREMETAL_FAST_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE 0x600
#endif

const size_t temp_allocation_pool_size =
    ET_ARM_BAREMETAL_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE;
unsigned char __attribute__((
    section(".alif_sram0.tensor_arena"),
    aligned(16))) temp_allocation_pool[temp_allocation_pool_size];

#if defined(ET_ARM_BAREMETAL_FAST_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE)
extern "C" {
size_t ethosu_fast_scratch_size =
    ET_ARM_BAREMETAL_FAST_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE;
unsigned char __attribute__((section(".alif_sram0.ethosu_scratch"), aligned(16)))
dedicated_sram[ET_ARM_BAREMETAL_FAST_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE];
unsigned char* ethosu_fast_scratch = dedicated_sram;
}
#endif

namespace {

static const char* kws_labels[] = {
    "silence", "unknown", "yes", "no", "up", "down",
    "left", "right", "on", "off", "stop", "go"
};
static const int kws_num_labels = sizeof(kws_labels) / sizeof(kws_labels[0]);

Result<BufferCleanup> prepare_input_tensors(
    Method& method,
    MemoryAllocator& allocator,
    const uint8_t* input_data,
    size_t input_size) {
  MethodMeta method_meta = method.method_meta();
  size_t num_inputs = method_meta.num_inputs();
  size_t num_allocated = 0;

  void** inputs =
      static_cast<void**>(allocator.allocate(num_inputs * sizeof(void*)));
  ET_CHECK_OR_RETURN_ERROR(
      inputs != nullptr,
      MemoryAllocationFailed,
      "Could not allocate memory for pointers to input buffers.");

  for (size_t i = 0; i < num_inputs; i++) {
    auto tag = method_meta.input_tag(i);
    ET_CHECK_OK_OR_RETURN_ERROR(tag.error());

    if (tag.get() != Tag::Tensor) {
      ET_LOG(Debug, "Skipping non-tensor input %zu", i);
      continue;
    }
    Result<TensorInfo> tensor_meta = method_meta.input_tensor_meta(i);
    ET_CHECK_OK_OR_RETURN_ERROR(tensor_meta.error());

    void* data_ptr = allocator.allocate(tensor_meta->nbytes());
    ET_CHECK_OR_RETURN_ERROR(
        data_ptr != nullptr,
        MemoryAllocationFailed,
        "Could not allocate memory for input buffers.");
    inputs[num_allocated++] = data_ptr;

    Error err = Error::Ok;
    ScalarType scalar_type = tensor_meta->scalar_type();
    size_t num_elements = 1;
    auto sizes = tensor_meta->sizes();
    for (size_t k = 0; k < sizes.size(); k++) {
      num_elements *= sizes[k];
    }

    ET_LOG(Info, "Input tensor: scalar_type=%s, numel=%zu, nbytes=%zu",
           executorch::runtime::toString(scalar_type), num_elements, tensor_meta->nbytes());

    if (scalar_type == ScalarType::Float && input_size == num_elements) {
      ET_LOG(Info, "Converting uint8 input (%zu elements) to float32", input_size);
      float* float_data = static_cast<float*>(data_ptr);
      for (size_t j = 0; j < input_size; j++) {
        float_data[j] = (static_cast<float>(input_data[j]) - 128.0f) / 128.0f;
      }
    } else if (input_size == tensor_meta->nbytes()) {
      ET_LOG(Info, "Copying input data to tensor (%zu bytes)", input_size);
      std::memcpy(data_ptr, input_data, input_size);
    } else {
      ET_LOG(
          Error,
          "Input size (%zu) and tensor size (%zu elements, %zu bytes) mismatch!",
          input_size, num_elements,
          tensor_meta->nbytes());
      err = Error::InvalidArgument;
    }

    TensorImpl impl = TensorImpl(
        tensor_meta.get().scalar_type(),
        tensor_meta.get().sizes().size(),
        const_cast<TensorImpl::SizesType*>(tensor_meta.get().sizes().data()),
        data_ptr,
        const_cast<TensorImpl::DimOrderType*>(
            tensor_meta.get().dim_order().data()));
    Tensor t(&impl);

    err = method.set_input(t, i);

    if (err != Error::Ok) {
      ET_LOG(
          Error, "Failed to prepare input %zu: 0x%" PRIx32, i, (uint32_t)err);
      BufferCleanup cleanup({inputs, num_allocated});
      return err;
    }
  }
  return BufferCleanup({inputs, num_allocated});
}

bool verify_output(
    const std::vector<EValue>& outputs,
    const uint8_t* expected_output,
    size_t expected_size) {
  if (outputs.empty() || !outputs[0].isTensor()) {
    ET_LOG(Error, "Output is not a tensor");
    return false;
  }

  Tensor output_tensor = outputs[0].toTensor();
  ScalarType scalar_type = output_tensor.scalar_type();
  size_t num_elements = output_tensor.numel();

  ET_LOG(Info, "Output tensor: scalar_type=%s, numel=%zu",
         executorch::runtime::toString(scalar_type), num_elements);

  if (num_elements != expected_size) {
    ET_LOG(Error, "Output element count mismatch: got %zu, expected %zu",
           num_elements, expected_size);
    return false;
  }

  bool match = true;

  if (scalar_type == ScalarType::Float) {
    const float* float_data = output_tensor.const_data_ptr<float>();
    for (size_t i = 0; i < expected_size; i++) {
      uint8_t quantized = static_cast<uint8_t>(
          std::max(0.0f, std::min(255.0f, float_data[i] * 128.0f + 128.0f)));
      if (quantized != expected_output[i]) {
        ET_LOG(Info, "Output mismatch at [%zu]: got 0x%02x (float %.4f), expected 0x%02x",
               i, quantized, float_data[i], expected_output[i]);
        match = false;
      }
    }
  } else {
    const uint8_t* output_data = output_tensor.const_data_ptr<uint8_t>();
    for (size_t i = 0; i < expected_size; i++) {
      if (output_data[i] != expected_output[i]) {
        ET_LOG(Info, "Output mismatch at index %zu: got 0x%02x, expected 0x%02x",
               i, output_data[i], expected_output[i]);
        match = false;
      }
    }
  }

  return match;
}

int get_top_prediction(const std::vector<EValue>& outputs) {
  if (outputs.empty() || !outputs[0].isTensor()) {
    return -1;
  }

  Tensor output_tensor = outputs[0].toTensor();
  ScalarType scalar_type = output_tensor.scalar_type();
  size_t num_classes = output_tensor.numel();

  int max_idx = 0;

  if (scalar_type == ScalarType::Float) {
    const float* float_data = output_tensor.const_data_ptr<float>();
    float max_val = float_data[0];
    for (size_t i = 1; i < num_classes && i < (size_t)kws_num_labels; i++) {
      if (float_data[i] > max_val) {
        max_val = float_data[i];
        max_idx = i;
      }
    }
  } else {
    const uint8_t* output_data = output_tensor.const_data_ptr<uint8_t>();
    uint8_t max_val = output_data[0];
    for (size_t i = 1; i < num_classes && i < (size_t)kws_num_labels; i++) {
      if (output_data[i] > max_val) {
        max_val = output_data[i];
        max_idx = i;
      }
    }
  }

  return max_idx;
}

} // namespace

int main(void) {
  printk("\n========================================\n");
  printk("ExecutorTorch Keyword Spotting Demo\n");
  printk("========================================\n\n");

#if defined(CONFIG_ARM_ETHOS_U)
  if (executorch_delegate_EthosUBackend_registered() != Error::Ok) {
    ET_LOG(
        Error,
        "Ethos-U backend registration failed; model execution cannot continue");
    return 1;
  }
  ET_LOG(Info, "Ethos-U backend registered successfully");
#endif

  executorch::runtime::runtime_init();

  size_t pte_size = sizeof(model_pte);
  ET_LOG(Info, "Model PTE at %p, Size: %lu bytes", model_pte, pte_size);

  const void* program_data = model_pte;
  size_t program_data_len = pte_size;

  auto loader = BufferDataLoader(program_data, program_data_len);
  ET_LOG(Info, "Model data loaded. Size: %lu bytes.", program_data_len);

  Result<Program> program = Program::load(&loader);
  if (!program.ok()) {
    ET_LOG(
        Error,
        "Program loading failed @ 0x%p: 0x%" PRIx32,
        program_data,
        program.error());
    return 1;
  }

  ET_LOG(Info, "Model loaded, has %zu methods", program->num_methods());

  const char* method_name = nullptr;
  {
    const auto method_name_result = program->get_method_name(0);
    ET_CHECK_MSG(method_name_result.ok(), "Program has no methods");
    method_name = *method_name_result;
  }
  ET_LOG(Info, "Running method: %s", method_name);

  Result<MethodMeta> method_meta = program->method_meta(method_name);
  if (!method_meta.ok()) {
    ET_LOG(
        Error,
        "Failed to get method_meta for %s: 0x%x",
        method_name,
        (unsigned int)method_meta.error());
    return 1;
  }

  ET_LOG(
      Info,
      "Method allocator pool size: %zu bytes",
      method_allocation_pool_size);

  ArmMemoryAllocator method_allocator(
      method_allocation_pool_size, method_allocation_pool);

  std::vector<uint8_t*> planned_buffers;
  std::vector<Span<uint8_t>> planned_spans;
  size_t num_memory_planned_buffers = method_meta->num_memory_planned_buffers();

  for (size_t id = 0; id < num_memory_planned_buffers; ++id) {
    size_t buffer_size =
        static_cast<size_t>(method_meta->memory_planned_buffer_size(id).get());
    ET_LOG(Info, "Setting up planned buffer %zu, size %zu.", id, buffer_size);

    uint8_t* buffer =
        reinterpret_cast<uint8_t*>(method_allocator.allocate(buffer_size));
    ET_CHECK_MSG(
        buffer != nullptr,
        "Could not allocate memory for memory planned buffer size %zu",
        buffer_size);
    planned_buffers.push_back(buffer);
    planned_spans.push_back({planned_buffers.back(), buffer_size});
  }

  HierarchicalAllocator planned_memory(
      {planned_spans.data(), planned_spans.size()});

  ArmMemoryAllocator temp_allocator(
      temp_allocation_pool_size, temp_allocation_pool);

  MemoryManager memory_manager(
      &method_allocator, &planned_memory, &temp_allocator);

  ET_LOG(Info, "Loading method...");
  executorch::runtime::EventTracer* event_tracer_ptr = nullptr;

  Result<Method> method =
      program->load_method(method_name, &memory_manager, event_tracer_ptr);

  if (!method.ok()) {
    ET_LOG(
        Error,
        "Loading of method %s failed with status 0x%" PRIx32,
        method_name,
        method.error());
    return 1;
  }
  ET_LOG(Info, "Method '%s' loaded successfully", method_name);

  ET_LOG(Info, "Preparing input tensor with static KWS data...");
  ET_LOG(Info, "Input data size: %zu bytes", sizeof(kws_input_data));

  {
    static auto prepared_inputs = ::prepare_input_tensors(
        *method, method_allocator, kws_input_data, sizeof(kws_input_data));

    if (!prepared_inputs.ok()) {
      ET_LOG(
          Error,
          "Preparing input tensors failed with status 0x%" PRIx32,
          prepared_inputs.error());
      return 1;
    }
  }
  ET_LOG(Info, "Input prepared successfully");

  ET_LOG(Info, "\n--- Starting inference ---");
  uint32_t start_time = k_uptime_get_32();

  Error status = method->execute();

  uint32_t end_time = k_uptime_get_32();
  uint32_t inference_time = end_time - start_time;

  if (status != Error::Ok) {
    ET_LOG(
        Error,
        "Execution of method %s failed with status 0x%" PRIx32,
        method_name,
        status);
    return 1;
  }
  ET_LOG(Info, "Inference completed in %u ms", inference_time);

  std::vector<EValue> outputs(method->outputs_size());
  status = method->get_outputs(outputs.data(), outputs.size());
  ET_CHECK(status == Error::Ok);

  ET_LOG(Info, "\n--- Inference Results ---");

  int predicted_class = get_top_prediction(outputs);
  if (predicted_class >= 0 && predicted_class < kws_num_labels) {
    ET_LOG(Info, "Predicted keyword: \"%s\" (class %d)",
           kws_labels[predicted_class], predicted_class);
  }

  ET_LOG(Info, "\nOutput tensor values:");
  for (size_t i = 0; i < outputs.size(); ++i) {
    if (!outputs[i].isTensor()) {
      ET_LOG(Info, "  output[%zu]: non-tensor value", i);
      continue;
    }
    Tensor tensor = outputs[i].toTensor();
    ScalarType scalar_type = tensor.scalar_type();
    ET_LOG(
        Info,
        "  output[%zu]: scalar_type=%s numel=%zd",
        i,
        executorch::runtime::toString(scalar_type),
        tensor.numel());

    if (scalar_type == ScalarType::Float) {
      const float* float_data = tensor.const_data_ptr<float>();
      for (ssize_t j = 0; j < tensor.numel() && j < kws_num_labels; ++j) {
        uint8_t quantized = static_cast<uint8_t>(
            std::max(0.0f, std::min(255.0f, float_data[j] * 128.0f + 128.0f)));
        ET_LOG(Info, "    [%zd] %s: %.4f (q: 0x%02x)",
               j, kws_labels[j], float_data[j], quantized);
      }
    } else {
      const uint8_t* data = tensor.const_data_ptr<uint8_t>();
      for (ssize_t j = 0; j < tensor.numel() && j < kws_num_labels; ++j) {
        ET_LOG(Info, "    [%zd] %s: 0x%02x (%d)",
               j, kws_labels[j], data[j], data[j]);
      }
    }
  }

  ET_LOG(Info, "\n--- Verification ---");
#if SKIP_OUTPUT_VERIFICATION
  bool output_match = true;
  if (!outputs.empty() && outputs[0].isTensor()) {
    Tensor out_tensor = outputs[0].toTensor();
    if (out_tensor.numel() == KWS_NUM_OUTPUT_CLASSES) {
      ET_LOG(Info, "SUCCESS: Output shape verified (12 classes)");
      ET_LOG(Info, "(Value verification skipped - using untrained model)");
    } else {
      ET_LOG(Error, "FAIL: Output shape mismatch (expected %d, got %zd)",
             KWS_NUM_OUTPUT_CLASSES, out_tensor.numel());
      output_match = false;
    }
  } else {
    ET_LOG(Error, "FAIL: No tensor output");
    output_match = false;
  }
#else
  bool output_match = verify_output(outputs, kws_expected_output, sizeof(kws_expected_output));
  if (output_match) {
    ET_LOG(Info, "SUCCESS: Output matches expected values!");
  } else {
    ET_LOG(Info, "WARNING: Output differs from expected values");
    ET_LOG(Info, "Expected output:");
    for (size_t i = 0; i < sizeof(kws_expected_output); i++) {
      ET_LOG(Info, "  [%zu]: 0x%02x", i, kws_expected_output[i]);
    }
  }
#endif

  ET_LOG(Info, "\n========================================");
  ET_LOG(Info, "Keyword Spotting Demo Complete");
  ET_LOG(Info, "Inference time: %u ms", inference_time);
  ET_LOG(Info, "Result: %s", output_match ? "PASS" : "FAIL");
  ET_LOG(Info, "========================================\n");

  return output_match ? 0 : 1;
}
