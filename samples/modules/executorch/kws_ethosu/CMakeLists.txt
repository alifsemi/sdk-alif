# Copyright 2026 Alif Semiconductor
# Copyright (c) Meta Platforms, Inc. and affiliates.
# Copyright 2025 Arm Limited and/or its affiliates.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20)

set(CMAKE_SKIP_INSTALL_RULES ON CACHE BOOL "" FORCE)

set(ET_PTE_FILE_PATH
    ""
    CACHE FILEPATH "Path to the ExecuTorch .pte model for KWS"
)
set(ET_PTE_SECTION
    ".rodata.model"
    CACHE STRING "Section attribute used for the generated model data"
)

if(NOT ET_PTE_FILE_PATH)
  message(
    FATAL_ERROR
      "ET_PTE_FILE_PATH must point to the ExecuTorch .pte model.\n"
      "Example: -DET_PTE_FILE_PATH=./kws_model.pte"
  )
endif()

if(NOT IS_ABSOLUTE "${ET_PTE_FILE_PATH}")
  get_filename_component(
    ET_PTE_FILE_PATH "${ET_PTE_FILE_PATH}" ABSOLUTE BASE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}"
  )
endif()

if(NOT EXISTS "${ET_PTE_FILE_PATH}")
  message(
    FATAL_ERROR
      "Could not find ExecuTorch model at ET_PTE_FILE_PATH: ${ET_PTE_FILE_PATH}"
  )
endif()

set(ET_PTE_FILE_PATH
    "${ET_PTE_FILE_PATH}"
    CACHE FILEPATH "Path to the ExecuTorch .pte model"
    FORCE
)

# Detect NPU architecture from ETHOSU_TARGET_NPU_CONFIG
if(ETHOSU_TARGET_NPU_CONFIG MATCHES "^ethos-(u[0-9]+)-([0-9]+)$")
    set(ETHOSU_ARCH "${CMAKE_MATCH_1}")
    set(ETHOSU_MACS "${CMAKE_MATCH_2}")

    message(STATUS "ETHOSU_TARGET_NPU_CONFIG: ${ETHOSU_TARGET_NPU_CONFIG}")
    message(STATUS "Detected NPU Architecture: ${ETHOSU_ARCH}, MACs: ${ETHOSU_MACS}")

    if((ETHOSU_ARCH STREQUAL "u55") OR (ETHOSU_ARCH STREQUAL "u65"))
        set(ETHOSU_ARCH_U55 y CACHE BOOL "U55 Flag")
        add_compile_definitions(ETHOSU_ARCH_U55="${ETHOSU_ARCH_U55}")

        if(ETHOSU_MACS STREQUAL "256")
            set(CONFIG_ARM_ETHOS_U55_256 y CACHE BOOL "U55 256 MACs" FORCE)
            message(STATUS "Auto-configured: CONFIG_ARM_ETHOS_U55_256=y")
        elseif(ETHOSU_MACS STREQUAL "128")
            set(CONFIG_ARM_ETHOS_U55_128 y CACHE BOOL "U55 128 MACs" FORCE)
            message(STATUS "Auto-configured: CONFIG_ARM_ETHOS_U55_128=y")
        endif()

        message(STATUS "Building for Ethos-U55 with ${ETHOSU_MACS} MACs")

    elseif(ETHOSU_ARCH STREQUAL "u85")
        set(ETHOSU_ARCH_U85 y CACHE BOOL "U85 Flag")
        add_compile_definitions(ETHOSU_ARCH_U85="${ETHOSU_ARCH_U85}")

        set(CONFIG_ARM_ETHOS_U85_256 y CACHE BOOL "U85 256 MACs" FORCE)
        message(STATUS "Auto-configured: CONFIG_ARM_ETHOS_U85_256=y")
        message(STATUS "Building for Ethos-U85 with ${ETHOSU_MACS} MACs")
    else()
        message(FATAL_ERROR "Unsupported NPU architecture: ${ETHOSU_ARCH}. Valid: u55, u65, u85")
    endif()
