# executorch Alif Overrides

This directory contains Alif-specific modifications to executorch files that override the upstream versions during the build process.

## Why Override Instead of Patch?

The override approach is more robust than patches because:

- **Minor upstream changes**: Works regardless of minor upstream executorch changes
- **No patch conflicts**: Files are simply replaced, no line-number dependencies
- **Easier maintenance**: Modified files are directly visible and editable
- **Version control friendly**: All Alif changes tracked in this directory
- **Simpler workflow**: No patch application/revert cycles needed

**Note:** Major API changes in upstream executorch may require updating these
override files. If upstream significantly changes the structure or interfaces of
the overridden files, manual merging will be necessary.

## PyTorch version handling (IMPORTANT)

The setup **no longer rewrites** ExecuTorch's `torch_pin.py` /
`install_requirements.py` to chase the *latest* PyTorch nightly. That
approach was the root cause of recurring breakage:

- PyTorch purges nightly wheels from its index after a few weeks, so a
  previously-resolved nightly would disappear; and
- the "latest" nightly drifts away from the exact build the pinned
  ExecuTorch *source* was written against, producing ABI / op-schema
  errors such as
  `The underlying op of 'aten.transpose' has no overload name 'Dimname'`.

ExecuTorch source and torch must move **together**. The module is pinned
(`submanifests/executorch.yaml`) to a commit that targets a **stable**
PyTorch release (`torch==2.12.0`), which is permanent on the index. Its own
`install_requirements.py` installs the matching
`torch` / `torchvision` / `torchaudio`, so the pairing never goes stale.

## What is (and is not) overridden

Alif's KWS application (`samples/modules/executorch/kws_ethosu/`) is a
**fully custom Zephyr app** with its own `src/main.cpp` and `prj.conf`.
It does **not** use ExecuTorch's upstream `arm_executor_runner.cpp` or any
sample-level `prj.conf`. Four upstream ExecuTorch files are overridden:

- **`torch_pin.py`** — pins `TORCH_VERSION = "2.12.0"` and clears
  `NIGHTLY_VERSION`. The upstream file at the pinned commit targeted
  `torch==2.11.0.dev20251222`, a nightly wheel purged from the index.
- **`install_requirements.py`** — installs `torch==2.12.0` from the standard
  PyPI index instead of chasing a nightly URL.
- **`examples/models/__init__.py`** — adds the Alif `kws` model registration
  so that `aot_arm_compiler.py` can export the KWS `.pte` file.
- **`zephyr/CMakeLists.txt`** — changes `CONFIG_ETHOS_U` to `CONFIG_ARM_ETHOS_U`
  (the Alif Zephyr driver uses the `ARM_ETHOS_U` Kconfig namespace), disables
  `EXECUTORCH_BUILD_KERNELS_QUANTIZED_AOT` (incompatible with `-fno-rtti`), and
  adds CMSIS-NN local path detection.

## Directory Structure

```
executorch_overrides/
├── torch_pin.py            # Stable torch==2.12.0 pin (replaces dead nightly)
├── install_requirements.py # Stable pip install logic (no nightly URL)
├── examples/
│   └── models/
│       └── __init__.py    # KWS model registration for AOT export
└── zephyr/
    └── CMakeLists.txt     # CONFIG_ARM_ETHOS_U, AOT-off, CMSIS-NN path
```

## Modified Files

### torch_pin.py

**Alif-specific changes:**
- `TORCH_VERSION = "2.12.0"` (was `"2.11.0"`)
- `NIGHTLY_VERSION = ""` (was `"dev20251222"` — purged from nightly index)

### install_requirements.py

**Alif-specific changes:**
- `install_requirements()` always installs `torch==2.12.0` from standard PyPI
  regardless of the `use_pytorch_nightly` flag
- `install_optional_example_requirements()` installs matching stable
  `torchvision==0.27.0` and `torchaudio==2.11.0`

### examples/models/__init__.py

**Alif-specific changes:**
- Added `Kws = "kws"` to Model enum
- Added KWS model registration to `MODEL_NAME_TO_MODEL`

### zephyr/CMakeLists.txt

**Alif-specific changes (rebased on upstream `45fe55c`):**
- Updated `CONFIG_ETHOS_U` to `CONFIG_ARM_ETHOS_U`
- `EXECUTORCH_BUILD_KERNELS_QUANTIZED_AOT` set to `OFF` (Zephyr uses `-fno-rtti`)
- Added CMSIS-NN local path detection from the Zephyr workspace

## How Overrides Are Applied

Overrides are **automatically applied** during `west executorch-setup`:

1. Initializes ExecuTorch git submodules
2. Applies Alif overrides to the ExecuTorch source tree via `apply_executorch_overrides.py`:
   - Copies files from `alif/samples/modules/executorch/executorch_overrides/` to `modules/lib/executorch/`
   - Copies KWS model files from `alif/samples/modules/executorch/models/kws/` to `modules/lib/executorch/examples/models/kws/`
3. Runs `install_executorch.sh` (installs stable `torch==2.12.0` + executorch wheel)
4. Copies KWS files into the installed venv package so `--model_name=kws` works
5. Optionally runs `examples/arm/setup.sh` (FVP simulator — only with `--fvp` flag)

Overrides persist until `west update` is run

## Setup Command

To set up executorch with Alif overrides:

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

To revert to upstream executorch files:

```bash
cd modules/lib/executorch
git restore torch_pin.py
git restore install_requirements.py
git restore examples/models/__init__.py
git restore zephyr/CMakeLists.txt
rm -rf examples/models/kws/
```

## After Upstream Updates

When you update the executorch module via `west update`:

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

1. Edit the file in `alif/samples/modules/executorch/executorch_overrides/`
2. Re-run the override script to apply changes:
   ```bash
   python3 alif/scripts/apply_executorch_overrides.py
   ```

## Tracking Upstream Changes

To see what changed in upstream ExecutorTorch:

```bash
cd modules/lib/executorch
git log --oneline -20
git diff <old-commit> <new-commit> -- zephyr/CMakeLists.txt examples/models/__init__.py
```

Then manually merge relevant changes into the override files in this directory.
