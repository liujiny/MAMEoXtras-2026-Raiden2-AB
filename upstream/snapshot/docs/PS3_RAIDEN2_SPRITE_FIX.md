# Raiden II corner-block regressions (2026-09-22)

Follow-up: the user's 20:18 photograph was reproduced from their PS3 save
state with packed graphics enabled. Its confirmed cause is the generic
packed-tile transparency classifier, not sprite timing. See
[the v3 investigation and fix](PS3_RAIDEN2_PACKED_TILEMAP_FIX.md).
The v1/v2 notes below record the earlier changes and their validation limits.

## v1: zero-code descriptors

The Salvia playability backport drawn from commit
`05d21f1639cc4b546c64c4f3e9ac45738671eb02` walked every sprite descriptor,
including entries whose initial tile code was zero. A zero-code descriptor
can be an empty entry or sprite-RAM control data; its size, palette and
priority fields must not be interpreted as a drawable multi-tile sprite.
Rendering a nontransparent tile from such an entry at native coordinates
`(0, 0)` can cover the displayed lower-left corner after `ROT270`.

The fix rejects zero-code descriptors before multi-tile expansion, matching
[MAME's SEI25x/RISE1x sprite generator](https://github.com/mamedev/mame/blob/master/src/mame/seibu/sei25x_rise1x_spr.cpp).
The MAME device filters `code % gfx_elements == 0`; this driver has 65,536
sprite tiles and reads a 16-bit descriptor code, so `tile_number != 0` is
equivalent. Normal sprites in slot zero or using palette zero are preserved.
The existing ordering, clipping, flips and coordinate wrapping are unchanged.
The v1 zero-code check needs no additional buffer or per-frame allocation.

This is a correction for spurious sprites, not an implementation of the
separate missing half-transparency/blending behaviour in the old backport.

## v2: latch sprite RAM before rendering

The follow-up photograph shows a pale square on the left edge, not just a
black block in the bottom-left corner. A zero-code check alone cannot rule
out partially rewritten nonzero descriptors.

The backport omitted the sprite latch at CPU addresses `0x0068e..0x0068f`
and rendered directly from the CPU/COP's work RAM. This differs from
[MAME's sprite-buffer implementation, commit 69c66e3046773d6d8ccf9b528e35289607e5e5d4](https://github.com/mamedev/mame/commit/69c66e3046773d6d8ccf9b528e35289607e5e5d4).
It permits a new tile code to be displayed with old coordinates while a
descriptor is being rebuilt.

v2 uses the existing MAME 0.78 generic sprite-buffer facility:

- Both byte lanes of the latch register copy the 4 KiB sprite RAM snapshot.
- The register mapping precedes the overlapping COP mapping (first match wins).
- Drawing uses that snapshot, not live RAM; reset clears it.
- The complete 4 KiB list is traversed. COP channel `0x14` is tilemap DMA,
  not a sprite count. Its old estimate is no longer used to limit drawing.
- Generic video saves and restores the snapshot with the rest of the state.
- The v1 zero-code guard remains. No palette, screen region or valid sprite
  is hidden just because it resembles the photographed square.

Additional RAM is 4096 bytes for Raiden II, allocated once. Other games do
not acquire this buffer. No intro-specific, CPU-clock or animation patch is
included. Tilemap/palette DMA and alpha blending are not newly implemented.

This fixes a verified emulation omission, **not a confirmed exact replay of
the photographed fault**. The supplied ROM was used for host replay, but the
precise intermittent square has not been reproduced there. PS3 scene testing
is still required; do not label this a confirmed final color-block fix.

## Verification

`rtk proxy python3 tools/test_raiden2_sprites.py --sanitize` compiles the actual
sprite traversal/drawing functions from `src/drivers/raiden2.c` against a
small raster test bed. No ROMs or copied sprite-walker implementation are used.
The pre-fix driver fails the zero-code test; the fixed driver passes:

- Zero-code descriptors with nonzero size, palette and all four priorities.
- The rotated lower-left pixel and preservation of a real sprite there.
- Valid slot-zero/palette-zero sprites, reverse ordering and multi-tile flips.
- A nonzero multi-tile descriptor that wraps the 16-bit tile code to zero.
- Coordinate wrapping, clipping, sprite-RAM bounds and disabled sprites.
- Host AddressSanitizer and UndefinedBehaviorSanitizer checks.
- Low/high byte latch-register routing and buffer/save-state allocation flag.
- No display before the first latch; partial and complete live-RAM rewrites
  remain invisible until the next latch, which also retires removed sprites.
- The final sprite slot and buffers shorter than a complete descriptor.

Host integration was built in an isolated source copy using `platform=unix`,
so PS3 objects were not overwritten. The user's `raiden2.zip` loaded and ran
for 9000 `retro_run` calls with coin/start/fire input, covering the first stage,
continue/game-over and subsequent attract sequences; frames were sampled
every 60 calls. A separate save/load check at call 3000 reproduced the next
30 video-frame hashes exactly (host state size 343792 bytes). There were no
host load failures or crashes. These are not measurements from PS3 hardware.

`tools/replay_raiden2.c` provides the headless replay harness. Build with:

```sh
rtk proxy cc -O2 -Wall tools/replay_raiden2.c -ldl -o /tmp/replay-raiden2
rtk proxy env R2_CHECK_STATE=1 /tmp/replay-raiden2 /absolute/core.so /absolute/raiden2.zip /fresh/output-directory 9000 60 1
```

Always use a fresh output directory: saved NVRAM/high scores can change game
timing. The harness writes PPM captures and core save data only there. ROMs,
decrypted ROM data and their graphics are not part of the release package.

PS3 validation uses Cell SDK 4.75 and the existing RetroArch 1.10.3 frontend
objects (`f592027092ca776099b282004766968175ce8d3c`). Only the changed driver
object is replaced in the MAME archive before relinking and NPDRM packaging.

For the new SELF, cold-boot the game without loading an old-version save
state: adding the registered snapshot changes the save-state layout. Existing
files are not deleted or converted. The old SELF/package remains available.
