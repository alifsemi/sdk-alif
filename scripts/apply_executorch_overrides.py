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


def _query_available_nightlies(package: str, torch_url: str):
    """
    Query pip (via 'pip index versions') for all installable nightly builds
    of a package.

    Args:
        package: Package name, e.g. 'torch', 'torchvision', 'torchaudio'
        torch_url: PyTorch nightly index URL

    Returns:
        Dict mapping date string (e.g. 'dev20260525') to version prefix
        (e.g. '0.28.0').  Empty dict on failure.  If multiple version
        prefixes exist for the same date, the highest one wins.
    """
    try:
        import subprocess as _sp

        result = _sp.run(
            [sys.executable, "-m", "pip", "index", "versions", package,
             "--pre", "--extra-index-url", torch_url],
            capture_output=True, text=True, timeout=60,
        )
        output = result.stdout + result.stderr

        matches = re.findall(r'(\d+\.\d+\.\d+)\.(dev\d{8})', output)
        date_to_version = {}
        for ver, dev in matches:
            # Higher version wins for the same date.  Compare numerically by
            # (major, minor, patch) so e.g. "2.10.0" correctly outranks
            # "2.9.0" (a plain string compare would order them incorrectly).
            def _ver_key(v):
                return tuple(int(p) for p in v.split("."))
            if dev not in date_to_version or \
                    _ver_key(ver) > _ver_key(date_to_version[dev]):
                date_to_version[dev] = ver
        return date_to_version
    except Exception as err:
        print(f"  Warning: Could not query nightlies for {package} from pip: {err}")
        return {}


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
    FALLBACK_TORCH_VERSION = "2.13.0"
    FALLBACK_NIGHTLY_VERSION = "dev20260525"
    FALLBACK_TORCHVISION_VERSION = "0.28.0"
    FALLBACK_TORCHAUDIO_VERSION = "2.11.0"

    # Read the upstream-pinned TORCH_VERSION from torch_pin.py so we prefer
    # the same major/minor that ExecuTorch was tested against.
    torch_pin_file = executorch_dir / "torch_pin.py"
    PINNED_TORCH_VERSION = None
    if torch_pin_file.exists():
        with open(torch_pin_file, 'r') as f:
            m = re.search(r'TORCH_VERSION\s*=\s*"(\d+\.\d+\.\d+)"', f.read())
            if m:
                PINNED_TORCH_VERSION = m.group(1)

    if PINNED_TORCH_VERSION:
        print(f"  Upstream-pinned torch version: {PINNED_TORCH_VERSION}")

    # Query the nightly index for all three packages.  We then pick the
    # latest nightly DATE for which torchvision AND torchaudio are both
    # published, and resolve the torch nightly from the torchvision wheel's
    # declared dependency (which is often a date a day or two earlier than
    # the tv/ta date itself).
    # This is critical because the per-day builds are often partial:
    # e.g. torch dev20260526 can exist while torchvision/torchaudio for
    # that same date are not yet published, and stale dates get purged
    # from PyPI over time.
    print(f"  Querying PyTorch nightly index for torch / torchvision / torchaudio...")
    torch_nightlies = _query_available_nightlies("torch", TORCH_NIGHTLY_URL)
    tv_nightlies = _query_available_nightlies("torchvision", TORCH_NIGHTLY_URL)
    ta_nightlies = _query_available_nightlies("torchaudio", TORCH_NIGHTLY_URL)

    # Restrict torch candidates to the upstream-pinned major.minor when
    # possible, so we don't unexpectedly jump to a newer torch series.
    torch_candidates = torch_nightlies
    if PINNED_TORCH_VERSION:
        pinned_only = {d: v for d, v in torch_nightlies.items()
                       if v == PINNED_TORCH_VERSION}
        if pinned_only:
            torch_candidates = pinned_only
        else:
            print(f"  No nightly available for pinned torch {PINNED_TORCH_VERSION}; "
                  f"falling back to any available torch nightly.")

    # Latest dates with torchvision AND torchaudio published.  We try these
    # newest-first; pip will then pull whatever torch nightly the
    # tv/ta wheels actually depend on (usually published a day or two
    # earlier than tv/ta).
    tv_ta_common_dates = sorted(
        set(tv_nightlies) & set(ta_nightlies),
        reverse=True,
    )

    # For each candidate date (newest first), download just the torchvision
    # wheel (~2 MB) and read its METADATA to discover the exact torch
    # nightly it depends on.  We then verify that the corresponding torch
    # nightly is actually available before accepting the triplet.
    resolved = None
    import subprocess as _sp
    import tempfile
    import zipfile
    for candidate_date in tv_ta_common_dates[:10]:
        cand_tv = tv_nightlies[candidate_date]
        cand_ta = ta_nightlies[candidate_date]
        tv_spec = f"torchvision=={cand_tv}.{candidate_date}"
        print(f"  Probing tv/ta date {candidate_date} via {tv_spec} wheel metadata...")
        with tempfile.TemporaryDirectory() as tmpdir:
            try:
                dl = _sp.run(
                    [sys.executable, "-m", "pip", "download", "--no-deps",
                     "-d", tmpdir, tv_spec,
                     "--extra-index-url", TORCH_NIGHTLY_URL],
                    capture_output=True, text=True, timeout=180,
                )
                if dl.returncode != 0:
                    print(f"    -> download failed, trying older date")
                    continue
                whl_files = [f for f in os.listdir(tmpdir) if f.endswith(".whl")]
                if not whl_files:
                    print(f"    -> no wheel produced, trying older date")
                    continue
                torch_req = None
                with zipfile.ZipFile(os.path.join(tmpdir, whl_files[0])) as zf:
                    for name in zf.namelist():
                        if name.endswith("METADATA"):
                            meta = zf.read(name).decode("utf-8", "replace")
                            for line in meta.splitlines():
                                if line.startswith("Requires-Dist:") and \
                                        re.search(r'\btorch\b', line):
                                    torch_req = line
                                    break
                            break
                if not torch_req:
                    print(f"    -> no torch Requires-Dist in metadata, "
                          f"trying older date")
                    continue
                # Examples we need to handle:
                #   "Requires-Dist: torch (==2.13.0.dev20260524)"
                #   "Requires-Dist: torch==2.13.0.dev20260524"
                tm = re.search(
                    r'torch\s*\(?==\s*(\d+\.\d+\.\d+)\.(dev\d{8})',
                    torch_req,
                )
                if not tm:
                    print(f"    -> could not parse torch pin from "
                          f"{torch_req!r}, trying older date")
                    continue
                cand_torch_version = tm.group(1)
                cand_torch_date = tm.group(2)
                # Verify the torch nightly actually exists in the index and
                # matches the upstream-pinned torch series (torch_candidates).
                if cand_torch_date not in torch_candidates or \
                        torch_candidates[cand_torch_date] != cand_torch_version:
                    print(f"    -> required torch "
                          f"{cand_torch_version}.{cand_torch_date} not "
                          f"available, trying older date")
                    continue
                resolved = (candidate_date, cand_torch_version,
                            cand_torch_date, cand_tv, cand_ta)
                break
            except Exception as err:
                print(f"    -> probe error: {err}")

    if resolved:
        TV_TA_DATE, TARGET_TORCH_VERSION, TARGET_NIGHTLY_VERSION, \
            TARGET_TORCHVISION_VERSION, TARGET_TORCHAUDIO_VERSION = resolved
        print(f"  Resolved:")
        print(f"    torch       = {TARGET_TORCH_VERSION}.{TARGET_NIGHTLY_VERSION}")
        print(f"    torchvision = {TARGET_TORCHVISION_VERSION}.{TV_TA_DATE}")
        print(f"    torchaudio  = {TARGET_TORCHAUDIO_VERSION}.{TV_TA_DATE}")
    else:
        TARGET_TORCH_VERSION = FALLBACK_TORCH_VERSION
        TARGET_NIGHTLY_VERSION = FALLBACK_NIGHTLY_VERSION
        TV_TA_DATE = FALLBACK_NIGHTLY_VERSION
        TARGET_TORCHVISION_VERSION = FALLBACK_TORCHVISION_VERSION
        TARGET_TORCHAUDIO_VERSION = FALLBACK_TORCHAUDIO_VERSION
        print(f"  Could not resolve nightly via dry-run; falling back to "
              f"torch={TARGET_TORCH_VERSION}.{TARGET_NIGHTLY_VERSION}, "
              f"torchvision={TARGET_TORCHVISION_VERSION}.{TV_TA_DATE}, "
              f"torchaudio={TARGET_TORCHAUDIO_VERSION}.{TV_TA_DATE}.")

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

        # torchvision/torchaudio nightlies are typically published one day
        # later than the torch nightly they depend on, so we hardcode a
        # specific tv/ta date here (TV_TA_DATE) which may differ from
        # NIGHTLY_VERSION (used for torch).  This decouples the two.
        # Replace 'X.Y.Z.{NIGHTLY_VERSION}' with 'A.B.C.devYYYYMMDD'.
        content = re.sub(
            r'torchvision==(\d+\.\d+\.\d+)\.\{NIGHTLY_VERSION\}',
            f'torchvision=={TARGET_TORCHVISION_VERSION}.{TV_TA_DATE}',
            content
        )
        content = re.sub(
            r'torchaudio==(\d+\.\d+\.\d+)\.\{NIGHTLY_VERSION\}',
            f'torchaudio=={TARGET_TORCHAUDIO_VERSION}.{TV_TA_DATE}',
            content
        )
        # Also handle the case where a previous patch already hardcoded
        # the date (so re-running is idempotent and picks up newer dates).
        content = re.sub(
            r'torchvision==(\d+\.\d+\.\d+)\.dev\d{8}',
            f'torchvision=={TARGET_TORCHVISION_VERSION}.{TV_TA_DATE}',
            content
        )
        content = re.sub(
            r'torchaudio==(\d+\.\d+\.\d+)\.dev\d{8}',
            f'torchaudio=={TARGET_TORCHAUDIO_VERSION}.{TV_TA_DATE}',
            content
        )

        if content != original_content:
            with open(install_req_file, 'w') as f:
                f.write(content)
            print(f"  ✓ Updated install_requirements.py: "
                  f"torchvision={TARGET_TORCHVISION_VERSION}.{TV_TA_DATE}, "
                  f"torchaudio={TARGET_TORCHAUDIO_VERSION}.{TV_TA_DATE}")
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
