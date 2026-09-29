#!/usr/bin/env python3
"""Extract MAMESAVE from RetroArch RZIP/RASTATE without altering input."""
import argparse
from pathlib import Path
import struct
import zlib


def unpack(data):
    if data.startswith(b"#RZIPv\x01#"):
        chunk_size, total = struct.unpack_from("<IQ", data, 8)
        chunks, pos = [], 20
        while pos < len(data):
            size, = struct.unpack_from("<I", data, pos)
            pos += 4
            if pos + size > len(data):
                raise ValueError("truncated RZIP chunk")
            chunk = zlib.decompress(data[pos:pos + size])
            if len(chunk) > chunk_size:
                raise ValueError("oversized RZIP chunk")
            chunks.append(chunk)
            pos += size
        data = b"".join(chunks)
        if len(data) != total:
            raise ValueError("RZIP size mismatch")
    if data.startswith(b"RASTATE\x01"):
        pos = 8
        while pos + 8 <= len(data):
            tag, size = struct.unpack_from("<4sI", data, pos)
            pos += 8
            if pos + size > len(data):
                raise ValueError("truncated RASTATE block")
            if tag == b"MEM ":
                data = data[pos:pos + size]
                break
            pos += (size + 7) & ~7
        else:
            raise ValueError("missing RASTATE MEM block")
    if not data.startswith(b"MAMESAVE\x01"):
        raise ValueError("not a MAME format-1 state")
    return data


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    state = unpack(args.input.read_bytes())
    with args.output.open("xb") as output:
        output.write(state)
    print(f"Extracted {len(state)} bytes; flags={state[9]:02x}; "
          f"signature={int.from_bytes(state[20:24], 'little'):08x}")
