#!/usr/bin/env python3

# Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

"""West extension command to setup executorch with submodules and scripts."""

import os
import subprocess
import sys
from pathlib import Path
from west.commands import WestCommand
from west import log

# Import the apply overrides script
script_dir = Path(__file__).parent
sys.path.insert(0, str(script_dir))
import apply_executorch_overrides


class ExecutorchSetup(WestCommand):
    def __init__(self):
        super().__init__(
            'executorch-setup',
            'setup executorch submodules and run installation scripts',
            'Initialize executorch git submodules and execute setup scripts')

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(
            self.name,
            help=self.help,
            description=self.description)
        parser.add_argument(
            '--fvp',
            action='store_true',
            default=False,
            help='Also install Arm Corstone FVP tools (examples/arm/setup.sh). '
                 'Not needed for kws_ethosu on real Alif hardware.')
        return parser

    def do_run(self, args, unknown_args):
        self._ensure_venv_west()

        workspace_root = Path(self.topdir)
        executorch_path = workspace_root / 'modules' / 'lib' / 'executorch'

        if not executorch_path.exists():
            log.err(f'Executorch path not found: {executorch_path}')
            log.err('Please run "west update" first to clone executorch')
            return 1

        log.inf(f'Setting up executorch at: {executorch_path}')

        # Step 1: Initialize git submodules
        log.inf('Initializing git submodules...')
        try:
            subprocess.run(
                ['git', 'submodule', 'update', '--init', '--recursive'],
                cwd=executorch_path,
                check=True,
                capture_output=False
            )
            log.inf('Git submodules initialized successfully')
        except subprocess.CalledProcessError as e:
            log.err(f'Failed to initialize git submodules: {e}')
            return 1

        # Step 2: Apply Alif overrides to the ExecuTorch *source* tree.
        # This must happen before install_executorch.sh so that the patched
        # examples/models/__init__.py is present when the package is built.
        log.inf('Applying Alif-specific overrides to source tree...')
        try:
            result = apply_executorch_overrides.apply_overrides(workspace_root)
            if result != 0:
                log.err('Failed to apply Alif overrides')
                return 1
            log.inf('Alif overrides applied successfully')
        except Exception as e:
            log.err(f'Failed to apply Alif overrides: {e}')
            return 1

        # Step 3: Install the executorch Python package into the venv.
        # install_executorch.sh installs the stable torch==2.12.0 and builds
        # the executorch wheel; we must NOT modify torch_pin.py before this.
        install_script = executorch_path / 'install_executorch.sh'
        if install_script.exists():
            log.inf('Running install_executorch.sh...')
            try:
                subprocess.run(
                    ['bash', str(install_script)],
                    cwd=executorch_path,
                    check=True,
                    capture_output=False
                )
                log.inf('install_executorch.sh completed successfully')
            except subprocess.CalledProcessError as e:
                log.err(f'Failed to run install_executorch.sh: {e}')
                return 1
        else:
            log.wrn(f'install_executorch.sh not found at {install_script}')

        # Step 4: Copy KWS files into the now-installed venv package so that
        # 'python -m ...aot_arm_compiler --model_name=kws' works without
        # needing PYTHONPATH tricks.
        try:
            apply_executorch_overrides.copy_kws_to_venv(workspace_root)
        except Exception as e:
            log.wrn(f'Could not copy KWS files to installed package: {e}')

        # Step 5: Install Arm backend runtime deps via examples/arm/setup.sh.
        # This is ALWAYS required for aot_arm_compiler to work (e.g. generating
        # .pte files with --delegate -t ethos-u55-128).  It installs:
        #   - tosa_serializer  (built from tosa-tools/serialization, not on PyPI)
        #   - ethos-u-vela     (Ethos-U compiler, needed for --delegate)
        # We pass --disable-ethos-u-deps to skip FVP/toolchain downloads, then
        # --enable-vela to re-enable only the vela+tosa pieces.
        arm_setup_script = executorch_path / 'examples' / 'arm' / 'setup.sh'
        if arm_setup_script.exists():
            log.inf('Running examples/arm/setup.sh (tosa_serializer + ethos-u-vela)...')
            try:
                subprocess.run(
                    [
                        'bash', str(arm_setup_script),
                        '--disable-ethos-u-deps',
                        '--enable-vela',
                    ],
                    cwd=executorch_path / 'examples' / 'arm',
                    check=True,
                    capture_output=False
                )
                log.inf('ARM backend deps installed successfully')
            except subprocess.CalledProcessError as e:
                log.err(f'Failed to install ARM backend deps: {e}')
                return 1
        else:
            log.wrn(f'examples/arm/setup.sh not found at {arm_setup_script}')

        # Step 6: Run examples/arm/setup.sh with FVP/baremetal toolchain (OPTIONAL).
        # Only needed for simulation with Arm Corstone FVP, not for real hardware.
        if getattr(args, 'fvp', False):
            if arm_setup_script.exists():
                log.inf('Running examples/arm/setup.sh (FVP tools)...')
                try:
                    subprocess.run(
                        ['bash', str(arm_setup_script), '--i-agree-to-the-contained-eula'],
                        cwd=executorch_path / 'examples' / 'arm',
                        check=True,
                        capture_output=False
                    )
                    log.inf('examples/arm/setup.sh (FVP) completed successfully')
                except subprocess.CalledProcessError as e:
                    log.err(f'Failed to run examples/arm/setup.sh (FVP): {e}')
                    return 1
            else:
                log.wrn(f'examples/arm/setup.sh not found at {arm_setup_script}')
        else:
            log.inf('Skipping Arm FVP setup (not needed for real hardware).')
            log.inf('Re-run with --fvp to install Arm Corstone FVP tools.')

        log.inf('Executorch setup completed successfully!')
        return 0

    def _ensure_venv_west(self):
        """Warn if not in a venv; auto-install west into the venv if missing."""
        venv_path = os.environ.get('VIRTUAL_ENV')

        if not venv_path:
            log.wrn('=' * 70)
            log.wrn('WARNING: No virtual environment detected!')
            log.wrn('It is recommended to run this setup inside a Python virtual environment.')
            log.wrn('Create and activate a venv with:')
            log.wrn('  python3 -m venv .zephyr_venv')
            log.wrn('  source .zephyr_venv/bin/activate')
            log.wrn('=' * 70)
            return

        # Check if west is installed in the venv
        west_in_venv = Path(venv_path) / 'bin' / 'west'
        if not west_in_venv.exists():
            log.wrn('=' * 70)
            log.wrn('West is not installed in the current virtual environment!')
            log.wrn(f'Virtual environment: {venv_path}')
            log.wrn(f'West location: {sys.argv[0]}')
            log.wrn('')
            log.inf('Installing west in the virtual environment...')
            log.wrn('=' * 70)

            # Auto-install west in the venv using the venv's pip
            venv_pip = Path(venv_path) / 'bin' / 'pip'
            if not venv_pip.exists():
                log.err(f'Venv pip not found at {venv_pip}')
                log.err('Please manually install west with: pip install west')
                return

            try:
                subprocess.run(
                    [str(venv_pip), 'install', 'west'],
                    check=True,
                    capture_output=False
                )
                log.inf('West successfully installed in the virtual environment!')
                log.inf('Note: You may need to use the venv west for future commands.')
                log.inf(f'Venv west location: {west_in_venv}')
            except subprocess.CalledProcessError as e:
                log.err(f'Failed to install west: {e}')
                log.err('Please manually install west with: pip install west')
                return
        else:
            log.inf(f'Virtual environment detected: {venv_path}')
            log.inf('West is correctly installed in the virtual environment.')
