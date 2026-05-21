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
import re
import shutil
import sys
from pathlib import Path


def copy_file_with_dirs(src: Path, dst: Path) -> None:
    """Copy a file, creating parent directories if needed."""
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)
    print(f"  Copied: {src.relative_to(src.parents[3])} -> {dst.relative_to(dst.parents[5])}")


def _resolve_latest_nightly_version(package: str, torch_url: str, nightly_date: str = None, version_prefix: str = None):
    """
    Query pip (via 'pip index versions') for the latest installable nightly
    build of a package.  This is the only authoritative source: the PyTorch
    nightly HTML index often lists wheel filenames before the files are fully
    propagated to the CDN that pip uses, causing spurious 'not found' errors.

    Args:
        package: Package name, e.g. 'torch', 'torchvision', 'torchaudio'
        torch_url: PyTorch nightly index URL
        nightly_date: If given, return the version prefix that matches this date.
        version_prefix: If given, only consider builds with this exact version
                        prefix (e.g. '2.13.0'), so we never jump to a newer
                        major/minor than ExecuTorch was tested against.

    Returns:
        Tuple (version_prefix, nightly_date) e.g. ('2.13.0', 'dev20260512'), or None on failure.
    """
    try:
        import subprocess as _sp

        result = _sp.run(
            [sys.executable, "-m", "pip", "index", "versions", package,
             "--pre", "--extra-index-url", torch_url],
            capture_output=True, text=True, timeout=60,
        )
        # pip index versions output: "AVAILABLE VERSIONS: x.y.z, ..." or similar
        output = result.stdout + result.stderr

        matches = re.findall(
            rf'(\d+\.\d+\.\d+)\.(dev(\d{{8}}))',
            output,
        )
        if not matches:
            return None

        if version_prefix:
            matches = [(v, d, s) for v, d, s in matches if v == version_prefix]
            if not matches:
                return None

        if nightly_date:
            for ver_prefix, dev_str, date_str in matches:
                if dev_str == nightly_date:
                    return (ver_prefix, dev_str)
            return None
        else:
            latest = max(matches, key=lambda t: t[2])
            return (latest[0], latest[1])
    except Exception as err:
        print(f"  Warning: Could not resolve latest nightly for {package} from pip: {err}")
        return None