else()
    message(FATAL_ERROR
        "Invalid or missing ETHOSU_TARGET_NPU_CONFIG: '${ETHOSU_TARGET_NPU_CONFIG}'\n"
        "Required format: ethos-u<arch>-<macs>\n"
        "Examples:\n"
        "  -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128\n"
        "  -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256\n"
        "  -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256")
endif()

set(CONF_FILE "${CMAKE_CURRENT_SOURCE_DIR}/prj.conf")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(executorch_kws_ethosu)

# Core-type checks that need Kconfig, so they have to run after find_package().
# The CONFIG_ARM_ETHOS_U* forcing above must stay before it, since Zephyr turns
# CONFIG_-prefixed CMake cache variables into Kconfig assignments while
# configuring.
if(CONFIG_APSS)
  if(ETHOSU_ARCH STREQUAL "u55" OR ETHOSU_ARCH STREQUAL "u65")
    message(FATAL_ERROR
        "Ethos-U55/U65 is not reachable from APSS (Cortex-A32).\n"
        "Use Ethos-U85 instead: -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256")
  endif()

  # Only E8 pairs an APSS core with an Ethos-U85, so it is the only board this
  # combination can target. Checked before the SRAM1 check below, which would
  # otherwise point at a snippet that does not cover this board.
  if(NOT CONFIG_SOC_SERIES_E8)
    message(FATAL_ERROR
        "Ethos-U85 on APSS (Cortex-A32) is only available on the E8 boards.\n"
        "This board's APSS core has no U85 to drive.")
  endif()

  # The NPU and the non-cacheable SRAM1 region both come from the
  # ethos-u85-apss-enable snippet; without it there is no ethosu1 node to bind
  # the driver to and no SRAM1 region for GROUP_LINK_IN() below to target.
  dt_nodelabel(_apss_sram1 NODELABEL "sram1")
  if(NOT _apss_sram1)
    message(FATAL_ERROR
        "APSS build is missing the SRAM1 memory region.\n"
        "Add the APSS NPU snippet: -S ethos-u85-apss-enable")
  endif()
endif()

find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Detect the correct Python executable for executorch scripts.
# Priority: 1) VIRTUAL_ENV/bin/python3 (if venv is active)
#           2) Python3_EXECUTABLE from find_package
# Note: WEST_PYTHON is unreliable as it reflects where west was installed,
# not the currently active virtual environment.
if(DEFINED ENV{VIRTUAL_ENV} AND EXISTS "$ENV{VIRTUAL_ENV}/bin/python3")
  set(ET_PYTHON_EXECUTABLE "$ENV{VIRTUAL_ENV}/bin/python3")
  message(STATUS "Using venv Python for executorch scripts: ${ET_PYTHON_EXECUTABLE}")
else()
  set(ET_PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
  message(STATUS "Using system Python for executorch scripts: ${ET_PYTHON_EXECUTABLE}")
  message(WARNING "No virtual environment detected. If executorch module is not found, "
                  "ensure you activate the venv before running west build.")
endif()

# ExecuTorch's codegen helpers (tools/cmake/Codegen.cmake) hardcode
# ${PYTHON_EXECUTABLE}, which Zephyr sets from find_package(Python3). That can
# resolve to a bare system interpreter even with the venv active -- this venv's
# bin/python3 is a symlink to /usr/bin/python3, and a venv is only entered when
# the interpreter is invoked *through* its own bin directory. The system
# interpreter cannot "import executorch", so gen_oplist fails at build time
# rather than configure time. Point PYTHON_EXECUTABLE at the interpreter
# resolved above for the rest of this file; Zephyr's own build steps are
# unaffected, because kernel.cmake already added ${ZEPHYR_BASE} as a
# subdirectory from inside find_package(Zephyr), capturing its own value.
set(PYTHON_EXECUTABLE "${ET_PYTHON_EXECUTABLE}")

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wall -Wno-switch -Wno-float-conversion -Wno-double-promotion -ffunction-sections -fdata-sections"
)

# Check for operators in the model
execute_process(
  COMMAND
    ${ET_PYTHON_EXECUTABLE} "${ZEPHYR_EXECUTORCH_MODULE_DIR}/codegen/tools/gen_oplist.py"
    --model_file_path=${ET_PTE_FILE_PATH}
    --output_path=${CMAKE_CURRENT_BINARY_DIR}/temp.yaml
  OUTPUT_VARIABLE CMD_RESULT
  RESULT_VARIABLE CMD_RESULT_CODE
  ERROR_VARIABLE CMD_ERROR
)

