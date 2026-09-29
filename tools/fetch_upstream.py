"""Download the two requested immutable upstream revisions for review/reproduction."""
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import hashlib
import json
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
COMMITS = ["3c1441bc8567bebd810046a03593c3b421bf08d7", "938c448045d8eeec992d5036cfff687412850da1"]
REPO = "liujiny/mame2003-plus-libretro-ps3"

def get(url, destination):
    destination.parent.mkdir(parents=True, exist_ok=True)
    if not destination.exists():
        request = urllib.request.Request(url, headers={"User-Agent": "MAMEoXtras-Raiden2-port"})
        with urllib.request.urlopen(request, timeout=60) as response:
            destination.write_bytes(response.read())
    return {"path": str(destination.relative_to(ROOT)), "url": url,
            "sha256": hashlib.sha256(destination.read_bytes()).hexdigest()}

if __name__ == "__main__":
    manifest = []
    paths = set()
    for commit in COMMITS:
        entry = get(f"https://api.github.com/repos/{REPO}/commits/{commit}", ROOT / "upstream" / f"{commit}.json")
        manifest.append(entry)
        metadata = json.loads((ROOT / entry["path"]).read_text())
        assert metadata["sha"] == commit
        print(commit, metadata["commit"]["message"].splitlines()[0], flush=True)
        for item in metadata["files"]:
            print(" ", item["filename"], item["status"], item["additions"], item["deletions"], flush=True)
            paths.add(item["filename"])
        manifest.append(get(f"https://github.com/{REPO}/commit/{commit}.patch", ROOT / "upstream" / f"{commit}.patch"))
    def fetch(path):
        return get(f"https://raw.githubusercontent.com/{REPO}/{COMMITS[-1]}/{path}", ROOT / "upstream" / "snapshot" / path)
    with ThreadPoolExecutor(max_workers=6) as pool:
        manifest += list(pool.map(fetch, sorted(paths)))
    (ROOT / "upstream" / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"Saved {len(manifest)} immutable upstream files.")
