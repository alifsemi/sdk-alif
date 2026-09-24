#!/usr/bin/env python3

# Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

"""Install the CMSIS-NN wheel pinned by this ExecuTorch revision.

examples/arm/setup.sh installs

    cmsis_nn @ git+https://github.com/ARM-software/CMSIS-NN.git@<commit>

with pip. That commit's pyproject.toml still says cmake.minimum-version and
requires scikit-build-core>=0.7 with no upper bound. A populated pip cache
replays the previously built wheel. A fresh user, cache, or machine resolves
scikit-build-core 1.x, which rejects the old key:

    ERROR: Use cmake.version instead of cmake.minimum-version
           with scikit-build-core >= 0.8

ARM corrected the build metadata in CMSIS-NN cf08f672 (#232). Later commits
also change the C library. The on-device FetchContent pin and this Python
package must stay on the same commit, so this script checks out the pin from
requirements-cortex-m.txt, applies only that metadata correction, and
installs the tree. setup.sh must be invoked with --disable-cortex-m-deps.
"""

import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path
from urllib.parse import unquote, urlparse


_CMSIS_NN_REQ_RE = re.compile(
    r'cmsis_nn\s*@\s*git\+'
    r'(?P<url>https://github\.com/ARM-software/CMSIS-NN\.git)'
    r'@(?P<commit>[0-9a-fA-F]{40})\s*$'
)

# Metadata edit from CMSIS-NN cf08f6728f1a39eb551c6e48c6058a346284fecb.
_OLD_BUILD_REQUIRES = 'requires = ["scikit-build-core>=0.7", "pybind11>=2.10"]'
_NEW_BUILD_REQUIRES = (
    'requires = ["scikit-build-core>=0.8,<1", '
    '"pybind11>=2.10,<3"]'
)
_OLD_CMAKE_KEY = 'cmake.minimum-version = "3.15"'
_NEW_CMAKE_KEY = 'cmake.version = ">=3.15"'


def _log(message):
    print(f'[cmsis-nn] {message}')


def _run(cmd, cwd=None):
    _log(' '.join(cmd))
    env = os.environ.copy()
    env['GIT_TERMINAL_PROMPT'] = '0'
    subprocess.run(cmd, cwd=cwd, env=env, check=True)