def patch_torch_versions(executorch_dir: Path) -> bool:
    """
    Patch ExecutorTorch torch version pins to use available nightly builds.

    The nightly version is resolved dynamically by querying the PyTorch nightly
    index, so this never goes stale when old builds are purged from PyPI.

    Args:
        executorch_dir: Path to the executorch module directory

    Returns:
        True on success, False on error
    """
    print("Patching PyTorch version pins...")

    TORCH_NIGHTLY_URL = "https://download.pytorch.org/whl/nightly/cpu"
    FALLBACK_NIGHTLY_VERSION = "dev20260415"
    FALLBACK_TORCHVISION_VERSION = "0.27.0"
    FALLBACK_TORCHAUDIO_VERSION = "2.12.0"

    # Read the upstream-pinned TORCH_VERSION from torch_pin.py so we never
    # jump to a newer major/minor than ExecuTorch was tested against.
    torch_pin_file = executorch_dir / "torch_pin.py"
    PINNED_TORCH_VERSION = None
    if torch_pin_file.exists():
        with open(torch_pin_file, 'r') as f:
            m = re.search(r'TORCH_VERSION\s*=\s*"(\d+\.\d+\.\d+)"', f.read())
            if m:
                PINNED_TORCH_VERSION = m.group(1)

    if PINNED_TORCH_VERSION:
        print(f"  Upstream-pinned torch version: {PINNED_TORCH_VERSION}")
        print(f"  Querying PyTorch nightly index for latest {PINNED_TORCH_VERSION} build...")
        torch_result = _resolve_latest_nightly_version("torch", TORCH_NIGHTLY_URL, version_prefix=PINNED_TORCH_VERSION)
        if not torch_result:
            print(f"  No {PINNED_TORCH_VERSION} nightly found, trying latest available nightly...")
            torch_result = _resolve_latest_nightly_version("torch", TORCH_NIGHTLY_URL)
    else:
        print(f"  Could not read upstream torch_pin.py, querying latest torch nightly...")
        torch_result = _resolve_latest_nightly_version("torch", TORCH_NIGHTLY_URL)

    if torch_result:
        TARGET_TORCH_VERSION, TARGET_NIGHTLY_VERSION = torch_result
        print(f"  Resolved torch: {TARGET_TORCH_VERSION}.{TARGET_NIGHTLY_VERSION}")
    else:
        TARGET_TORCH_VERSION = PINNED_TORCH_VERSION or "2.12.0"
        TARGET_NIGHTLY_VERSION = FALLBACK_NIGHTLY_VERSION
        print(f"  Could not resolve torch nightly, falling back to {TARGET_TORCH_VERSION}.{TARGET_NIGHTLY_VERSION}")

    print(f"  Querying PyTorch nightly index for torchvision matching {TARGET_NIGHTLY_VERSION}...")
    tv_result = _resolve_latest_nightly_version("torchvision", TORCH_NIGHTLY_URL, TARGET_NIGHTLY_VERSION)
    if tv_result:
        TARGET_TORCHVISION_VERSION = tv_result[0]
        print(f"  Resolved torchvision: {TARGET_TORCHVISION_VERSION}.{TARGET_NIGHTLY_VERSION}")
    else:
        TARGET_TORCHVISION_VERSION = FALLBACK_TORCHVISION_VERSION
        print(f"  Could not resolve torchvision nightly, falling back to {TARGET_TORCHVISION_VERSION}")

    print(f"  Querying PyTorch nightly index for torchaudio matching {TARGET_NIGHTLY_VERSION}...")
    ta_result = _resolve_latest_nightly_version("torchaudio", TORCH_NIGHTLY_URL, TARGET_NIGHTLY_VERSION)
    if ta_result:
        TARGET_TORCHAUDIO_VERSION = ta_result[0]
        print(f"  Resolved torchaudio: {TARGET_TORCHAUDIO_VERSION}.{TARGET_NIGHTLY_VERSION}")
    else:
        TARGET_TORCHAUDIO_VERSION = FALLBACK_TORCHAUDIO_VERSION
        print(f"  Could not resolve torchaudio nightly, falling back to {TARGET_TORCHAUDIO_VERSION}")

    # Patch torch_pin.py using regex for robustness
    torch_pin_file = executorch_dir / "torch_pin.py"
    if torch_pin_file.exists():
        with open(torch_pin_file, 'r') as f:
            content = f.read()

        original_content = content

        # Match TORCH_VERSION = "X.Y.Z" where X, Y, Z are digits
        content = re.sub(
            r'TORCH_VERSION\s*=\s*"(\d+\.\d+\.\d+)"',
            f'TORCH_VERSION = "{TARGET_TORCH_VERSION}"',
            content
        )
        # Match NIGHTLY_VERSION = "devYYYYMMDD"
        content = re.sub(
            r'NIGHTLY_VERSION\s*=\s*"(dev\d+)"',
            f'NIGHTLY_VERSION = "{TARGET_NIGHTLY_VERSION}"',
            content
        )

        if content != original_content:
            with open(torch_pin_file, 'w') as f:
                f.write(content)
            print(f"  ✓ Updated torch_pin.py: TORCH_VERSION={TARGET_TORCH_VERSION}, NIGHTLY_VERSION={TARGET_NIGHTLY_VERSION}")
        else:
            print(f"  ✓ torch_pin.py already up to date or pattern not found")
    else:
        print(f"  Warning: torch_pin.py not found")

    # Patch install_requirements.py using regex
    install_req_file = executorch_dir / "install_requirements.py"
    if install_req_file.exists():
        with open(install_req_file, 'r') as f:
            content = f.read()

        original_content = content

        # Match torchvision==X.Y.Z.{NIGHTLY_VERSION}
        content = re.sub(
            r'torchvision==(\d+\.\d+\.\d+)\.\{NIGHTLY_VERSION\}',
            f'torchvision=={TARGET_TORCHVISION_VERSION}.{{NIGHTLY_VERSION}}',
            content
        )
        # Match torchaudio==X.Y.Z.{NIGHTLY_VERSION}
        content = re.sub(
            r'torchaudio==(\d+\.\d+\.\d+)\.\{NIGHTLY_VERSION\}',
            f'torchaudio=={TARGET_TORCHAUDIO_VERSION}.{{NIGHTLY_VERSION}}',
            content
        )

        if content != original_content:
            with open(install_req_file, 'w') as f:
                f.write(content)
            print(f"  ✓ Updated install_requirements.py: torchvision={TARGET_TORCHVISION_VERSION}, torchaudio={TARGET_TORCHAUDIO_VERSION}")
        else:
            print(f"  ✓ install_requirements.py already up to date or pattern not found")
    else:
        print(f"  Warning: install_requirements.py not found")

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

    # Patch PyTorch version pins first
    patch_torch_versions(executorch_dir)

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

    # Copy KWS model files from alif/samples/modules/executorch/models/kws to executorch/examples/models/kws
    kws_src_dir = alif_dir / 'samples' / 'modules' / 'executorch' / 'models' / 'kws'
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
