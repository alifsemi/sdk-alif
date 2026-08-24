#!/usr/bin/env python3

# Copyright Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

"""
Host-side pytest for USB MSC SD after the device firmware is running.

    sudo pytest test_usb_msc_sd_host.py -v \
        --mount-point /mnt/usb_msc --block-dev /dev/sdb
"""

import hashlib
import os
import shutil
import subprocess
import tempfile
import time

import pytest

USB_VID = "2fe3"
USB_PID = "0008"
USB_VID_PID = f"{USB_VID}:{USB_PID}"


def pytest_addoption(parser):
    parser.addoption("--mount-point", default="/mnt/usb_msc",
                     help="Host mount point for the USB MSC drive")
    parser.addoption("--block-dev", default="/dev/sdb",
                     help="Block device for the USB MSC drive")


@pytest.fixture(scope="session")
def mount_point(request):
    return request.config.getoption("--mount-point")


@pytest.fixture(scope="session")
def block_dev(request):
    return request.config.getoption("--block-dev")


def _run(argv, **kwargs):
    return subprocess.run(
        argv, capture_output=True, text=True, timeout=30, **kwargs
    )


def _sync():
    subprocess.run(["sync"], check=False)


def _findmnt_source(mount_point):
    r = _run(["findmnt", "-n", "-o", "SOURCE", "--target", mount_point])
    return r.stdout.strip() if r.returncode == 0 else ""


def _source_matches_block(source, block_dev):
    if not source or not block_dev:
        return False
    src = os.path.realpath(source)
    blk = os.path.realpath(block_dev)
    if src == blk:
        return True
    if not src.startswith(blk):
        return False
    rest = src[len(blk):]
    return rest.isdigit() or (len(rest) > 1 and rest[0] == "p"
                              and rest[1:].isdigit())


def _assert_mounted_block(mount_point, block_dev):
    source = _findmnt_source(mount_point)
    assert _source_matches_block(source, block_dev), (
        f"{mount_point} is mounted from {source!r}, expected {block_dev}"
    )


def _secure_temp_path(suffix=".bin"):
    fd, path = tempfile.mkstemp(prefix="msc_host_", suffix=suffix)
    os.close(fd)
    return path


def _md5(path):
    digest = hashlib.md5()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _remove(path):
    if os.path.lexists(path):
        if os.path.isdir(path) and not os.path.islink(path):
            os.rmdir(path)
        else:
            os.remove(path)
        _sync()


@pytest.fixture(scope="session", autouse=True)
def ensure_mounted(mount_point, block_dev):
    assert os.path.exists(block_dev), f"Block device {block_dev} not found"
    os.makedirs(mount_point, exist_ok=True)

    source = _findmnt_source(mount_point)
    if source:
        assert _source_matches_block(source, block_dev), (
            f"{mount_point} is mounted from {source}, not {block_dev}"
        )
    else:
        r = _run(["mount", block_dev, mount_point])
        assert r.returncode == 0, (
            f"mount {block_dev} on {mount_point} failed: {r.stderr}"
        )
        time.sleep(1)
        _assert_mounted_block(mount_point, block_dev)
    yield
    _sync()


def test_usb_enumeration(mount_point, block_dev):
    r = _run(["lsusb", "-d", USB_VID_PID])
    assert r.returncode == 0 and USB_VID in r.stdout, (
        f"USB device {USB_VID_PID} not found"
    )

    r = _run(["lsusb", "-v", "-d", USB_VID_PID])
    assert f"idVendor           0x{USB_VID}" in r.stdout
    assert f"idProduct          0x{USB_PID}" in r.stdout
    assert ("Mass Storage" in r.stdout or
            "bInterfaceClass         8" in r.stdout), (
        "MSC interface class not found"
    )

    dmesg = _run(["dmesg"]).stdout.lower()
    assert "mass storage" in dmesg or "usb-storage" in dmesg, (
        "No Mass Storage entries in dmesg"
    )

    _assert_mounted_block(mount_point, block_dev)
    os.listdir(mount_point)


