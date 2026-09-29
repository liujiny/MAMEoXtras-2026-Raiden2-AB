"""Check XBE headers, section bounds/digests, entry point and kernel imports.

Format reference: https://xboxdevwiki.net/Xbe
The optional retail patch changes header encoding and media/region settings only.
This verifies the file structure, not runtime behavior on an Xbox.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import struct

KEYS = {
    "retail": (0xA8FC57AB, 0x5B6D40B6),
    "debug": (0x94859D4B, 0xEFB1F152),
}


def inspect(path):
    data = path.read_bytes()
    if len(data) < 0x184 or data[:4] != b"XBEH":
        raise ValueError(f"Invalid XBE header: {path}")

    def u32(offset):
        if not 0 <= offset <= len(data) - 4:
            raise ValueError(f"Out-of-file field at {offset:#x}")
        return struct.unpack_from("<I", data, offset)[0]

    base = u32(0x104)
    header_size = u32(0x108)
    image_size = u32(0x10c)
    count = u32(0x11c)
    table = u32(0x120) - base
    certificate = u32(0x118) - base
    if not (0 < count < 4096 and 0 <= table and table + count * 56 <= header_size <= len(data)):
        raise ValueError("Invalid section table")
    if not (0 <= certificate and certificate + u32(certificate) <= header_size):
        raise ValueError("Invalid certificate range")
    sections = []
    for index in range(count):
        offset = table + index * 56
        flags, va, size, raw, raw_size, name_va = struct.unpack_from("<6I", data, offset)
        name_offset = name_va - base
        if not 0 <= name_offset < header_size:
            raise ValueError(f"Invalid section name pointer: {index}")
        end = data.find(b"\0", name_offset, header_size)
        if end < 0:
            raise ValueError("Unterminated section name")
        name = data[name_offset:end].decode("ascii")
        if not (base <= va and va + size <= base + image_size and raw + raw_size <= len(data)):
            raise ValueError(f"Section outside image or file: {name}")
        expected = data[offset + 36:offset + 56]
        actual = hashlib.sha1(struct.pack("<I", raw_size) + data[raw:raw+raw_size]).digest()
        if actual != expected:
            raise ValueError(f"Section SHA-1 mismatch: {name}")
        sections.append(dict(name=name, flags=flags, address=va, size=size,
                             file_offset=raw, file_size=raw_size, preload=bool(flags & 2)))

    def offset_for_va(address, length=4):
        if base <= address and address + length <= base + header_size:
            return address - base
        for section in sections:
            relative = address - section["address"]
            if 0 <= relative and relative + length <= section["file_size"]:
                return section["file_offset"] + relative
        raise ValueError(f"Unmapped address: {address:#x}")

    encoding = None
    for kind, (entry_key, thunk_key) in KEYS.items():
        entry = u32(0x128) ^ entry_key
        thunk = u32(0x158) ^ thunk_key
        if any(s["address"] <= entry < s["address"] + s["size"] and s["flags"] & 4 for s in sections):
            offset_for_va(thunk)
            encoding = kind
            break
    if not encoding:
        raise ValueError("No valid executable entry point")
    imports = []
    for index in range(1024):
        value = u32(offset_for_va(thunk + index * 4))
        if not value:
            break
        if not value & 0x80000000 or not 0 < (value & 0x7fffffff) < 512:
            raise ValueError(f"Invalid kernel import {value:#x}")
        imports.append(value & 0x7fffffff)
    else:
        raise ValueError("Unterminated kernel import table")
    return {
        "file": str(path.resolve()), "bytes": len(data),
        "sha256": hashlib.sha256(data).hexdigest(), "encoding": encoding,
        "title": data[certificate+12:certificate+92].decode("utf-16le").split("\0", 1)[0],
        "title_id": f"0x{u32(certificate+8):08X}", "entry_point": f"0x{entry:08X}",
        "kernel_thunk": f"0x{thunk:08X}", "kernel_import_count": len(imports),
        "stack_bytes": u32(0x130), "initialization_flags": f"0x{u32(0x124):08X}",
        "allowed_media": f"0x{u32(certificate+0x9c):08X}",
        "game_region": f"0x{u32(certificate+0xa0):08X}",
        "section_count": count, "section_hashes_valid": True,
        "preloaded_section_bytes": sum(s["size"] for s in sections if s["preload"]),
        "sections": sections,
    }


def patch_retail(path):
    before = inspect(path)
    data = bytearray(path.read_bytes())
    base = struct.unpack_from("<I", data, 0x104)[0]
    certificate = struct.unpack_from("<I", data, 0x118)[0] - base
    if before["encoding"] == "debug":
        for offset, debug, retail in [(0x128, KEYS["debug"][0], KEYS["retail"][0]),
                                      (0x158, KEYS["debug"][1], KEYS["retail"][1])]:
            value = struct.unpack_from("<I", data, offset)[0]
            struct.pack_into("<I", data, offset, value ^ debug ^ retail)
    struct.pack_into("<I", data, certificate + 0x9c, 0x800000FF)
    struct.pack_into("<I", data, certificate + 0xa0, 0x00000007)
    path.write_bytes(data)
    return inspect(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("files", nargs="+", type=Path)
    parser.add_argument("--retail", action="store_true", help="Patch debug encoding for homebrew retail consoles")
    parser.add_argument("--json", type=Path, help="Save full section metadata to this JSON file")
    args = parser.parse_args()
    results = [(patch_retail(path) if args.retail else inspect(path)) for path in args.files]
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(results, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    for result in results:
        print(json.dumps({key: value for key, value in result.items() if key != "sections"},
                         ensure_ascii=True, indent=2))


if __name__ == "__main__":
    main()