if(CMD_RESULT_CODE AND NOT CMD_RESULT_CODE EQUAL 0)
  # By far the most common cause, so name it rather than only dumping the
  # traceback: the same failure would otherwise reappear later inside codegen.
  if(CMD_ERROR MATCHES "No module named 'executorch'")
    message(FATAL_ERROR
        "'${ET_PYTHON_EXECUTABLE}' cannot import the executorch Python "
        "package, which this sample needs to read the operator list out of "
        "the .pte.\n"
        "Activate the workspace virtual environment before building:\n"
        "  source <workspace>/.zephyr_venv/bin/activate")
  endif()
  message(FATAL_ERROR "Failed to generate operator list: ${CMD_ERROR}")
endif()

if(CMD_RESULT MATCHES "aten::" OR CMD_RESULT MATCHES "dim_order_ops::")
  set(FOUND_OPS_IN_FILE "true")
else()
  set(FOUND_OPS_IN_FILE "false")
endif()

if(NOT DEFINED EXECUTORCH_DIR)
  if(DEFINED ZEPHYR_EXECUTORCH_MODULE_DIR)
    set(EXECUTORCH_DIR ${ZEPHYR_EXECUTORCH_MODULE_DIR})
    message(STATUS "Using Zephyr module: EXECUTORCH_DIR=${EXECUTORCH_DIR}")
  else()
    message(FATAL_ERROR "executorch module not found.")
  endif()
endif()

set(EXECUTORCH_ROOT ${EXECUTORCH_DIR})

if(NOT TARGET portable_kernels)
  set(EXECUTORCH_PORTABLE_BUILD_KERNELS_ONLY ON)
  add_subdirectory(
    ${EXECUTORCH_DIR}/kernels/portable
    ${CMAKE_CURRENT_BINARY_DIR}/executorch/kernels/portable
  )
  unset(EXECUTORCH_PORTABLE_BUILD_KERNELS_ONLY)
endif()

set(EXECUTORCH_OPS_LIB "")
if(${FOUND_OPS_IN_FILE})
  include(${EXECUTORCH_DIR}/tools/cmake/Utils.cmake)
  include(${EXECUTORCH_DIR}/tools/cmake/Codegen.cmake)
  if(NOT DEFINED EXECUTORCH_ENABLE_DTYPE_SELECTIVE_BUILD)
    set(EXECUTORCH_ENABLE_DTYPE_SELECTIVE_BUILD "")
  endif()
  gen_selected_ops(
    LIB_NAME "cpu_portable_ops_lib"
    OPS_SCHEMA_YAML ""
    ROOT_OPS ""
    INCLUDE_ALL_OPS ""
    OPS_FROM_MODEL "${ET_PTE_FILE_PATH}"
    DTYPE_SELECTIVE_BUILD "${EXECUTORCH_ENABLE_DTYPE_SELECTIVE_BUILD}"
  )
  generate_bindings_for_kernels(
    LIB_NAME "cpu_portable_ops_lib"
    FUNCTIONS_YAML ${EXECUTORCH_DIR}/kernels/portable/functions.yaml
    DTYPE_SELECTIVE_BUILD "${EXECUTORCH_ENABLE_DTYPE_SELECTIVE_BUILD}"
  )
  gen_operators_lib(
    LIB_NAME "cpu_portable_ops_lib"
    KERNEL_LIBS portable_kernels
    DEPS executorch
    DTYPE_SELECTIVE_BUILD "${EXECUTORCH_ENABLE_DTYPE_SELECTIVE_BUILD}"
  )
  set(EXECUTORCH_OPS_LIB "cpu_portable_ops_lib")
endif()

set(_local_flatcc_root ${CMAKE_BINARY_DIR}/flatcc_src)
if(NOT EXISTS ${_local_flatcc_root}/CMakeLists.txt)
  file(MAKE_DIRECTORY ${_local_flatcc_root})
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${EXECUTORCH_DIR}/third-party/flatcc ${_local_flatcc_root}
  )