def test_file_read_write_delete(mount_point):
    path = os.path.join(mount_point, "host_rwd_test.txt")
    data = "USB MSC SD host read/write/delete test\n"
    try:
        with open(path, "w") as f:
            f.write(data)
        _sync()
        with open(path, "r") as f:
            assert f.read() == data
        os.remove(path)
        _sync()
        assert not os.path.exists(path)
    finally:
        _remove(path)


def test_directory_operations(mount_point):
    dir_path = os.path.join(mount_point, "host_test_dir")
    nested = os.path.join(dir_path, "nested.txt")
    try:
        os.makedirs(dir_path, exist_ok=True)
        with open(nested, "w") as f:
            f.write("nested file\n")
        _sync()
        assert os.path.isdir(dir_path)
        assert os.path.isfile(nested)
    finally:
        _remove(nested)
        _remove(dir_path)


def test_file_transfer_integrity(mount_point):
    src = _secure_temp_path(".src.bin")
    rb = _secure_temp_path(".rb.bin")
    dst = os.path.join(mount_point, "host_large_test.bin")
    try:
        with open(src, "wb") as f:
            f.write(os.urandom(10 * 1024 * 1024))
        src_md5 = _md5(src)
        shutil.copy2(src, dst)
        _sync()
        shutil.copy2(dst, rb)
        assert src_md5 == _md5(rb), "Large file MD5 mismatch"
    finally:
        _remove(src)
        _remove(rb)
        _remove(dst)

    files = []
    try:
        for i in range(1, 21):
            path = os.path.join(mount_point, f"host_sf_{i}.txt")
            content = f"File content {i}\n"
            with open(path, "w") as f:
                f.write(content)
            files.append((path, content))
        _sync()
        for path, expected in files:
            with open(path, "r") as f:
                assert f.read() == expected, f"Mismatch in {path}"
    finally:
        for path, _ in files:
            _remove(path)


def test_lfn_support(mount_point):
    path = os.path.join(mount_point, "this_is_a_long_filename_host_test.txt")
    data = "LFN test content from host\n"
    try:
        with open(path, "w") as f:
            f.write(data)
        _sync()
        with open(path, "r") as f:
            assert f.read() == data
    finally:
        _remove(path)


def test_overwrite_eject_persistence(mount_point, block_dev):
    path = os.path.join(mount_point, "host_oep_test.txt")
    try:
        with open(path, "w") as f:
            f.write("original content\n")
        _sync()
        with open(path, "w") as f:
            f.write("new overwritten content\n")
        _sync()
        with open(path, "r") as f:
            content = f.read()
        assert content == "new overwritten content\n"
        assert "original" not in content

        r = _run(["umount", mount_point])
        assert r.returncode == 0, r.stderr
        time.sleep(1)
        r = _run(["mount", block_dev, mount_point])
        assert r.returncode == 0, r.stderr
        time.sleep(1)
        _assert_mounted_block(mount_point, block_dev)

        with open(path, "r") as f:
            assert f.read() == "new overwritten content\n"
    finally:
        _remove(path)


def test_alignment(mount_point):
    files = []
    try:
        for size in (1, 511, 512, 513, 1024):
            path = os.path.join(mount_point, f"host_align_{size}.bin")
            data = os.urandom(size)
            with open(path, "wb") as f:
                f.write(data)
            files.append((path, data))
        _sync()
        for path, expected in files:
            assert os.stat(path).st_size == len(expected)
            with open(path, "rb") as f:
                assert f.read() == expected
    finally:
        for path, _ in files:
            _remove(path)


def test_stress_file_io(mount_point):
    files = []
    try:
        for i in range(1, 51):
            path = os.path.join(mount_point, f"host_stress_{i}.bin")
            data = os.urandom(100 * 1024)
            with open(path, "wb") as f:
                f.write(data)
            files.append((path, data))
        _sync()
        for path, expected in files:
            with open(path, "rb") as f:
                assert f.read() == expected
    finally:
        for path, _ in files:
            _remove(path)
