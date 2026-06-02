# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
# Copyright 2026 Alif Semiconductor - Modified for Alif SDK compatibility
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.
#
# Alif override: pin to stable PyTorch 2.12.0 (permanent on PyPI).
# The original file at commit 45fe55c targeted torch==2.11.0.dev20251222,
# a nightly wheel that has been purged from the index and is no longer
# installable.  Setting NIGHTLY_VERSION to "" ensures that
# install_requirements.py constructs "torch==2.12.0" (stable) rather than
# "torch==2.12.0.devYYYYMMDD" when use_pytorch_nightly=True.
TORCH_VERSION = "2.12.0"
NIGHTLY_VERSION = ""
