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
        return parser

    def do_run(self, args, unknown_args):
        # Check if running in a virtual environment
        self._check_virtual_environment()

        # Get the workspace root (where .west directory is located)
        workspace_root = Path(self.topdir)
        executorch_path = workspace_root / 'modules' / 'lib' / 'executorch'

        if not executorch_path.exists():
            log.err(f'Executorch path not found: {executorch_path}')
            log.err('Please run "west update" first to clone executorch')
            return 1

        log.inf(f'Setting up executorch at: {executorch_path}')

        # Step 1: Initialize git submodules recursively
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

        # Step 2: Apply Alif-specific overrides (including torch version patches)
        log.inf('Applying Alif-specific overrides...')
        try:
            result = apply_executorch_overrides.apply_overrides(workspace_root)
            if result != 0:
                log.err('Failed to apply Alif overrides')
                return 1
            log.inf('Alif overrides applied successfully')
        except Exception as e:
            log.err(f'Failed to apply Alif overrides: {e}')
            return 1

        # Step 3: Run install_executorch.sh
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

        # Step 4: Run examples/arm/setup.sh with user EULA acceptance
        arm_setup_script = executorch_path / 'examples' / 'arm' / 'setup.sh'
        if arm_setup_script.exists():
            log.wrn('=' * 70)
            log.wrn('IMPORTANT: Arm Corstone FVP EULA Acceptance Required')
            log.wrn('=' * 70)
            log.inf('')
            log.inf('The next step will download and install Arm\'s Corstone Fixed')
            log.inf('Virtual Platform (FVP), which requires accepting Arm\'s EULA.')
            log.inf('')
            log.inf('The EULA will be displayed during installation.')
            log.inf('You must review and accept it to proceed.')
            log.inf('')

            # Prompt user for acceptance
            while True:
                response = input('Do you want to continue and review the EULA? (yes/no): ').strip().lower()
                if response in ['yes', 'y']:
                    log.inf('')
                    log.inf('Proceeding with setup...')
                    try:
                        subprocess.run(
                            ['bash', str(arm_setup_script), '--i-agree-to-the-contained-eula'],
                            cwd=executorch_path / 'examples' / 'arm',
                            check=True,
                            capture_output=False
                        )
                        log.inf('examples/arm/setup.sh completed successfully')
                    except subprocess.CalledProcessError as e:
                        log.err(f'Failed to run examples/arm/setup.sh: {e}')
                        return 1
                    break
                elif response in ['no', 'n']:
                    log.inf('')
                    log.wrn('EULA declined. Skipping Arm FVP setup.')
                    log.wrn('Note: Some ExecutorTorch ARM features may not be available.')
                    break
                else:
                    log.wrn('Please answer "yes" or "no".')
        else:
            log.wrn(f'examples/arm/setup.sh not found at {arm_setup_script}')

        log.inf('Executorch setup completed successfully!')
        return 0

    def _check_virtual_environment(self):
        """Check if running in a virtual environment and if west is installed in it."""
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
