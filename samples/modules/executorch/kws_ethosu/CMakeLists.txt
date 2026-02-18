# Copyright 2025 Alif Semiconductor
# Copyright (c) Meta Platforms, Inc. and affiliates.
# Copyright 2025 Arm Limited and/or its affiliates.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.24)

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

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wall -Wno-switch -Wno-float-conversion -Wno-double-promotion -ffunction-sections -fdata-sections"
)

# Check for operators in the model
execute_process(
  COMMAND
    python "${ZEPHYR_EXECUTORCH_MODULE_DIR}/codegen/tools/gen_oplist.py"
    --model_file_path=${ET_PTE_FILE_PATH}
    --output_path=${CMAKE_CURRENT_BINARY_DIR}/temp.yaml
  OUTPUT_VARIABLE CMD_RESULT
)

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
    message(FATAL_ERROR "ExecutorTorch module not found.")
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
    ${Python3_EXECUTABLE}
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

if(DEFINED CONFIG_EXECUTORCH_METHOD_ALLOCATOR_POOL_SIZE)
  target_compile_definitions(
    app PRIVATE
    ET_ARM_METHOD_ALLOCATOR_POOL_SIZE=${CONFIG_EXECUTORCH_METHOD_ALLOCATOR_POOL_SIZE}
  )
endif()
if(DEFINED CONFIG_EXECUTORCH_TEMP_ALLOCATOR_POOL_SIZE)
  target_compile_definitions(
    app PRIVATE
    ET_ARM_BAREMETAL_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE=${CONFIG_EXECUTORCH_TEMP_ALLOCATOR_POOL_SIZE}
  )
endif()

target_link_libraries(app PRIVATE libexecutorch)
if(EXECUTORCH_OPS_LIB)
  target_link_libraries(app PRIVATE ${EXECUTORCH_OPS_LIB})
endif()
if(CONFIG_CPU_CORTEX_M)
  if(TARGET cortex_m_ops_lib)
    target_link_libraries(app PRIVATE cortex_m_ops_lib)
  endif()
  if(TARGET cortex_m_kernels)
    target_link_libraries(app PRIVATE cortex_m_kernels)
  endif()
endif()
if(TARGET executorch_delegate_ethos_u)
  target_link_libraries(app PRIVATE executorch_delegate_ethos_u)
endif()
if(TARGET ethosu_core_driver)
  target_link_libraries(app PRIVATE ethosu_core_driver)
endif()

target_include_directories(app PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "ExecutorTorch KWS application configured successfully")
message(STATUS "  Model: ${ET_PTE_FILE_PATH}")
message(STATUS "  NPU: ${ETHOSU_TARGET_NPU_CONFIG}")
