#!/usr/bin/env python3
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

        # Step 2: Run install_executorch.sh
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

        # Step 3: Run examples/arm/setup.sh with EULA agreement
        arm_setup_script = executorch_path / 'examples' / 'arm' / 'setup.sh'
        if arm_setup_script.exists():
            log.inf('Running examples/arm/setup.sh...')
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
        else:
            log.wrn(f'examples/arm/setup.sh not found at {arm_setup_script}')

        # Step 4: Apply Alif-specific overrides
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

        log.inf('Executorch setup completed successfully!')
        return 0
