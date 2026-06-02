#!/usr/bin/env python3

# Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

"""
Script to apply Alif-specific overrides to ExecutorTorch.

This script copies modified files from alif/samples/modules/executorch/executorch_overrides/
to the executorch module directory, replacing upstream files with Alif-specific versions.
"""

import os
import shutil
import sys
from pathlib import Path


def copy_file_with_dirs(src: Path, dst: Path,
                        src_base: Path = None, dst_base: Path = None) -> None:
    """Copy a file, creating parent directories if needed."""
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)
    src_label = src.relative_to(src_base) if src_base else src.name
    dst_label = dst.relative_to(dst_base) if dst_base else dst.name
    print(f"  Copied: {src_label} -> {dst_label}")


def _read_pinned_torch_version(executorch_dir: Path):
    """
    Read the 'TORCH_VERSION' that ExecuTorch pins in its torch_pin.py.

    Returns the version string (e.g. '2.12.0'), or None if it can't be read.
    """
    torch_pin_file = executorch_dir / "torch_pin.py"
    if not torch_pin_file.exists():
        return None
    try:
        with open(torch_pin_file, 'r') as f:
            for line in f:
                line = line.strip()
                if line.startswith('TORCH_VERSION') and '=' in line and '"' in line:
                    return line.split('"')[1]
        return None
    except Exception as err:
        print(f"  Warning: could not read torch_pin.py: {err}")
        return None


def report_torch_pin(executorch_dir: Path) -> bool:
    """
    Report (but deliberately DO NOT modify) the PyTorch version that the
    pinned ExecuTorch commit depends on.

    History / rationale
    -------------------
    This script used to rewrite ExecuTorch's torch_pin.py and
    install_requirements.py to chase the *latest* PyTorch nightly.  That was
    the root cause of recurring, hard-to-debug breakage:

      * PyTorch purges nightly wheels from its index after a few weeks, so a
        previously-resolved nightly would simply disappear; and
      * the "latest" nightly inevitably drifts away from the exact build the
        pinned ExecuTorch *source* was written against, producing ABI /
        op-schema errors such as
        "The underlying op of 'aten.transpose' has no overload name 'Dimname'".

    ExecuTorch source and torch must move together.  The module is pinned
    (see submanifests/executorch.yaml) to commit 45fe55c.  That commit's own
    torch_pin.py still referenced a nightly wheel (dev20251222) that has been
    purged from the index.  The Alif overrides replace torch_pin.py and
    install_requirements.py with versions that install torch==2.12.0+cpu
    (CPU-only, permanent on the PyTorch wheel index) before this function
    is called, so the version reported here should always be 2.12.0.
    """
    print("Checking PyTorch version pin...")
    pinned = _read_pinned_torch_version(executorch_dir)
    if pinned:
        print(f"  torch_pin.py reports: torch=={pinned} (stable release, Alif override active).")
    else:
        print("  Warning: could not read torch_pin.py — "
              "Alif override may not have been applied yet.")
    print()
    return True


