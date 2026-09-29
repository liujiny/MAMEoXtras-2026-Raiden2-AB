"""Refresh the isolated RXDK source mirror from the checked-out branch.

The rxdk-build branch already contains all source compatibility changes.  This
script only copies that source into build/rxdk/source so build outputs and
generated resource files never dirty the checked-out source tree.
"""
from pathlib import Path
import hashlib
import json
import shutil
import sys

W = Path(__file__).resolve().parent
ROOT = W.parent.parent
SOURCE = ROOT / "MAMEoXtras 2026 Src"
MIRROR = W / "source"


def main() -> int:
    sys.stdout.reconfigure(encoding="utf8", errors="replace")
    required = [
        SOURCE / "MAMEoX" / "Includes" / "RxdkVmmSeh.h",
        SOURCE / "MAMEoX" / "Sources" / "RxdkDsoundMath.c",
        SOURCE / "MAME" / "src" / "drivers" / "raiden2.c",
        SOURCE / "VCPPMame.h",
    ]
    missing = [p for p in required if not p.is_file()]
    if missing:
        raise FileNotFoundError("RXDK branch source is incomplete: " + ", ".join(str(p) for p in missing))

    if MIRROR.exists():
        shutil.rmtree(MIRROR)
    shutil.copytree(
        SOURCE,
        MIRROR,
        ignore=shutil.ignore_patterns("*.obj", "*.pdb", "*.ilk", "*.pch", "*.ncb", "*.suo", "Thumbs.db"),
    )

    # The legacy MAME project references this as an include directory even when
    # it contains no generated files. Git cannot preserve an empty directory.
    (MIRROR / "c_m68000" / "output").mkdir(parents=True, exist_ok=True)

    count = 0
    digest = hashlib.sha256()
    for p in sorted(MIRROR.rglob("*")):
        if p.is_file():
            count += 1
            digest.update(p.relative_to(MIRROR).as_posix().encode("utf8"))
            digest.update(b"\0")
            digest.update(hashlib.sha256(p.read_bytes()).digest())
    record = {"source_files": count, "tree_digest": digest.hexdigest()}
    (W / "SOURCE_MIRROR.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf8")
    print("SOURCE_MIRROR_OK", count, record["tree_digest"])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
