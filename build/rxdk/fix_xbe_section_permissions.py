"""Restore writable flags on MAMEoX sectionized CPU/driver XBE sections.

The legacy XDK linker propagates writable input sections through the MAMEoX
/merge:Cxxx=xxx /merge:Dxxx=xxx /merge:Bxxx=xxx sectionizer scheme. RXDK's
LLVM/lld path can keep the executable target characteristic while dropping the
writable characteristic, even though the merged region contains mutable data
and BSS. On hardware this faults when CPU/driver state is first written.

Section 531 (nmk16.c) is intentionally excluded because the legacy XDK XBE also
does not mark that special section writable.
"""
from __future__ import annotations

from pathlib import Path
import argparse
import re
import struct

_XBE_MAGIC = b"XBEH"
_WRITABLE = 0x00000001
_DRIVER_READONLY_EXCEPTIONS = {"531"}


def _is_sectionized_runtime_region(name: str) -> bool:
    if re.fullmatch(r"CPU\d+", name):
        return True
    return name.isdigit() and name not in _DRIVER_READONLY_EXCEPTIONS


def restore_sectionizer_writable(path: Path) -> list[dict[str, int | str]]:
    path = Path(path)
    data = bytearray(path.read_bytes())
    if len(data) < 0x184 or data[:4] != _XBE_MAGIC:
        raise ValueError(f"Invalid XBE header: {path}")

    base = struct.unpack_from("<I", data, 0x104)[0]
    header_size = struct.unpack_from("<I", data, 0x108)[0]
    count = struct.unpack_from("<I", data, 0x11C)[0]
    table = struct.unpack_from("<I", data, 0x120)[0] - base
    if not (0 < count < 4096 and 0 <= table and table + count * 56 <= header_size <= len(data)):
        raise ValueError(f"Invalid XBE section table: {path}")

    changed = []
    for index in range(count):
        offset = table + index * 56
        flags, _va, size, _raw, _raw_size, name_va = struct.unpack_from("<6I", data, offset)
        name_offset = name_va - base
        if not 0 <= name_offset < header_size:
            raise ValueError(f"Invalid XBE section name pointer: {index}")
        end = data.find(b"\0", name_offset, header_size)
        if end < 0:
            raise ValueError(f"Unterminated XBE section name: {index}")
        name = data[name_offset:end].decode("ascii")

        if _is_sectionized_runtime_region(name) and not (flags & _WRITABLE):
            new_flags = flags | _WRITABLE
            struct.pack_into("<I", data, offset, new_flags)
            changed.append({
                "name": name,
                "before": flags,
                "after": new_flags,
                "size": size,
            })

    if changed:
        path.write_bytes(data)
    return changed


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("xbe", type=Path)
    args = parser.parse_args()
    changed = restore_sectionizer_writable(args.xbe)
    cpu = sum(1 for item in changed if str(item["name"]).startswith("CPU"))
    drivers = len(changed) - cpu
    print(f"Restored writable flag on {len(changed)} section(s): {cpu} CPU, {drivers} driver")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