endif()
set(EXECUTORCH_FLATCC_SOURCE_ROOT ${_local_flatcc_root} CACHE PATH "" FORCE)
set(EXECUTORCH_FLATCC_INSTALL_ROOT ${_local_flatcc_root} CACHE PATH "" FORCE)

set(app_sources
    src/main.cpp
    ${EXECUTORCH_DIR}/examples/arm/executor_runner/arm_memory_allocator.cpp
)
target_sources(app PRIVATE ${app_sources})

set(_model_pte_header ${CMAKE_CURRENT_BINARY_DIR}/model_pte.h)
add_custom_command(
  OUTPUT ${_model_pte_header}
  COMMAND
    ${ET_PYTHON_EXECUTABLE}
    ${EXECUTORCH_DIR}/examples/arm/executor_runner/pte_to_header.py
    --pte ${ET_PTE_FILE_PATH}
    --outdir ${CMAKE_CURRENT_BINARY_DIR}
    --section ${ET_PTE_SECTION}
  DEPENDS ${ET_PTE_FILE_PATH}
          ${EXECUTORCH_DIR}/examples/arm/executor_runner/pte_to_header.py
  COMMENT "Converting ${ET_PTE_FILE_PATH} to model_pte.h"
)
add_custom_target(gen_model_header DEPENDS ${_model_pte_header})
add_dependencies(app gen_model_header)

# Memory pool sizes — Kconfig is the single source of truth.
# NPU scratch (temp pool) is always placed in DTCM for all boards (E7, E8, E1C, B1).
# Method pool uses .alif_sram0 on boards that have it; see main.cpp for details.
target_compile_definitions(
  app PRIVATE
  ET_ARM_METHOD_ALLOCATOR_POOL_SIZE=${CONFIG_EXECUTORCH_METHOD_ALLOCATOR_POOL_SIZE}
  ET_ARM_BAREMETAL_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE=${CONFIG_EXECUTORCH_TEMP_ALLOCATOR_POOL_SIZE}
)

message(STATUS "ExecutorTorch: Method pool: ${CONFIG_EXECUTORCH_METHOD_ALLOCATOR_POOL_SIZE} bytes")
message(STATUS "ExecutorTorch: Temp pool:   ${CONFIG_EXECUTORCH_TEMP_ALLOCATOR_POOL_SIZE} bytes")

target_link_libraries(app PRIVATE libexecutorch)
if(EXECUTORCH_OPS_LIB)
  target_link_libraries(app PRIVATE ${EXECUTORCH_OPS_LIB})
endif()

# The cortex_m operators the model calls.
#
# aot_arm_compiler applies ReplaceQuantNodesPass to every Ethos-U export, so the
# int8 boundary around the delegated subgraph is always expressed as
# cortex_m::quantize_per_tensor.out / cortex_m::dequantize_per_tensor.out rather
# than the quantized_decomposed equivalents. Those kernels must be registered or
# Method::load() aborts with "Missing operator" (Error::NotFound, 0x14) -- the
# .pte itself is core-agnostic, only the kernel registry is not.
#
# On RTSS the executorch Zephyr module builds the whole cortex_m backend, so the
# kernels come from there. It gates that on CONFIG_CPU_CORTEX_M because most of
# the backend's kernels are CMSIS-NN wrappers, and CMSIS-NN is Cortex-M only.
# quantize/dequantize_per_tensor are the two exceptions: they are plain C++ that
# includes nothing beyond kernel_includes.h, with an MVE fast path behind
# __ARM_FEATURE_MVE and a scalar fallback for everything else. So on APSS
# (Cortex-A32) we compile just those two and generate a registration lib for
# them, which keeps one .pte working across both core types.
if(TARGET cortex_m_ops_lib)
  target_link_libraries(app PRIVATE cortex_m_ops_lib)
  if(TARGET cortex_m_kernels)
    target_link_libraries(app PRIVATE cortex_m_kernels)
  endif()
