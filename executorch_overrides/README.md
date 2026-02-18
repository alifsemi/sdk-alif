# ExecutorTorch Alif Overrides

This directory contains Alif-specific modifications to ExecutorTorch files that override the upstream versions during the build process.

## Why Override Instead of Patch?

The override approach is more robust than patches because:

- **Upstream-resilient**: Works regardless of upstream ExecutorTorch changes
- **No patch conflicts**: Files are simply replaced, no line-number dependencies
- **Easier maintenance**: Modified files are directly visible and editable
- **Version control friendly**: All Alif changes tracked in this directory
- **Simpler workflow**: No patch application/revert cycles needed

## Directory Structure

```
executorch_overrides/
├── examples/
│   ├── arm/zephyr/
│   │   ├── src/
│   │   │   └── arm_executor_runner.cpp  # Alif memory sections & CONFIG_ARM_ETHOS_U
│   │   └── prj.conf                      # Alif-specific Zephyr configuration
│   └── models/
│       └── __init__.py                   # KWS model registration
└── zephyr/
    └── CMakeLists.txt                    # CONFIG_ARM_ETHOS_U build logic
```

## Modified Files

### examples/arm/zephyr/src/arm_executor_runner.cpp

**Alif-specific changes:**
- Memory sections changed to `.alif_sram0.tensor_arena` and `.alif_sram0.ethosu_scratch`
- Updated `CONFIG_ETHOS_U` to `CONFIG_ARM_ETHOS_U`
- Added `MODEL_IN_RAM` conditional compilation guards
- Changed `main()` signature to `main(void)` for Zephyr compatibility

### examples/arm/zephyr/prj.conf

**Alif-specific changes:**
- Updated `CONFIG_ETHOS_U` to `CONFIG_ARM_ETHOS_U`
- Added `CONFIG_ARM_ETHOS_U_LOG_LEVEL_DBG=y`
- Increased `CONFIG_EXECUTORCH_METHOD_ALLOCATOR_POOL_SIZE` to 1.5MB

### examples/models/__init__.py

**Alif-specific changes:**
- Added `Kws = "kws"` to Model enum
- Added KWS model registration to `MODEL_NAME_TO_MODEL`

### zephyr/CMakeLists.txt

**Alif-specific changes:**
- Updated `CONFIG_ETHOS_U` check to `CONFIG_ARM_ETHOS_U`

## How Overrides Are Applied

Overrides are **automatically applied** during `west executorch-setup`:

1. The command runs git submodule initialization
2. Executes `install_executorch.sh` and `examples/arm/setup.sh`
3. **Applies Alif overrides** via `apply_executorch_overrides.py`:
   - Copies files from `alif/executorch_overrides/` to `modules/lib/executorch/`
   - Copies KWS model files from `alif/models/kws/` to `modules/lib/executorch/examples/models/kws/`
4. Overrides persist until `west update` is run

## Setup Command

To set up ExecutorTorch with Alif overrides:

```bash
west executorch-setup
```

This single command handles all initialization and applies the overrides automatically.

## Manual Application

To manually apply overrides without running the full setup:

```bash
python3 alif/scripts/apply_executorch_overrides.py
```

## Reverting Overrides

To revert to upstream ExecutorTorch files:

```bash
cd modules/lib/executorch
git restore examples/arm/zephyr/src/arm_executor_runner.cpp
git restore examples/arm/zephyr/prj.conf
git restore examples/models/__init__.py
git restore zephyr/CMakeLists.txt
rm -rf examples/models/kws/
```

## After Upstream Updates

When you update the ExecutorTorch module via `west update`:

1. Upstream changes will overwrite the overrides
2. Simply re-run the setup command to reapply overrides:
   ```bash
   west executorch-setup
   ```
   Or manually apply just the overrides:
   ```bash
   python3 alif/scripts/apply_executorch_overrides.py
   ```
3. If upstream changes conflict with Alif modifications, you'll need to manually merge changes in this directory

## Updating Override Files

To update an override file:

1. Edit the file in `alif/executorch_overrides/`
2. Re-run the override script to apply changes:
   ```bash
   python3 alif/scripts/apply_executorch_overrides.py
   ```

## Tracking Upstream Changes

To see what changed in upstream ExecutorTorch:

```bash
cd modules/lib/executorch
git log --oneline -20
git diff <old-commit> <new-commit> -- examples/arm/zephyr/src/arm_executor_runner.cpp
```

Then manually merge relevant changes into the override files in this directory.
