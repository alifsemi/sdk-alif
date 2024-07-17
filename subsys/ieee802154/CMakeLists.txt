# Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license


add_subdirectory_ifdef(CONFIG_IEEE802154_ALIF_AHI_MSG_LIB msglib/)
add_subdirectory_ifdef(CONFIG_IEEE802154_ALIF_SHELL shell/)