elseif(CMD_RESULT MATCHES "cortex_m::")
  # Anything outside the CMSIS-NN-free pair cannot be served here. Catch it now
  # rather than at method load on the target.
  set(_cortex_m_standalone_ops "cortex_m::quantize_per_tensor.out"
                               "cortex_m::dequantize_per_tensor.out"
  )
  string(REGEX MATCHALL "cortex_m::[A-Za-z0-9_]+\\.[A-Za-z0-9_]+" _model_cortex_m_ops
         "${CMD_RESULT}"
  )
  list(REMOVE_DUPLICATES _model_cortex_m_ops)
  foreach(_op IN LISTS _model_cortex_m_ops)
    if(NOT _op IN_LIST _cortex_m_standalone_ops)
      message(FATAL_ERROR
          "The model needs the operator '${_op}', which the cortex_m backend "
          "implements on top of CMSIS-NN and is therefore available on the "
          "RTSS (Cortex-M) cores only.\n"
          "Re-export the model so that this operator is delegated to the "
          "Ethos-U, or run it on an RTSS core.")
    endif()
  endforeach()

  include(${EXECUTORCH_DIR}/tools/cmake/Utils.cmake)
  include(${EXECUTORCH_DIR}/tools/cmake/Codegen.cmake)

  # The kernel sources are taken from the backend as-is; only the CMSIS-NN
  # dependency is dropped. Compiled with zephyr_interface so they pick up this
  # core's -mcpu/ABI flags and autoconf.h.
  add_library(cortex_m_standalone_kernels
              ${EXECUTORCH_DIR}/backends/cortex_m/ops/op_quantize_per_tensor.cpp
              ${EXECUTORCH_DIR}/backends/cortex_m/ops/op_dequantize_per_tensor.cpp
  )
  get_filename_component(_executorch_parent_dir ${EXECUTORCH_DIR} DIRECTORY)
  target_include_directories(
    cortex_m_standalone_kernels
    PRIVATE ${_executorch_parent_dir}
            ${EXECUTORCH_DIR}/runtime/core/portable_type/c10
  )
  # kernels_util_all_deps provides resize_tensor(), used by both kernels.
  target_link_libraries(
    cortex_m_standalone_kernels PRIVATE zephyr_interface executorch
                                        kernels_util_all_deps
  )

  # Select from the model, so only the operators it actually calls get
  # registered, while the schema still comes from the backend's own yaml.
  gen_selected_ops(
    LIB_NAME "cortex_m_standalone_ops_lib"
    OPS_SCHEMA_YAML ""
    ROOT_OPS ""
    INCLUDE_ALL_OPS ""
    OPS_FROM_MODEL "${ET_PTE_FILE_PATH}"
    DTYPE_SELECTIVE_BUILD ""
  )
  generate_bindings_for_kernels(
    LIB_NAME "cortex_m_standalone_ops_lib"
    CUSTOM_OPS_YAML ${EXECUTORCH_DIR}/backends/cortex_m/ops/operators.yaml
    DTYPE_SELECTIVE_BUILD ""
  )
  gen_operators_lib(
    LIB_NAME "cortex_m_standalone_ops_lib" KERNEL_LIBS
    cortex_m_standalone_kernels DEPS executorch
  )
  target_link_libraries(cortex_m_standalone_ops_lib PRIVATE zephyr_interface)
  target_link_libraries(app PRIVATE cortex_m_standalone_ops_lib)

  message(STATUS
      "ExecuTorch: registering cortex_m quantize/dequantize kernels locally "
      "(cortex_m backend not built for this core)")
endif()
if(TARGET executorch_delegate_ethos_u)
  target_link_libraries(app PRIVATE executorch_delegate_ethos_u)
endif()
if(TARGET ethosu_core_driver)
  target_link_libraries(app PRIVATE ethosu_core_driver)
endif()

target_include_directories(app PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})

# APSS (Cortex-A32): add the SRAM1 output section that holds the NPU-visible
# buffers. SRAM1 is non-cacheable in the MMU, which is what keeps the CPU and
# the NPU coherent on this core; see src/main.cpp and the snippet itself.
if(CONFIG_APSS)
  zephyr_linker_sources(SECTIONS boards/apss_sram1_sections.ld)
endif()

message(STATUS "executorch KWS application configured successfully")
message(STATUS "  Model: ${ET_PTE_FILE_PATH}")
message(STATUS "  NPU: ${ETHOSU_TARGET_NPU_CONFIG}")
