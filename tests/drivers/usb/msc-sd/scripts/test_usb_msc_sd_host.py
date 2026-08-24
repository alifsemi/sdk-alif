#!/usr/bin/env python3

# Copyright Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

"""
Host-side pytest script for USB MSC SD test verification (consolidated).

Complements the device-side MSC SD ztest suite (8 tests) with host-side verification checks. Runs on the Linux host PC
connected to the Alif E8 DK board via USB.

Usage:
    sudo pytest test_usb_msc_sd_host.py -v \
        --mount-point /mnt/usb_msc --block-dev /dev/sdb

Prerequisites:
    - Board flashed and booted with USB MSC SD firmware
    - USB cable connected between board and host
    - lsusb, dmesg, mount utilities available
    - pytest installed (pip install pytest)
"""

import hashlib
import os
import shlex
import shutil
import subprocess
import tempfile
import time

import pytest

# ─── Configuration ───
USB_VID = "2fe3"
USB_PID = "0008"
USB_VID_PID = f"{USB_VID}:{USB_PID}"


def pytest_addoption(parser):
    parser.addoption("--mount-point", default="/mnt/usb_msc",
                     help="Host mount point for USB MSC drive")
    parser.addoption("--block-dev", default="/dev/sdb",
                     help="Block device for USB MSC drive")


@pytest.fixture(scope="session")
def mount_point(request):
    return request.config.getoption("--mount-point")


@pytest.fixture(scope="session")
def block_dev(request):
    return request.config.getoption("--block-dev")


@pytest.fixture(scope="session", autouse=True)
def ensure_mounted(mount_point, block_dev):
    """Ensure USB MSC drive is mounted before tests."""
    os.makedirs(mount_point, exist_ok=True)
    result = subprocess.run(["mountpoint", "-q", mount_point])
    if result.returncode != 0:
        subprocess.run(
            ["sudo", "mount", block_dev, mount_point],
            check=False, capture_output=True
        )
        time.sleep(1)
    yield
    subprocess.run(["sync"], check=False)


def _run(cmd, **kwargs):
    """Run a shell command and return CompletedProcess."""
    return subprocess.run(
        cmd, shell=True, capture_output=True, text=True,
        timeout=30, **kwargs
    )


def _md5(path):
    """Compute MD5 checksum of a file."""
    h = hashlib.md5()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()


# ========================================================================
# 1. test_build_and_config  (TC-001, TC-002)
# ========================================================================
class TestBuildAndConfig:
    """Verify USB device with expected VID:PID is detected (build ran)."""

    def test_build_and_config(self):
        r = _run(f"lsusb -d {USB_VID_PID}")
        assert r.returncode == 0 and USB_VID in r.stdout, \
            f"USB device {USB_VID_PID} not found — firmware not running?"


# ========================================================================
# 2. test_boot_and_sd_mount  (TC-003, TC-029)
# ========================================================================
class TestBootAndSdMount:
    """Verify drive is mounted and df reports sane stats."""

    def test_boot_and_sd_mount(self, mount_point):
        r = subprocess.run(["mountpoint", "-q", mount_point])
        assert r.returncode == 0, f"{mount_point} not mounted"

        r = _run(f"df -h {shlex.quote(mount_point)}")
        assert r.returncode == 0
        lines = r.stdout.strip().split("\n")
        assert len(lines) >= 2, "df output too short"
        print(f"df output:\n{r.stdout}")


# ========================================================================
# 3. test_usb_init_and_enumeration  (TC-004, TC-005)
# ========================================================================
class TestUsbInitAndEnumeration:
    """USB enumeration, dmesg, block device, mount checks."""

    def test_usb_init_and_enumeration(self, mount_point, block_dev):
        # lsusb
        r = _run(f"lsusb | grep {USB_VID}")
        assert USB_VID in r.stdout, "Device not in lsusb"

        # dmesg mass storage
        r = _run("dmesg | grep -i 'mass storage\\|usb-storage' | tail -5")
        assert r.stdout.strip(), "No Mass Storage entries in dmesg"

        # block device
        assert os.path.exists(block_dev), \
            f"Block device {block_dev} not found"

        # mount point accessible
        entries = os.listdir(mount_point)
        assert isinstance(entries, list)


