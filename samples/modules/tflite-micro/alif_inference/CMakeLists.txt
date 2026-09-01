cmake_minimum_required(VERSION 3.20.0)

# Locate the alif-mlek west module. It provides the Vela-compiled models and the
# source-generator CMake helpers used to turn a .tflite into a C array. Ask west
# for the module path instead of hardcoding the workspace layout.
execute_process(
    COMMAND west list --format "{abspath}" alif-mlek
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE MLEK_ROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE MLEK_LIST_RESULT
    ERROR_QUIET
)
if (MLEK_LIST_RESULT OR NOT EXISTS ${MLEK_ROOT})
    message(FATAL_ERROR
        "alif-mlek module not found. Enable and fetch it first:\n"
        "  west config manifest.project-filter -- +alif-mlek\n"
        "  west update\n")
endif()

# Make the MLEK source-generator helpers (setup_source_generator,
# generate_model_code) available without pulling in the full MLEK examples lib.
set(MLEK_SCRIPTS_DIR ${MLEK_ROOT}/scripts)
list(APPEND CMAKE_MODULE_PATH ${MLEK_SCRIPTS_DIR}/cmake)
include(source_gen_utils)

# ---------------------------------------------------------------------------
# Ethos-U NPU selection
# ---------------------------------------------------------------------------
# When ETHOSU_TARGET_NPU_CONFIG is provided on the command line it is the single
# source of truth for the NPU. We must act on it BEFORE find_package(Zephyr) so
# that the matching Kconfig choice (CONFIG_ARM_ETHOS_U<arch>_<macs>) is fed into
# the build and the Ethos-U driver targets the requested NPU. This overrides the
# board default (E8 rtss_hp defaults to Ethos-U55 with 256 MACs).
#
# The parsed value is also used to pick the correct Vela-compiled model below.
# Vela config-ID prefix: U55 -> H, U65 -> Y, U85 -> Z (e.g. ethos-u85-256 -> Z256).
if (DEFINED ETHOSU_TARGET_NPU_CONFIG)
    string(TOLOWER "${ETHOSU_TARGET_NPU_CONFIG}" _npu_cfg)
    if (_npu_cfg MATCHES "^ethos-(u[0-9]+)-([0-9]+)$")
        set(_npu_arch "${CMAKE_MATCH_1}")
        set(_npu_macs "${CMAKE_MATCH_2}")
        if (_npu_arch STREQUAL "u55")
            set(_npu_prefix "H")
        elseif (_npu_arch STREQUAL "u65")
            set(_npu_prefix "Y")
        elseif (_npu_arch STREQUAL "u85")
            set(_npu_prefix "Z")
        else()
            message(FATAL_ERROR
                "Unsupported Ethos-U architecture '${_npu_arch}' in "
                "ETHOSU_TARGET_NPU_CONFIG='${ETHOSU_TARGET_NPU_CONFIG}'")
        endif()
        set(ETHOS_U_NPU_CONFIG_ID "${_npu_prefix}${_npu_macs}")

        # Feed the matching Kconfig choice into the Zephyr configuration.
        string(TOUPPER "${_npu_arch}" _npu_arch_uc)
        set(CONFIG_ARM_ETHOS_${_npu_arch_uc}_${_npu_macs} y
            CACHE INTERNAL "Selected Ethos-U NPU configuration" FORCE)
        message(STATUS
            "Ethos-U: ETHOSU_TARGET_NPU_CONFIG=${ETHOSU_TARGET_NPU_CONFIG} -> "
            "CONFIG_ARM_ETHOS_${_npu_arch_uc}_${_npu_macs}=y, model ID ${ETHOS_U_NPU_CONFIG_ID}")
    else()
        message(FATAL_ERROR
            "Invalid ETHOSU_TARGET_NPU_CONFIG='${ETHOSU_TARGET_NPU_CONFIG}'.\n"
            "Expected format: ethos-u<arch>-<macs>, e.g. ethos-u55-256 or ethos-u85-256")
    endif()
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_inference)

# ---------------------------------------------------------------------------
# Build-time model generation from MLEK resources
# ---------------------------------------------------------------------------
set(RESOURCES_PATH ${MLEK_ROOT}/resources_downloaded CACHE PATH
    "Path to MLEK resources (models compiled with Vela)")
setup_source_generator()

# Source generation output directory
set(SRC_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/src)
file(MAKE_DIRECTORY ${SRC_GEN_DIR})

# Determine NPU config ID based on Zephyr Kconfig.
# If ETHOSU_TARGET_NPU_CONFIG was supplied on the command line, ETHOS_U_NPU_CONFIG_ID
# has already been computed above. Otherwise fall back to the board Kconfig defaults.
if (NOT DEFINED ETHOS_U_NPU_CONFIG_ID)
    if (CONFIG_ARM_ETHOS_U85_256)
        set(ETHOS_U_NPU_CONFIG_ID "Z256")
    elseif (CONFIG_ARM_ETHOS_U55_256)
        set(ETHOS_U_NPU_CONFIG_ID "H256")
    else()
        set(ETHOS_U_NPU_CONFIG_ID "H128")
    endif()
endif()

# Model file path
set(KWS_MODEL_PATH ${RESOURCES_PATH}/kws/kws_micronet_m_vela_${ETHOS_U_NPU_CONFIG_ID}.tflite
    CACHE FILEPATH "Path to the KWS TFLite model (Vela-compiled)")

if (NOT EXISTS ${KWS_MODEL_PATH})
    message(FATAL_ERROR
        "KWS model not found at ${KWS_MODEL_PATH}\n"
        "Run set_up_default_resources.py first:\n"
        "  python3 ${MLEK_ROOT}/set_up_default_resources.py\n")
endif()

# Validate the generated model C array against the selected NPU.
set(ETHOS_U_NPU_ENABLED ON)

# Generate model C array and parameter constants
set(EXTRA_MODEL_CODE
    "/* Model parameters for alif_inference */"
    "extern const int   g_FrameLength    = 640"
    "extern const int   g_FrameStride    = 320"
    "extern const int   g_AudioRate      = 16000"
    "extern const float g_ScoreThreshold = 0.5"
)

generate_model_code(
    MODEL_PATH  ${KWS_MODEL_PATH}
    DESTINATION ${SRC_GEN_DIR}
    EXPRESSIONS ${EXTRA_MODEL_CODE}
    NAMESPACE   "arm" "app" "kws"
)

# Collect the generated source files
file(GLOB KWS_GENERATED_SOURCES ${SRC_GEN_DIR}/*.cc)

# ---------------------------------------------------------------------------

target_include_directories(app PRIVATE
    src
    ../../alif_common
    ../../alif_common/audio
    ../../alif_common/audio/include
)

target_sources(app PRIVATE
    src/mfcc/PlatformMath.cc
    src/mfcc/KwsProcessing.cc
    src/mfcc/Mfcc.cc
    src/main.cpp
    src/KWSModel.cpp
    src/LiveMicInput.cpp
    ../../alif_common/audio/AudioBackend.cpp
    ${KWS_GENERATED_SOURCES}
)