def _git_head(repo):
    result = subprocess.run(
        ['git', '-C', str(repo), 'rev-parse', 'HEAD'],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        return None
    return result.stdout.strip()


def _requirements_file(executorch_dir):
    candidates = [
        executorch_dir / 'backends' / 'cortex_m' / 'requirements-cortex-m.txt',
        executorch_dir / 'src' / 'executorch' / 'backends' / 'cortex_m'
        / 'requirements-cortex-m.txt',
    ]
    for path in candidates:
        if path.is_file():
            return path
    raise FileNotFoundError(
        f'Could not find requirements-cortex-m.txt under {executorch_dir}'
    )


def _read_pin(req_file):
    for raw in req_file.read_text().splitlines():
        line = raw.split('#', 1)[0].strip()
        if not line.startswith('cmsis_nn'):
            continue
        match = _CMSIS_NN_REQ_RE.match(line)
        if not match:
            raise ValueError(f'Unrecognized cmsis_nn requirement: {raw!r}')
        return match.group('url'), match.group('commit')
    raise ValueError(f'No cmsis_nn requirement in {req_file}')


def _patch_pyproject(repo):
    pyproject = repo / 'pyproject.toml'
    text = pyproject.read_text()
    if _OLD_CMAKE_KEY in text:
        if _OLD_BUILD_REQUIRES not in text:
            raise RuntimeError(
                f'{pyproject} uses cmake.minimum-version but not the expected '
                'build-system.requires line; refusing to guess a patch.'
            )
        text = text.replace(_OLD_BUILD_REQUIRES, _NEW_BUILD_REQUIRES, 1)
        text = text.replace(_OLD_CMAKE_KEY, _NEW_CMAKE_KEY, 1)
        pyproject.write_text(text)
        _log(
            'Patched pyproject.toml: cmake.minimum-version -> cmake.version, '
            'scikit-build-core>=0.8,<1'
        )
        return
    if 'cmake.version' in text:
        _log('pyproject.toml already uses cmake.version')
        return
    raise RuntimeError(
        f'{pyproject} has no cmake.version or cmake.minimum-version setting; '
        'refusing to build CMSIS-NN.'
    )


def _target_python():
    """Python that should receive cmsis_nn.

    setup.sh installs with pip from PATH. An activated virtualenv puts that
    pip on PATH even when this process is a system west.
    """
    venv = os.environ.get('VIRTUAL_ENV')
    if venv:
        for name in ('python', 'python3'):
            candidate = Path(venv) / 'bin' / name
            if os.access(candidate, os.X_OK):
                return str(candidate)
    return sys.executable


def _imports(python):
    result = subprocess.run(
        [python, '-c', 'import cmsis_nn'],
        capture_output=True,
        text=True,
    )
    return result.returncode == 0


def _installed_source_commit(python):
    """CMSIS-NN commit the installed cmsis_nn wheel was built from.

    Empty when the package is missing or the commit cannot be recovered.
    A git install records the commit in direct_url.json. An install from
    the checkout created here records a file:// URL; the commit is that
    checkout's HEAD.
    """
    result = subprocess.run(
        [
            python, '-c',
            'import importlib.metadata as m, sys\n'
            'try:\n'
            "    dist = m.distribution('cmsis_nn')\n"
            'except m.PackageNotFoundError:\n'
            '    raise SystemExit(2)\n'
            "sys.stdout.write(dist.read_text('direct_url.json') or '')\n",
        ],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0 or not result.stdout.strip():
        return ''
    info = json.loads(result.stdout)
    commit = (info.get('vcs_info') or {}).get('commit_id') or ''
    if commit:
        return commit
    url = info.get('url') or ''
    if url.startswith('file:'):
        path = Path(unquote(urlparse(url).path))
        return _git_head(path) or ''
    return ''


def _ensure_checkout(url, commit, dest):
    if dest.exists() and not (dest / '.git').is_dir():
        shutil.rmtree(dest)
    if not dest.exists():
        dest.mkdir(parents=True)
        _run(['git', 'init'], cwd=dest)
        _run(['git', 'remote', 'add', 'origin', url], cwd=dest)
    else:
        _run(['git', 'remote', 'set-url', 'origin', url], cwd=dest)

    if _git_head(dest) == commit:
        return

    try:
        _run(['git', 'fetch', '--depth', '1', 'origin', commit], cwd=dest)
    except subprocess.CalledProcessError:
        _log('Shallow fetch failed; fetching the pinned commit without --depth')
        _run(['git', 'fetch', 'origin', commit], cwd=dest)
    _run(
        [
            'git', '-c', 'advice.detachedHead=false',
            'checkout', '--force', 'FETCH_HEAD',
        ],
        cwd=dest,
    )
    head = _git_head(dest)
    if head != commit:
        raise RuntimeError(f'CMSIS-NN checkout is {head}, expected {commit}')


def install(executorch_dir):
    """Install the pinned cmsis_nn wheel. Return 0 on success."""
    try:
        req_file = _requirements_file(executorch_dir)
        url, commit = _read_pin(req_file)
        python = _target_python()
        installed = _installed_source_commit(python)
        if installed == commit and _imports(python):
            _log(
                f'cmsis_nn already installed from {commit[:12]}; '
                'skipping source build'
            )
            return 0

        dest = executorch_dir / 'examples' / 'arm' / 'arm-scratch' / 'cmsis-nn'
        _log(f'Checking out CMSIS-NN {commit} (pinned by {req_file.name})')
        _ensure_checkout(url, commit, dest)
        _patch_pyproject(dest)
        _log('Building and installing cmsis_nn from the patched checkout')
        _run([
            python, '-m', 'pip', 'install',
            '--no-dependencies',
            '--upgrade',
            '--force-reinstall',
            str(dest),
        ])
        if not _imports(python):
            raise RuntimeError('cmsis_nn installed but cannot be imported')
        _log('cmsis_nn installed')
        return 0
    except (OSError, ValueError, RuntimeError,
            subprocess.CalledProcessError) as exc:
        print(f'[cmsis-nn] ERROR: {exc}', file=sys.stderr)
        return 1


def main(argv):
    if len(argv) != 1:
        print('usage: install_cmsis_nn.py <executorch-dir>', file=sys.stderr)
        return 2
    return install(Path(argv[0]).resolve())


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