# ========================================================================
# 4. test_file_read_write_delete  (TC-006, TC-007, TC-009)
# ========================================================================
class TestFileReadWriteDelete:
    """Write, read-back, verify, delete via USB MSC."""

    FILE = "host_rwd_test.txt"
    DATA = "USB MSC SD host read/write/delete test\n"

    def test_file_read_write_delete(self, mount_point):
        path = os.path.join(mount_point, self.FILE)
        try:
            # write
            with open(path, "w") as f:
                f.write(self.DATA)
            subprocess.run(["sync"], check=False)

            # read back
            with open(path, "r") as f:
                content = f.read()
            assert content == self.DATA, "Read-back mismatch"

            # delete
            os.remove(path)
            subprocess.run(["sync"], check=False)
            assert not os.path.exists(path), "File still exists after delete"
        finally:
            if os.path.exists(path):
                os.remove(path)
                subprocess.run(["sync"], check=False)


# ========================================================================
# 5. test_directory_operations  (TC-008)
# ========================================================================
class TestDirectoryOperations:
    """Create dir, nested file, list, remove."""

    DIR = "host_test_dir"

    def test_directory_operations(self, mount_point):
        dir_path = os.path.join(mount_point, self.DIR)
        nested = os.path.join(dir_path, "nested.txt")
        try:
            os.makedirs(dir_path, exist_ok=True)
            with open(nested, "w") as f:
                f.write("nested file\n")
            subprocess.run(["sync"], check=False)
            assert os.path.isdir(dir_path)
            assert os.path.isfile(nested)
        finally:
            if os.path.exists(nested):
                os.remove(nested)
            if os.path.isdir(dir_path):
                os.rmdir(dir_path)
            subprocess.run(["sync"], check=False)


# ========================================================================
# 6. test_file_transfer_integrity  (TC-010, TC-011)
# ========================================================================
class TestFileTransferIntegrity:
    """Large file MD5 integrity + multiple small files."""

    LARGE_MB = 10
    SMALL_COUNT = 20

    def test_file_transfer_integrity(self, mount_point):
        # --- large file ---
        src = os.path.join(tempfile.gettempdir(), "msc_large_src.bin")
        dst = os.path.join(mount_point, "host_large_test.bin")
        rb = os.path.join(tempfile.gettempdir(), "msc_large_rb.bin")
        try:
            _run(f"dd if=/dev/urandom of={src} bs=1M count={self.LARGE_MB}")
            src_md5 = _md5(src)

            shutil.copy2(src, dst)
            subprocess.run(["sync"], check=False)
            time.sleep(1)

            shutil.copy2(dst, rb)
            rb_md5 = _md5(rb)

            assert src_md5 == rb_md5, \
                f"Large file MD5 mismatch: {src_md5} != {rb_md5}"
            print(f"Large file ({self.LARGE_MB} MB) integrity OK")
        finally:
            for p in [src, rb]:
                if os.path.exists(p):
                    os.remove(p)
            if os.path.exists(dst):
                os.remove(dst)
                subprocess.run(["sync"], check=False)

        # --- multiple small files ---
        files = []
        try:
            for i in range(1, self.SMALL_COUNT + 1):
                name = f"host_sf_{i}.txt"
                path = os.path.join(mount_point, name)
                content = f"File content {i}\n"
                with open(path, "w") as f:
                    f.write(content)
                files.append((path, content))
            subprocess.run(["sync"], check=False)

            for path, expected in files:
                with open(path, "r") as f:
                    actual = f.read()
                assert actual == expected, f"Mismatch in {path}"
            print(f"Small files ({self.SMALL_COUNT}) integrity OK")
        finally:
            for path, _ in files:
                if os.path.exists(path):
                    os.remove(path)
            subprocess.run(["sync"], check=False)


