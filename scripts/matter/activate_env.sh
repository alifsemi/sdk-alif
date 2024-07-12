# Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

#!/bin/bash

PWD_ORG=`pwd`

cd ../modules/lib/matter
# Activate Matter virtual Environment
source scripts/activate.sh

echo "Build a Host tools"

# Build without wifi interface. This is because of lack of Global Ipv6 at wifi interface.
gn gen out/host --args='chip_enable_wifi=false'
ninja -C out/host

HOST_PATH=`pwd`
HOST_PATH=$HOST_PATH/out/host

echo "Add to host tools to path:"
echo $HOST_PATH
export PATH=$HOST_PATH:$PATH

# go back original folder
cd $PWD_ORG
