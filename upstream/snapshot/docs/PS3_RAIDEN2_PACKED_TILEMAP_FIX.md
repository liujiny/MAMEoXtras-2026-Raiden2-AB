# Raiden II PS3 black-block fix v3 (2026-09-22)

## Confirmed cause

The PS3 build enables packed 4-bit graphics in `src/drawgfx.c` to reduce
decoded graphics memory. Each byte contains two pixel pens. The generic
`HandleTransparencyPen` in `src/tilemap.c` updated the tile-wide
`bWhollyOpaque`/`bWhollyTransparent` flags for the low-nibble pixel only.
It wrote the high-nibble pixel's individual mask, but did not include that
pixel in the tile-wide classification.

A mixed tile can therefore be classified as uniform. The cached tilemap
renderer then bypasses its per-pixel mask and draws or skips an entire
rectangle. This produces a black hole in the bottom layer, or a solid
palette-coloured rectangle over a lower layer. Rotation/clipping and the
particular tile pattern affect its appearance and position.

This is a software tilemap bug exposed by the PS3 packed-memory option,
not PS3 GPU failure, a corrupt ROM, or evidence of another TOC misaddress.
Ordinary Unix builds disabled packing, which explains why the earlier
unpacked replays did not reproduce the user's square.

## Reproduction and evidence

The user supplied `raiden2.zip`, `photo_2026-09-22_20-18-14.jpg` and
`raiden2.state`. The latter is RZIP-compressed, with a RASTATE wrapper and
a 343792-byte big-endian MAMESAVE payload, signature `2faff315`.
MAME's normal typed save-state loader handles the endian conversion.

Using the same save state and same source:

1. Unpacked graphics render the photographed field/aircraft scene normally.
2. Packed graphics with the old tilemap handler reproduce the black square
   immediately, including a render before executing another emulated CPU frame.
3. Separating layers locates the error in the background layer. In the
   240x320 output, differing pixels are bounded by x=50..65, y=237..252.
   There are 222 differing pixels in the composite; the plane overlaps it.
4. Updating the second pixel's classification removes the square. The full
   composite and all five layer captures match the unpacked reference byte
   for byte, not just visually.
5. From the supplied state, 600 consecutive frames using the final packed
   core also match 600 unpacked-reference frames exactly.

The fix keeps packed graphics enabled, with no extra framebuffer, per-tile
storage or new per-frame allocation. It applies to both raw-palette and
indexed-palette variants of the shared handler. There are no intro-animation,
clock, speed-hack, layer-hiding or blanket palette changes. Sprite v1/v2
changes are retained; tilemap/palette DMA emulation is not changed in v3.

## Automated checks

`tools/test_packed_tilemap.py` compiles the actual production handler twice
(raw/indexed palette) and compares packed/unpacked classifications, pixel
values, pixel masks and clipped composites across 12096 cases. Coverage:
8/16/32-pixel tiles, row padding, all flips, rotation, priorities, several
transparent pens, ignored transparency, fully opaque/transparent tiles,
and transparency present exclusively in either nibble. ASan/UBSan pass.
The unmodified handler fails the mixed-high-nibble negative control.

```sh
rtk proxy python3 tools/test_packed_tilemap.py --sanitize
rtk proxy sh -c 'git show 3c1441bc:src/tilemap.c | python3 tools/test_packed_tilemap.py --source -'
```

The second command is expected to fail. The sprite regression suite remains
green. A cold-boot packed-graphics replay runs 9000 `retro_run` calls and its
save/load check reproduces 30 subsequent video hashes exactly. The state size
is unchanged from v2 (343792 bytes).

For host integration, use an isolated source/build directory so PS3 objects
are not overwritten. The test-only `MAME2003_TEST_PACKED_GFX` define enables
the same decode format as PS3 without enabling PS3 SDK dependencies. For
example, after building the Unix core normally:

```sh
rtk proxy make -B platform=unix SPLIT_UP_LINK=0 GIT_VERSION= CC='cc -DMAME2003_TEST_PACKED_GFX' src/drawgfx.o
rtk proxy make -j8 platform=unix SPLIT_UP_LINK=0 GIT_VERSION=
rtk proxy cc -O2 -Wall -Wextra tools/replay_raiden2.c -ldl -o /tmp/replay-raiden2
rtk proxy python3 tools/unpack_retro_state.py /path/raiden2.state /new/path/raiden2.mamesave
rtk proxy env R2_LOAD_STATE=/new/path/raiden2.mamesave /tmp/replay-raiden2 /path/core.so /path/raiden2.zip /fresh/output-directory 600 1 0
```

The extractor never overwrites its output or alters the input. ROMs, save
states, decoded ROM graphics and the user's photographs are not included in
the distributable package. PS3 hardware confirmation remains for the user.

## Deployment

The test package is `ps3_mame2003plus_raiden2_tilemapfix_v3_20260922`.
It links the updated MAME archive into the unchanged RetroArch 1.10.3
frontend objects (`f592027092ca776099b282004766968175ce8d3c`), using Cell SDK
4.75 and the existing NPDRM SELF packaging method.

Back up and replace only:

`/dev_hdd0/game/SSNE10001/USRDIR/cores/mame2003_plus_libretro_psl1ght.self`

Keep the filename of the installed MAME core if different; do not overwrite
EBOOT.BIN, the FBNeo SELF, configurations, saves or the old v2 test package.
The supplied v2 state loads in the new code without conversion. Cold-boot
the new core first, then load that state for a direct comparison.