# ========================================================================
# 7. test_usb_disconnect_reconnect  (TC-012, TC-013)
#    Host cannot programmatically disconnect USB — manual only.
# ========================================================================
class TestUsbDisconnectReconnect:
    """Placeholder — USB disconnect/reconnect is device-side + manual."""

    def test_usb_disconnect_reconnect(self):
        pytest.skip("USB disconnect/reconnect is device-side / manual")


# ========================================================================
# 8. test_lfn_support  (TC-015)
# ========================================================================
class TestLfnSupport:
    """Write and read a file with a long name."""

    LFN = "this_is_a_long_filename_host_test.txt"

    def test_lfn_support(self, mount_point):
        path = os.path.join(mount_point, self.LFN)
        data = "LFN test content from host\n"
        try:
            with open(path, "w") as f:
                f.write(data)
            subprocess.run(["sync"], check=False)
            assert os.path.exists(path), "LFN file not found"
            with open(path, "r") as f:
                content = f.read()
            assert content == data
        finally:
            if os.path.exists(path):
                os.remove(path)
                subprocess.run(["sync"], check=False)


# ========================================================================
# 9. test_overwrite_eject_persistence  (TC-016, TC-017, TC-023)
# ========================================================================
class TestOverwriteEjectPersistence:
    """Overwrite file, unmount/remount, verify data persisted."""

    def test_overwrite_eject_persistence(self, mount_point, block_dev):
        path = os.path.join(mount_point, "host_oep_test.txt")
        try:
            # overwrite
            with open(path, "w") as f:
                f.write("original content\n")
            subprocess.run(["sync"], check=False)

            with open(path, "w") as f:
                f.write("new overwritten content\n")
            subprocess.run(["sync"], check=False)

            with open(path, "r") as f:
                content = f.read()
            assert content == "new overwritten content\n"
            assert "original" not in content

            # eject + persistence
            subprocess.run(["sudo", "umount", mount_point],
                           check=True, capture_output=True)
            time.sleep(1)
            subprocess.run(["sudo", "mount", block_dev, mount_point],
                           check=True, capture_output=True)
            time.sleep(1)

            with open(path, "r") as f:
                content = f.read()
            assert content == "new overwritten content\n", \
                "Data lost after eject/remount"
        finally:
            if os.path.exists(path):
                os.remove(path)
                subprocess.run(["sync"], check=False)


# ========================================================================
# 10. test_usb_descriptors_and_wp  (TC-018, TC-019)
# ========================================================================
class TestUsbDescriptorsAndWp:
    """VID/PID, MSC interface class, SCSI inquiry, write-protect."""

    def test_usb_descriptors_and_wp(self, mount_point, block_dev):
        # VID / PID
        r = _run(f"lsusb -v -d {USB_VID_PID} 2>/dev/null")
        assert f"idVendor           0x{USB_VID}" in r.stdout
        assert f"idProduct          0x{USB_PID}" in r.stdout

        # Interface class 08 (Mass Storage)
        assert "Mass Storage" in r.stdout or \
               "bInterfaceClass         8" in r.stdout, \
            "MSC interface class not found"

        # SCSI inquiry (optional — sg_inq may not be installed)
        r2 = _run(f"sudo sg_inq {block_dev} 2>/dev/null")
        if r2.returncode == 0:
            assert "Zephyr" in r2.stdout or "ZEPHYR" in r2.stdout, \
                f"Vendor 'Zephyr' not in SCSI inquiry"

        # Write-protect off
        r3 = _run("dmesg | grep -i 'write protect' | tail -3")
        if r3.stdout.strip():
            assert "off" in r3.stdout.lower(), "Write protect may be on"

        # Confirm writable
        wp_path = os.path.join(mount_point, "host_wp_test.txt")
        try:
            with open(wp_path, "w") as f:
                f.write("wp test\n")
            subprocess.run(["sync"], check=False)
            assert os.path.exists(wp_path)
        finally:
            if os.path.exists(wp_path):
                os.remove(wp_path)
                subprocess.run(["sync"], check=False)


