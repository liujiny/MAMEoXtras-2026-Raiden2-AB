#!/usr/bin/env python3
"""Exercise the real tilemap transparency handler with packed/unpacked pixels.

No ROM/SDK is needed. --source - accepts an older tilemap.c via stdin for a
negative control. Both palette variants are compiled, including the PS3 path.
"""
import argparse
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile


PREAMBLE = r"""
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint32_t pen_t;
#define MAX_TILESIZE 64
#define TILE_FLIPX 1
#define TILE_FLIPY 2
#define TILE_IGNORE_TRANSPARENCY 8
#define TILE_4BPP 16
#define TILE_FLAG_FG_OPAQUE 16
struct mame_bitmap { void *line[64]; };
struct tilemap {
    UINT32 cached_tile_width, cached_tile_height;
    struct mame_bitmap *pixmap, *transparency_bitmap;
    UINT32 *pPenToPixel[4];
    int transparent_pen;
};
static struct {
    int skip;
    const UINT8 *pen_data;
    const pen_t *pal_data;
    UINT32 priority;
} tile_info;
static struct { pen_t *remapped_colortable; } machine;
#define Machine (&machine)
#define CHECK(c) do { if (!(c)) { \
    fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#c); exit(1); } } while (0)
"""

TESTS = r"""
static UINT16 pix[2][64][64];
static UINT8 mask[2][64][64];
static UINT8 unpacked[64*68], packed[64*34];
static UINT32 mapping[4][64*64];
static pen_t palette[4096];
static unsigned tests;
static void run(unsigned size, unsigned skip, unsigned flip, unsigned rotate,
        unsigned transparent, unsigned priority, unsigned ignore,
        unsigned pattern, unsigned raw)
{
    struct tilemap t;
    struct mame_bitmap pmap, tmap;
    unsigned x,y,k,count=0,pitch=size+skip, opaque=(transparent+1)&15;
    UINT8 codes[2];
    memset(&t,0,sizeof(t));
    memset(pix,0xa5,sizeof(pix));
    memset(mask,0x5a,sizeof(mask));
    memset(unpacked,0,sizeof(unpacked));
    memset(packed,0,sizeof(packed));
    t.cached_tile_width=t.cached_tile_height=size;
    t.transparent_pen=transparent;
    t.pixmap=&pmap; t.transparency_bitmap=&tmap;
    for(k=0;k<4;k++)t.pPenToPixel[k]=mapping[k];
    for(y=0;y<size;y++)for(x=0;x<size;x++) {
        unsigned v=opaque, dx=x,dy=y;
        switch(pattern) {
        case 0: break; /* all opaque */
        case 1: v=transparent; break;
        case 2: v=(x&1)?transparent:opaque; break;
        case 3: v=(x&1)?opaque:transparent; break;
        case 4: if(x==size-1 && y==size-1)v=transparent; break;
        case 5: v=(x==size-1 && y==size-1)?opaque:transparent; break;
        default: v=(x*7+y*3+(x*y)%11)&15; break;
        }
        if(ignore || v!=transparent)count++;
        unpacked[y*pitch+x]=v;
        packed[(y*pitch+x)/2]|=v<<((x&1)*4);
        if(rotate) { dx=y; dy=size-1-x; }
        if(flip&TILE_FLIPX)dx=size-1-dx;
        if(flip&TILE_FLIPY)dy=size-1-dy;
        mapping[flip][y*size+x]=dy*MAX_TILESIZE+dx;
    }
    tile_info.skip=skip;
    tile_info.priority=priority;
    tile_info.pal_data=palette+0x460;
    for(k=0;k<2;k++) {
        unsigned flags=flip|(ignore?TILE_IGNORE_TRANSPARENCY:0)|(k?TILE_4BPP:0);
        tile_info.pen_data=k?packed:unpacked;
        for(y=0;y<64;y++) { pmap.line[y]=pix[k][y]; tmap.line[y]=mask[k][y]; }
        codes[k]=raw?HandleTransparencyPen_raw(&t,0,0,flags):HandleTransparencyPen_ind(&t,0,0,flags);
    }
    if(codes[0]!=codes[1]) {
        fprintf(stderr,"classification mismatch: size=%u skip=%u flip=%u rot=%u pen=%u priority=%u ignore=%u pattern=%u raw=%u unpacked=%u packed=%u\n",
            size,skip,flip,rotate,transparent,priority,ignore,pattern,raw,codes[0],codes[1]);
    }
    CHECK(codes[0]==codes[1]);
    CHECK(codes[1]==((count && count!=size*size)?TILE_FLAG_FG_OPAQUE:0));
    CHECK(!memcmp(pix[0],pix[1],sizeof(pix[0])));
    CHECK(!memcmp(mask[0],mask[1],sizeof(mask[0])));
    /* Verify the cached uniform-tile fast path yields the same composite
     * as testing every pixel, even when the first visible pixel changes. */
    for(k=0;k<size;k++)for(y=0;y<size;y++)for(x=k;x<size;x++) {
        unsigned actual=(codes[1]&TILE_FLAG_FG_OPAQUE)?mask[1][y][x]:mask[1][y][k];
        UINT16 a=(actual&TILE_FLAG_FG_OPAQUE)?pix[1][y][x]:0xbeef;
        UINT16 b=(mask[0][y][x]&TILE_FLAG_FG_OPAQUE)?pix[0][y][x]:0xbeef;
        CHECK(a==b);
    }
    tests++;
}
int main(void)
{
    unsigned i,s,skip,flip,rot,pen,p,ign,pattern,raw;
    const unsigned sizes[]={8,16,32}, pens[]={0,7,15}, priorities[]={0,5,15};
    machine.remapped_colortable=palette;
    for(i=0;i<4096;i++)palette[i]=(i*13+17)&65535;
    for(s=0;s<3;s++)for(skip=0;skip<=4;skip+=4)
    for(flip=0;flip<4;flip++)for(rot=0;rot<2;rot++)
    for(pen=0;pen<3;pen++)for(p=0;p<3;p++)for(ign=0;ign<2;ign++)
    for(pattern=0;pattern<7;pattern++)for(raw=0;raw<2;raw++)
        run(sizes[s],skip,flip,rot,pens[pen],priorities[p],ign,pattern,raw);
    printf("PASS: %u packed/unpacked cases; classifications, pixels, masks and clipped composites match\n",tests);
    return 0;
}
"""


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", default=str(Path(__file__).resolve().parents[1] / "src/tilemap.c"))
    parser.add_argument("--cc", default="cc")
    parser.add_argument("--sanitize", action="store_true")
    args = parser.parse_args()
    source = sys.stdin.read() if args.source == "-" else Path(args.source).read_text()
    start = source.index("static UINT8 TRANSP(HandleTransparencyPen)(")
    end = source.index("static UINT8 TRANSP(HandleTransparencyPenBit)(", start)
    handler = source[start:end]
    variants = []
    for name, init, get in (
        ("ind", "const pen_t *pPalData = tile_info.pal_data", "pPalData[pen]"),
        ("raw", "int palBase = tile_info.pal_data - Machine->remapped_colortable", "(palBase + (pen))"),
    ):
        variants.append(f"#define TRANSP(f) f ## _{name}\n#define PAL_INIT {init}\n#define PAL_GET(pen) {get}\n"
                        + handler + "\n#undef TRANSP\n#undef PAL_INIT\n#undef PAL_GET\n")
    with tempfile.TemporaryDirectory(prefix="packed-tilemap-") as tmp:
        test = Path(tmp) / "test.c"
        exe = Path(tmp) / "test"
        test.write_text(PREAMBLE + "\n".join(variants) + TESTS)
        cmd = shlex.split(args.cc) + ["-O1", "-g", "-Wall", "-Wextra", "-Werror"]
        if args.sanitize:
            cmd += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        subprocess.run(cmd + [str(test), "-o", str(exe)], check=True)
        subprocess.run([str(exe)], check=True)


if __name__ == "__main__":
    main()
