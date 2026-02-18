#!/bin/bash
# Post-update script to setup executorch after west update
# Usage: Run this script after "west update" to setup executorch

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
EXECUTORCH_PATH="$WORKSPACE_ROOT/modules/lib/executorch"

echo "=== Executorch Post-Update Setup ==="
echo "Workspace root: $WORKSPACE_ROOT"
echo "Executorch path: $EXECUTORCH_PATH"

if [ ! -d "$EXECUTORCH_PATH" ]; then
    echo "ERROR: Executorch directory not found at $EXECUTORCH_PATH"
    echo "Please run 'west update' first"
    exit 1
fi

cd "$EXECUTORCH_PATH"

echo ""
echo "Step 1: Initializing git submodules..."
git submodule update --init --recursive

echo ""
echo "Step 2: Running install_executorch.sh..."
if [ -f "./install_executorch.sh" ]; then
    ./install_executorch.sh
else
    echo "WARNING: install_executorch.sh not found"
fi

echo ""
echo "Step 3: Running examples/arm/setup.sh..."
if [ -f "./examples/arm/setup.sh" ]; then
    cd examples/arm
    ./setup.sh --i-agree-to-the-contained-eula
else
    echo "WARNING: examples/arm/setup.sh not found"
fi

echo ""
echo "=== Executorch setup completed successfully! ==="