# ========================================================================
# 11. test_stress_usb_and_file_io  (TC-020, TC-030)
# ========================================================================
class TestStressUsbAndFileIo:
    """Stress: 50 x 100 KB files + concurrent write/list."""

    STRESS_COUNT = 50
    STRESS_KB = 100

    def test_stress_usb_and_file_io(self, mount_point):
        # --- stress files ---
        files = []
        try:
            for i in range(1, self.STRESS_COUNT + 1):
                path = os.path.join(mount_point, f"host_stress_{i}.bin")
                data = os.urandom(self.STRESS_KB * 1024)
                with open(path, "wb") as f:
                    f.write(data)
                files.append(path)
            subprocess.run(["sync"], check=False)

            assert len(files) == self.STRESS_COUNT
            for path in files:
                with open(path, "rb") as f:
                    _ = f.read()
            print(f"Stress: {self.STRESS_COUNT} files OK")
        finally:
            for path in files:
                if os.path.exists(path):
                    os.remove(path)
            subprocess.run(["sync"], check=False)

        # --- concurrent write + list ---
        src = os.path.join(tempfile.gettempdir(), "msc_conc_src.bin")
        dst = os.path.join(mount_point, "host_concurrent.bin")
        try:
            _run(f"dd if=/dev/urandom of={src} bs=1M count=5")
            proc = subprocess.Popen(
                ["cp", src, dst],
                stdout=subprocess.PIPE, stderr=subprocess.PIPE
            )
            time.sleep(0.5)
            entries = os.listdir(mount_point)
            assert isinstance(entries, list)
            proc.wait(timeout=60)
            subprocess.run(["sync"], check=False)
            assert proc.returncode == 0, "Background copy failed"
        finally:
            if os.path.exists(src):
                os.remove(src)
            if os.path.exists(dst):
                os.remove(dst)
                subprocess.run(["sync"], check=False)


# ========================================================================
# 12. test_adma_and_alignment  (TC-021, TC-022)
# ========================================================================
class TestAdmaAndAlignment:
    """Write files at 512-byte boundary sizes and verify."""

    SIZES = [1, 511, 512, 513, 1024]

    def test_adma_and_alignment(self, mount_point):
        files = []
        try:
            for size in self.SIZES:
                path = os.path.join(mount_point, f"host_align_{size}.bin")
                data = os.urandom(size)
                with open(path, "wb") as f:
                    f.write(data)
                files.append((path, data))
            subprocess.run(["sync"], check=False)

            for path, expected in files:
                stat = os.stat(path)
                assert stat.st_size == len(expected), \
                    f"Size mismatch for {path}"
                with open(path, "rb") as f:
                    actual = f.read()
                assert actual == expected, f"Data mismatch for {path}"
        finally:
            for path, _ in files:
                if os.path.exists(path):
                    os.remove(path)
            subprocess.run(["sync"], check=False)


# ========================================================================
# 13. test_regulator_init_order  (TC-028)
#     Device-side only — host just confirms drive works.
# ========================================================================
class TestRegulatorInitOrder:
    """Confirm drive is functional (regulator/SDHC ordering OK)."""

    def test_regulator_init_order(self, mount_point):
        r = subprocess.run(["mountpoint", "-q", mount_point])
        assert r.returncode == 0, "Drive not mounted — init order issue?"


# ========================================================================
# 14. test_sd_card_info  (TC-026)
# ========================================================================
class TestSdCardInfo:
    """Report block device size via lsblk."""

    def test_sd_card_info(self, block_dev):
        r = _run(f"lsblk -o NAME,SIZE,TYPE,FSTYPE {block_dev}")
        assert r.returncode == 0
        print(f"SD card info:\n{r.stdout}")


# ========================================================================
# 15. test_manual_scenarios  (TC-014, TC-024, TC-025, TC-027)
# ========================================================================
class TestManualScenarios:
    """All manual scenarios — skip with instructions."""

    def test_manual_scenarios(self):
        print("=== MANUAL TEST SCENARIOS ===")
        print("[TC-014] Boot without SD card")
        print("[TC-024] SD card hot-removal while idle")
        print("[TC-025] Windows host compatibility")
        print("[TC-027] Unformatted / corrupted SD card")
        pytest.skip("Manual scenarios require physical intervention")
