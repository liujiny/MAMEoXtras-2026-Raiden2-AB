"""Download and stage the verified RXDK-VS20XX 1.3.2 command-line toolchain.

Everything downloaded by this script stays under build/rxdk and is git-ignored.
No Microsoft XDK files are used.
"""
from __future__ import annotations

import argparse
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import sys
import urllib.request
import zipfile

W = Path(__file__).resolve().parent
VSIX_URL = "https://github.com/Team-Resurgent/RXDK-VS20XX/releases/download/latest/rxdk-vs-1.3.2.vsix"
VSIX_SHA256 = "4d8b6b09041bdadc8beb5dce7e28e9cadb27c1929cf7436d32e734ae0071754d"
VSIX = W / "downloads" / "rxdk-vs-1.3.2.vsix"
VSIX_DIR = W / "vsix-1.3.2"
CLI = VSIX_DIR / "tools" / "Rxdk.Cli.exe"
RXDK_ROOT = W / "toolchain"


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def environment() -> dict[str, str]:
    env = os.environ.copy()
    env["RXDK"] = str(RXDK_ROOT)
    env["RXDK_STAGED_SDK"] = str(RXDK_ROOT / "sdk")
    env["RXDK_STAGED_TOOLS"] = str(RXDK_ROOT / "tools")
    env["RXDK_STAGED_SAMPLES"] = str(RXDK_ROOT / "samples")
    return env


def verify_ready() -> list[Path]:
    required = [
        CLI,
        RXDK_ROOT / "sdk" / "include" / "d3d8.h",
        RXDK_ROOT / "sdk" / "lib" / "libc.lib",
        RXDK_ROOT / "tools" / "imagebld.exe",
        RXDK_ROOT / "llvm" / "xbox-windows-x64" / "bin" / "clang.exe",
        RXDK_ROOT / "llvm" / "xbox-windows-x64" / "bin" / "lld.exe",
        RXDK_ROOT / "samples" / "RxdkSamples" / "Common" / "Include" / "xbfont.h",
        RXDK_ROOT / "samples" / "RxdkSamples" / "Common" / "Src" / "xbfont.cpp",
    ]
    return [p for p in required if not p.is_file()]


def main() -> int:
    sys.stdout.reconfigure(encoding="utf8", errors="replace")
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--check", action="store_true", help="Only verify that prerequisites are staged")
    ap.add_argument("--force-download", action="store_true", help="Redownload the 1.3.2 VSIX")
    args = ap.parse_args()

    if args.check:
        missing = verify_ready()
        if missing:
            print("RXDK_SETUP_INCOMPLETE")
            for p in missing:
                print("MISSING", p)
            return 1
        print("RXDK_SETUP_OK", RXDK_ROOT)
        return 0

    if os.name != "nt":
        raise RuntimeError("This build flow is currently validated on Windows x64 only")

    VSIX.parent.mkdir(parents=True, exist_ok=True)
    if args.force_download and VSIX.exists():
        VSIX.unlink()
    if not VSIX.exists():
        print("DOWNLOAD", VSIX_URL, flush=True)
        with urllib.request.urlopen(VSIX_URL, timeout=120) as response, VSIX.open("wb") as out:
            shutil.copyfileobj(response, out)
    actual = sha256(VSIX)
    if actual != VSIX_SHA256:
        raise RuntimeError(f"RXDK VSIX SHA-256 mismatch: {actual}")
    print("VSIX_SHA256_OK", actual, flush=True)

    if not CLI.is_file():
        if VSIX_DIR.exists():
            shutil.rmtree(VSIX_DIR)
        VSIX_DIR.mkdir(parents=True)
        with zipfile.ZipFile(VSIX) as z:
            z.extractall(VSIX_DIR)
    if not CLI.is_file():
        raise FileNotFoundError(CLI)

    RXDK_ROOT.mkdir(parents=True, exist_ok=True)
    env = environment()
    for command in ("install-sdk", "install-tools", "install-samples", "install-llvm"):
        print("RXDK", command, flush=True)
        proc = subprocess.run([str(CLI), command], cwd=W.parent.parent, env=env)
        if proc.returncode:
            raise RuntimeError(f"{command} failed with exit code {proc.returncode}")

    missing = verify_ready()
    if missing:
        raise RuntimeError("RXDK setup incomplete: " + ", ".join(str(p) for p in missing))
    print("RXDK_SETUP_OK", RXDK_ROOT)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
