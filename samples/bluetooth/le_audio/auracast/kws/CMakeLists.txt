# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../src)

target_sources(app PRIVATE
    KwsMain.cpp
    KwsModel.cpp
    kws_micronet_m_vela_H128.tflite.cpp
    LiveMicInput.cpp
    mfcc/KwsProcessing.cpp
    mfcc/Mfcc.cpp
    mfcc/PlatformMath.cpp
)
