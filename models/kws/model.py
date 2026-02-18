# Copyright (c) Meta Platforms, Inc. and affiliates.
# Copyright 2025 Arm Limited and/or its affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

"""
Keyword Spotting CNN Small Model for ExecutorTorch

This model is equivalent to the keyword_spotting_cnn_small_int8 model used in
the ML Embedded Evaluation Kit and TFLite-Micro samples.

Architecture: DS-CNN (Depthwise Separable CNN) Small
Input: 49 time steps x 10 MFCC features (490 elements, int8 quantized)
Output: 12 classes (silence, unknown, yes, no, up, down, left, right, on, off, stop, go)

Reference:
- ARM ML Zoo: https://github.com/ARM-software/ML-zoo/tree/master/models/keyword_spotting
- Google Speech Commands: https://arxiv.org/abs/1804.03209
- DS-CNN Paper: https://arxiv.org/abs/1711.07128
"""

import torch
import torch.nn as nn

from ..model_base import EagerModelBase


class DepthwiseSeparableConv2d(nn.Module):
    """Depthwise Separable Convolution block used in DS-CNN."""

    def __init__(self, in_channels, out_channels, kernel_size, stride=1, padding=0):
        super().__init__()
        self.depthwise = nn.Conv2d(
            in_channels, in_channels, kernel_size,
            stride=stride, padding=padding, groups=in_channels, bias=False
        )
        self.bn1 = nn.BatchNorm2d(in_channels)
        self.pointwise = nn.Conv2d(in_channels, out_channels, 1, bias=False)
        self.bn2 = nn.BatchNorm2d(out_channels)
        self.relu = nn.ReLU(inplace=True)

    def forward(self, x):
        x = self.depthwise(x)
        x = self.bn1(x)
        x = self.relu(x)
        x = self.pointwise(x)
        x = self.bn2(x)
        x = self.relu(x)
        return x


class DSCNN_Small(nn.Module):
    """
    DS-CNN Small model for Keyword Spotting.

    This architecture matches the keyword_spotting_cnn_small_int8 model
    from ARM ML Zoo / ML Embedded Evaluation Kit.

    Input shape: (batch, 1, 49, 10) - 49 time frames, 10 MFCC coefficients
    Output shape: (batch, 12) - 12 keyword classes
    """

    def __init__(self, num_classes=12):
        super().__init__()

        # Initial convolution layer
        self.conv1 = nn.Conv2d(1, 64, kernel_size=(10, 4), stride=(2, 2), padding=(5, 1), bias=False)
        self.bn1 = nn.BatchNorm2d(64)
        self.relu = nn.ReLU(inplace=True)

        # Depthwise separable convolution layers
        self.ds_conv1 = DepthwiseSeparableConv2d(64, 64, kernel_size=(3, 3), stride=1, padding=1)
        self.ds_conv2 = DepthwiseSeparableConv2d(64, 64, kernel_size=(3, 3), stride=1, padding=1)
        self.ds_conv3 = DepthwiseSeparableConv2d(64, 64, kernel_size=(3, 3), stride=1, padding=1)
        self.ds_conv4 = DepthwiseSeparableConv2d(64, 64, kernel_size=(3, 3), stride=1, padding=1)

        # Global average pooling
        self.avgpool = nn.AdaptiveAvgPool2d(1)

        # Fully connected layer
        self.fc = nn.Linear(64, num_classes)

    def forward(self, x):
        # Input: (batch, 1, 49, 10)
        x = self.conv1(x)
        x = self.bn1(x)
        x = self.relu(x)

        x = self.ds_conv1(x)
        x = self.ds_conv2(x)
        x = self.ds_conv3(x)
        x = self.ds_conv4(x)

        x = self.avgpool(x)
        x = torch.flatten(x, 1)
        x = self.fc(x)

        return x


class KWSModel(EagerModelBase):
    """
    ExecutorTorch-compatible Keyword Spotting model wrapper.

    Use with aot_arm_compiler.py:
        python -m executorch.examples.arm.aot_arm_compiler \
            --model_name kws \
            --quantize \
            --delegate \
            -t ethos-u55-256 \
            --system_config Ethos_U55_High_End_Embedded \
            --memory_mode Shared_Sram \
            --output kws_u55_256.pte
    """

    def __init__(self, num_classes=12):
        self.num_classes = num_classes
        # Input dimensions: 49 time steps x 10 MFCC features
        self.time_steps = 49
        self.mfcc_features = 10

    def get_eager_model(self) -> torch.nn.Module:
        model = DSCNN_Small(num_classes=self.num_classes)
        model.eval()
        return model

    def get_example_inputs(self):
        """
        Returns example input tensor for tracing.
        Shape: (1, 1, 49, 10) - batch=1, channels=1, time=49, mfcc=10
        """
        # Create input tensor matching the expected shape
        # Using float for export, will be quantized to int8 during quantization
        return (torch.randn(1, 1, self.time_steps, self.mfcc_features),)


# Export labels for reference
KWS_LABELS = [
    "silence",
    "unknown",
    "yes",
    "no",
    "up",
    "down",
    "left",
    "right",
    "on",
    "off",
    "stop",
    "go"
]