def apply_overrides(workspace_root: Path) -> int:
    """
    Apply Alif-specific overrides to ExecutorTorch.

    Args:
        workspace_root: Path to the workspace root (where .west directory is)

    Returns:
        0 on success, 1 on error
    """
    alif_dir = workspace_root / 'alif'
    executorch_dir = workspace_root / 'modules' / 'lib' / 'executorch'
    overrides_dir = alif_dir / 'samples' / 'modules' / 'executorch' / 'executorch_overrides'

    if not executorch_dir.exists():
        print(f"Error: ExecutorTorch directory not found: {executorch_dir}")
        print("Please run 'west update' first to clone executorch")
        return 1

    if not overrides_dir.exists():
        print(f"Error: Overrides directory not found: {overrides_dir}")
        return 1

    print(f"Applying Alif overrides to ExecutorTorch...")
    print(f"  Source: {overrides_dir}")
    print(f"  Target: {executorch_dir}")
    print()

    # Copy override files.  The key maps an override source (relative to the
    # overrides dir) to its destination path in the ExecuTorch tree.  Keeping
    # them as explicit (src -> dst) pairs lets us follow upstream layout
    # changes without renaming the files in this repo.
    override_files = {
        # Stable PyTorch pin — replaces the upstream nightly pin
        # (torch==2.11.0.dev20251222) that has been purged from the index.
        # Must be applied BEFORE install_executorch.sh runs.
        'torch_pin.py': 'torch_pin.py',

        # Stable pip install logic — ignores use_pytorch_nightly flag and
        # always installs torch==2.12.0 from the standard PyPI index.
        # Must be applied BEFORE install_executorch.sh runs.
        'install_requirements.py': 'install_requirements.py',

        # AOT (.pte generation) overrides — required for KWS export.
        # This file is kept in sync with the pinned upstream commit and only
        # adds the Alif 'kws' model registration, so it is safe to apply.
        'examples/models/__init__.py': 'examples/models/__init__.py',

        # Top-level Zephyr CMakeLists: CONFIG_ETHOS_U -> CONFIG_ARM_ETHOS_U,
        # EXECUTORCH_BUILD_KERNELS_QUANTIZED_AOT OFF, CMSIS-NN path detection.
        'zephyr/CMakeLists.txt': 'zephyr/CMakeLists.txt',
    }

    for src_rel, dst_rel in override_files.items():
        src = overrides_dir / src_rel
        dst = executorch_dir / dst_rel

        if not src.exists():
            print(f"  Warning: Override source not found: {src}")
            continue
        # Only overwrite files whose destination already exists upstream.
        # This prevents creating stray files/dirs if the upstream layout has
        # changed (which would silently mask a real porting problem).
        if not dst.exists():
            print(f"  SKIPPED (no upstream target): {dst_rel} -- the upstream "
                  f"layout changed for this ExecuTorch version; this override "
                  f"needs re-pathing. See executorch_overrides/README.md.")
            continue
        copy_file_with_dirs(src, dst, overrides_dir, executorch_dir)

    # Report the torch version now in effect (after torch_pin.py override).
    report_torch_pin(executorch_dir)

    # Copy KWS model files into the ExecuTorch source tree so aot_arm_compiler
    # can find them at build time via 'executorch.examples.models.kws'.
    kws_src_dir = alif_dir / 'samples' / 'modules' / 'executorch' / 'models' / 'kws'
    kws_dst_dir = executorch_dir / 'examples' / 'models' / 'kws'

    if kws_src_dir.exists():
        print()
        print("Copying KWS model files to ExecuTorch source tree...")
        kws_dst_dir.mkdir(parents=True, exist_ok=True)
        for file in kws_src_dir.glob('*.py'):
            copy_file_with_dirs(file, kws_dst_dir / file.name,
                                alif_dir, executorch_dir)
    else:
        print(f"  Warning: KWS model directory not found: {kws_src_dir}")

    print()
    print("Alif overrides applied to ExecuTorch source tree.")
    print("Note: Re-run 'west executorch-setup' or this script after 'west update'.")
    return 0


def copy_kws_to_venv(workspace_root: Path) -> None:
    """
    Copy KWS model files into the *installed* executorch package inside the
    active venv.  Must be called after install_executorch.sh has run so the
    package directory exists.
    """
    alif_dir = workspace_root / 'alif'
    overrides_dir = alif_dir / 'samples' / 'modules' / 'executorch' / 'executorch_overrides'
    kws_src_dir = alif_dir / 'samples' / 'modules' / 'executorch' / 'models' / 'kws'

    _venv_env = os.environ.get('VIRTUAL_ENV')
    venv_dir = Path(_venv_env) if _venv_env else (workspace_root / '.zephyr_venv')
    if not venv_dir.exists():
        return

    site_packages = list(venv_dir.glob('lib/python*/site-packages'))
    if not site_packages:
        return

    installed_models = site_packages[0] / 'executorch' / 'examples' / 'models'
    if not installed_models.exists():
        return

    print()
    print("Copying KWS model files to installed executorch package...")

    if kws_src_dir.exists():
        kws_dst = installed_models / 'kws'
        kws_dst.mkdir(parents=True, exist_ok=True)
        for file in kws_src_dir.glob('*.py'):
            copy_file_with_dirs(file, kws_dst / file.name, alif_dir, venv_dir)

    # Update __init__.py in the installed package so 'kws' is importable.
    override_init = overrides_dir / 'examples' / 'models' / '__init__.py'
    installed_init = installed_models / '__init__.py'
    if override_init.exists() and installed_init.exists():
        shutil.copy2(override_init, installed_init)
        print(f"  Updated: examples/models/__init__.py in installed package")


def main():
    """Main entry point."""
    # Determine workspace root
    # When called from west command, CWD should be workspace root
    # When called directly, try to find .west directory
    cwd = Path.cwd()

    if (cwd / '.west').exists():
        workspace_root = cwd
    elif (cwd.parent / '.west').exists():
        workspace_root = cwd.parent
    elif (cwd.parent.parent / '.west').exists():
        workspace_root = cwd.parent.parent
    else:
        print("Error: Could not find workspace root (.west directory)")
        print("Please run this script from the workspace root or alif directory")
        return 1

    return apply_overrides(workspace_root)


if __name__ == '__main__':
    sys.exit(main())
