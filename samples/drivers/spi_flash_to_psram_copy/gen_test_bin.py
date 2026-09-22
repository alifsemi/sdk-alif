#!/usr/bin/env python3
"""Generate a flash test binary for the spi_flash sample.

Layout:
- 8-byte magic: b"ALIFPSRM"
- 32-bit little-endian words where each word equals its byte offset
- 32-bit little-endian CRC32 trailer over all prior bytes
"""

from __future__ import annotations

import argparse
import pathlib
import re
import struct
import zlib

MAGIC = b"ALIFPSRM"
MIN_SIZE = len(MAGIC) + 4  # Magic + CRC32 trailer


_SIZE_RE = re.compile(r"^\s*(\d+)\s*([kKmMgG]?[iI]?[bB]?)?\s*$")


def parse_size(size_text: str) -> int:
    """Parse sizes like: 4194304, 4096, 4M, 4MB, 4MiB, 128k."""
    match = _SIZE_RE.match(size_text)
    if not match:
        raise ValueError(f"Invalid size: {size_text!r}")

    number = int(match.group(1))
    suffix = (match.group(2) or "").lower()

    if suffix in ("", "b"):
        multiplier = 1
    elif suffix in ("k", "kb", "kib"):
        multiplier = 1024
    elif suffix in ("m", "mb", "mib"):
        multiplier = 1024 * 1024
    elif suffix in ("g", "gb", "gib"):
        multiplier = 1024 * 1024 * 1024
    else:
        raise ValueError(f"Unsupported size suffix: {suffix!r}")

    size = number * multiplier
    if size < MIN_SIZE:
        raise ValueError(f"Size must be at least {MIN_SIZE} bytes")
    if size % 4 != 0:
        raise ValueError("Size must be a multiple of 4 bytes")

    return size


def build_payload(size: int) -> bytes:
    payload = bytearray(size)

    payload[: len(MAGIC)] = MAGIC

    for off in range(len(MAGIC), size - 4, 4):
        struct.pack_into("<I", payload, off, off)

    crc = zlib.crc32(payload[:-4]) & 0xFFFFFFFF
    struct.pack_into("<I", payload, size - 4, crc)

    return bytes(payload)


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate spi_flash sample test binary")
    parser.add_argument(
        "--size",
        required=True,
        help="Output size in bytes or with suffix (e.g. 4194304, 4M, 4MiB)",
    )
    parser.add_argument(
        "--output",
        default="test.bin",
        help="Output file path (default: test.bin)",
    )
    args = parser.parse_args()

    try:
        size = parse_size(args.size)
    except ValueError as err:
        parser.error(str(err))

    out_path = pathlib.Path(args.output)
    out_path.parent.mkdir(parents=True, exist_ok=True)

    payload = build_payload(size)
    out_path.write_bytes(payload)

    crc = struct.unpack_from("<I", payload, len(payload) - 4)[0]
    print(f"Wrote {len(payload)} bytes to {out_path}")
    print(f"Magic: {MAGIC.decode('ascii')}")
    print(f"CRC32 trailer: 0x{crc:08x}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
