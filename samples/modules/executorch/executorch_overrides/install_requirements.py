# Copyright (c) Meta Platforms, Inc. and affiliates.
# Copyright 2024-25 Arm Limited and/or its affiliates.
# All rights reserved.
# Copyright 2026 Alif Semiconductor - Modified for Alif SDK compatibility
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.
#
# Alif override: install stable torch==2.12.0 from the normal PyPI index.
# The upstream version of this file always tried to install a nightly build
# (torch==2.11.0.dev20251222) whose wheel has been purged from the PyTorch
# nightly index and is no longer available.  Stable releases are permanent,
# so this pairing never goes stale.

import argparse
import os
import subprocess
import sys

from install_utils import determine_torch_url, is_intel_mac_os, python_is_compatible

from torch_pin import NIGHTLY_VERSION, TORCH_VERSION

# The pip repository that hosts nightly torch packages.
TORCH_NIGHTLY_URL_BASE = "https://download.pytorch.org/whl/nightly"

# CPU-only wheel index — stable, permanent, no CUDA dependency.
TORCH_CPU_URL = "https://download.pytorch.org/whl/cpu"

# Stable CPU-only torch version spec.
# torch==2.12.0+cpu is used instead of the default PyPI torch==2.12.0 because
# the default wheel includes CUDA support which causes the executorch cmake
# preset 'pybind' to fail on machines without CUDA (libcuda.so not found).
_STABLE_TORCH_SPEC = f"torch=={TORCH_VERSION}+cpu"


def install_requirements(use_pytorch_nightly):
    # Skip pip install on Intel macOS if using nightly.
    if use_pytorch_nightly and is_intel_mac_os():
        print(
            "ERROR: Prebuilt PyTorch wheels are no longer available for Intel-based macOS.\n"
            "Please build from source by following https://docs.pytorch.org/executorch/main/using-executorch-building-from-source.html",
            file=sys.stderr,
        )
        sys.exit(1)

    # Alif: always install the stable torch release from the standard PyPI
    # index.  We intentionally ignore use_pytorch_nightly here because the
    # pinned nightly (NIGHTLY_VERSION at the upstream commit) no longer exists
    # on the index.  Stable wheels are permanent.
    #
    # NOTE: requirements-dev.txt is installed separately from PyPI (no
    # --index-url) because the PyTorch CPU wheel index only carries a handful
    # of packages (e.g. cmake==3.25.0) and does NOT satisfy the
    # cmake>=3.29,<4.0.0 constraint.  torch is then installed from the PyTorch
    # CPU index in a second call.
    print("Installing requirements-dev.txt from PyPI")
    subprocess.run(
        [
            sys.executable,
            "-m",
            "pip",
            "install",
            "-r",
            "requirements-dev.txt",
        ],
        check=True,
    )

    print(f"Installing {_STABLE_TORCH_SPEC} (CPU-only stable, Alif override)")
    subprocess.run(
        [
            sys.executable,
            "-m",
            "pip",
            "install",
            _STABLE_TORCH_SPEC,
            "--index-url",
            TORCH_CPU_URL,
        ],
        check=True,
    )

    LOCAL_REQUIREMENTS = [
        "third-party/ao",
    ] + (
        [
            "extension/llm/tokenizers",
        ]
        if sys.platform != "win32"
        else []
    )

    new_env = os.environ.copy()
    if ("EXECUTORCH_BUILD_KERNELS_TORCHAO" not in new_env) or (
        new_env["EXECUTORCH_BUILD_KERNELS_TORCHAO"] == "0"
    ):
        new_env["USE_CPP"] = "0"
    else:
        assert new_env["EXECUTORCH_BUILD_KERNELS_TORCHAO"] == "1"
        new_env["USE_CPP"] = "1"
        new_env["CMAKE_POLICY_VERSION_MINIMUM"] = "3.5"
    subprocess.run(
        [
            sys.executable,
            "-m",
            "pip",
            "install",
            "--no-build-isolation",
            *LOCAL_REQUIREMENTS,
        ],
        env=new_env,
        check=True,
    )


def install_optional_example_requirements(use_pytorch_nightly):
    # Alif: install stable matching torchvision/torchaudio for torch 2.12.0.
    print("Installing torch domain libraries (stable, Alif override)")
    DOMAIN_LIBRARIES = [
        "torchvision==0.27.0+cpu",
        "torchaudio==2.11.0+cpu",
    ]
    subprocess.run(
        [
            sys.executable,
            "-m",
            "pip",
            "install",
            *DOMAIN_LIBRARIES,
            "--index-url",
            TORCH_CPU_URL,
        ],
        check=True,
    )

    print("Installing packages in requirements-examples.txt")
    subprocess.run(
        [
            sys.executable,
            "-m",
            "pip",
            "install",
            "-r",
            "requirements-examples.txt",
            "--upgrade-strategy",
            "only-if-needed",
        ],
        check=True,
    )


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--use-pt-pinned-commit",
        action="store_true",
        help="build from the pinned PyTorch commit instead of nightly",
    )
    parser.add_argument(
        "--example",
        action="store_true",
        help="Also installs required packages for running example scripts.",
    )
    args = parser.parse_args(args)
    use_pytorch_nightly = not bool(args.use_pt_pinned_commit)
    install_requirements(use_pytorch_nightly)
    if args.example:
        install_optional_example_requirements(use_pytorch_nightly)


if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    if not python_is_compatible():
        sys.exit(1)
    main(sys.argv[1:])
