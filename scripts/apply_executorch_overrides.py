#!/usr/bin/env python3
"""
Script to apply Alif-specific overrides to ExecutorTorch.

This script copies modified files from alif/executorch_overrides/ to the
executorch module directory, replacing upstream files with Alif-specific versions.
"""

import os
import shutil
import sys
from pathlib import Path


def copy_file_with_dirs(src: Path, dst: Path) -> None:
    """Copy a file, creating parent directories if needed."""
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)
    print(f"  Copied: {src.relative_to(src.parents[3])} -> {dst.relative_to(dst.parents[5])}")


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
    overrides_dir = alif_dir / 'executorch_overrides'

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

    # Copy all override files
    override_files = [
        'examples/arm/zephyr/src/arm_executor_runner.cpp',
        'examples/arm/zephyr/prj.conf',
        'examples/models/__init__.py',
        'zephyr/CMakeLists.txt',
    ]

    for file_path in override_files:
        src = overrides_dir / file_path
        dst = executorch_dir / file_path

        if src.exists():
            copy_file_with_dirs(src, dst)
        else:
            print(f"  Warning: Override file not found: {src}")

    # Copy KWS model files from alif/models/kws to executorch/examples/models/kws
    kws_src_dir = alif_dir / 'models' / 'kws'
    kws_dst_dir = executorch_dir / 'examples' / 'models' / 'kws'

    if kws_src_dir.exists():
        print()
        print("Copying KWS model files to source directory...")

        # Create destination directory
        kws_dst_dir.mkdir(parents=True, exist_ok=True)

        # Copy Python files (skip README and __pycache__)
        for file in kws_src_dir.glob('*.py'):
            dst = kws_dst_dir / file.name
            shutil.copy2(file, dst)
            print(f"  Copied: {file.relative_to(alif_dir)} -> {dst.relative_to(executorch_dir)}")
    else:
        print(f"  Warning: KWS model directory not found: {kws_src_dir}")

    # Also copy KWS files to installed executorch package in venv if it exists
    venv_dir = workspace_root / '.zephyr_venv'
    if venv_dir.exists():
        # Find the site-packages directory
        site_packages = list(venv_dir.glob('lib/python*/site-packages'))
        if site_packages:
            installed_executorch = site_packages[0] / 'executorch' / 'examples' / 'models'
            if installed_executorch.exists() and kws_src_dir.exists():
                print()
                print("Copying KWS model files to installed package...")
                kws_installed_dst = installed_executorch / 'kws'
                kws_installed_dst.mkdir(parents=True, exist_ok=True)

                for file in kws_src_dir.glob('*.py'):
                    dst = kws_installed_dst / file.name
                    shutil.copy2(file, dst)
                    print(f"  Copied: {file.relative_to(alif_dir)} -> {dst.relative_to(venv_dir)}")

                # Also update the __init__.py in the installed package
                installed_init = installed_executorch / '__init__.py'
                override_init = overrides_dir / 'examples' / 'models' / '__init__.py'
                if override_init.exists() and installed_init.exists():
                    shutil.copy2(override_init, installed_init)
                    print(f"  Updated: examples/models/__init__.py in installed package")

    print()
    print("Alif overrides applied successfully!")
    print()
    print("Note: These changes are local to your workspace.")
    print("Running 'west update' will revert these changes.")
    print("Re-run 'west executorch-setup' to reapply overrides after updates.")

    return 0


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
