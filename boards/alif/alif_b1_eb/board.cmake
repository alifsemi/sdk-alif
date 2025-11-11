# Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

board_runner_args(alif_flash "--device=AB1C1F4M51820")

include(${ZEPHYR_BASE}/boards/common/alif_flash.board.cmake)

