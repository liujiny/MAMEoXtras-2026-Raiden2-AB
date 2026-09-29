"""Generate a host C test of the actual OG Xbox decoder and packed blitters."""

TEST = r'''
static unsigned raster_cases;
static UINT16 dst8[64*64], dst4[64*64];
static pen_t pal[16];
static void compare_blits(const struct GfxElement *g8, const struct GfxElement *g4)
{
    unsigned tile, sample, flip, ls, ts, crop, opaque, raw, width, height;
    unsigned samples[4] = {0,1,17,0};
    samples[3] = g8->total_elements - 1;
    for (sample=0; sample<4; ++sample)
    for (flip=0; flip<4; ++flip)
    for (ls=0; ls<g8->width; ++ls)
    for (ts=0; ts<g8->height; ++ts)
    for (crop=0; crop<2; ++crop)
    for (opaque=0; opaque<2; ++opaque)
    for (raw=0; raw<2; ++raw) {
        const UINT8 *s8, *s4;
        tile = samples[sample];
        s8 = g8->gfxdata + tile*g8->char_modulo;
        s4 = g4->gfxdata + tile*g4->char_modulo;
        width = g8->width-ls; height = g8->height-ts;
        if (crop) { width = (width+1)/2; height = (height+1)/2; }
        memset(dst8,0x57,sizeof(dst8)); memset(dst4,0x57,sizeof(dst4));
#define BLIT_ARGS(g,s,d) (s),(g)->width,(g)->height,(g)->line_modulo,ls,ts,flip&1,(flip>>1)&1,(d)+16*64+16,width,height,64
        if (raw) {
            if (opaque) {
                blockmove_8toN_opaque_raw(BLIT_ARGS(g8,s8,dst8),0x460);
                blockmove_4toN_opaque_raw(BLIT_ARGS(g4,s4,dst4),0x460);
            } else {
                blockmove_8toN_transpen_raw(BLIT_ARGS(g8,s8,dst8),0x460,15);
                blockmove_4toN_transpen_raw(BLIT_ARGS(g4,s4,dst4),0x460,15);
            }
        } else {
            if (opaque) {
                blockmove_8toN_opaque_ind(BLIT_ARGS(g8,s8,dst8),pal);
                blockmove_4toN_opaque_ind(BLIT_ARGS(g4,s4,dst4),pal);
            } else {
                blockmove_8toN_transpen_ind(BLIT_ARGS(g8,s8,dst8),pal,15);
                blockmove_4toN_transpen_ind(BLIT_ARGS(g4,s4,dst4),pal,15);
            }
        }
#undef BLIT_ARGS
        CHECK(!memcmp(dst8,dst4,sizeof(dst8)));
        ++raster_cases;
    }
}

int main(void)
{
    struct GfxLayout *layouts[3] = {&r2play_charlayout, &r2play_tilelayout, &r2play_spritelayout};
    struct GfxLayout *raws[3] = {&r2play_charlayout_raw, &r2play_tilelayout_raw, &r2play_spritelayout_raw};
    unsigned region, i, tile, pixel, decoded_pixels=0, packed_bytes=0;
    for (i=0;i<16;++i) pal[i]=(i*419+93)&65535;
    for (region=0;region<3;++region) {
        struct GfxLayout *layout=layouts[region];
        unsigned stride=layout->charincrement/8, bytes=layout->total*stride;
        UINT8 *allocation=(UINT8 *)malloc(bytes+32), *rom=allocation+16;
        struct GfxElement *reference, *actual;
        CHECK(allocation);
        memset(allocation,0xa3,bytes+32);
        for (i=0;i<bytes;++i) rom[i]=(UINT8)(i*23 + (i>>9)*73 + (i>>4)*7);
        memset(rom,255,stride); memset(rom+stride,0,stride);
        reference=decodegfx(rom,layout); CHECK(reference);
        CHECK(!(reference->flags&GFX_PACKED));
        CHECK(stride == layout->width*layout->height/2 && stride<=128);
        r2_predecode_packed(rom,layout);
        actual=decodegfx(rom,raws[region]); CHECK(actual);
        CHECK(actual->flags&GFX_PACKED);
        CHECK(actual->flags&GFX_DONT_FREE_GFXDATA);
        CHECK(actual->gfxdata==rom);
        CHECK(actual->width==reference->width && actual->height==reference->height);
        CHECK(actual->total_elements==reference->total_elements);
        CHECK(actual->line_modulo*2==reference->line_modulo);
        CHECK(!memcmp(actual->pen_usage,reference->pen_usage,layout->total*sizeof(UINT32)));
        for (tile=0;tile<layout->total;++tile) {
            for (pixel=0;pixel<reference->char_modulo;++pixel) {
                UINT8 packed=actual->gfxdata[tile*stride+pixel/2];
                CHECK(((packed>>((pixel&1)*4))&15)==reference->gfxdata[tile*reference->char_modulo+pixel]);
                ++decoded_pixels;
            }
        }
        for (i=0;i<16;++i) CHECK(allocation[i]==0xa3 && allocation[bytes+16+i]==0xa3);
        compare_blits(reference,actual);
        packed_bytes+=bytes;
        freegfx(reference); freegfx(actual); free(allocation);
    }
    printf("PASS: all %u decoded pixels and pen masks match; packed store %u bytes; raw storage aliases original regions\n",decoded_pixels,packed_bytes);
    printf("PASS: %u actual packed/unpacked blitter cases, including odd skips, clipping, flips, opaque/transpen and raw/indexed palettes\n",raster_cases);
    return 0;
}
'''

def generate(src):
    def read(path):
        return path.read_text(encoding="latin1")
    def part(text, start, end):
        a = text.index(start)
        return text[a:text.index(end, a)]
    gfx = read(src / "drawgfx.c")
    header = read(src / "drawgfx.h")
    driver = read(src / "drivers/raiden2_playable.inc")
    preamble = '''#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef UINT32 pen_t;
#define MAX_GFX_PLANES 8
#define MAX_GFX_SIZE 1024
#define INLINE __inline
#define osd_malloc malloc
#define SHIFT0 0
#define SHIFT1 8
#define SHIFT2 16
#define SHIFT3 24
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"FAIL line %d: %s\\n",__LINE__,#c); exit(1); } } while(0)
'''
    result = preamble + part(header, "struct GfxLayout", "struct GfxDecodeInfo")
    result += part(gfx, "INLINE int readbit(", "struct _alpha_cache")
    result += part(gfx, "static void calc_penusage(", "INLINE void blockmove_NtoN_transpen_noremap8(")
    result += part(driver, "static struct GfxLayout r2play_charlayout =", "static struct GfxDecodeInfo r2play_gfxdecodeinfo[]")
    adjust = part(gfx, "#define ADJUST_8", "DECLARE_SWAP_RAW_PRI(blockmove_8toN_opaque,")
    blits = part(gfx, "DECLARE_SWAP_RAW_PRI(blockmove_8toN_opaque,", "DECLARE_SWAP_RAW_PRI(blockmove_8toN_transblend,")
    for suffix, colorarg, lookup in (("ind", "const pen_t *paldata", "paldata[n]"), ("raw", "unsigned int colorbase", "colorbase+(n)")):
        result += f'''\n#define DATA_TYPE UINT16
#define HMODULO 1
#define VMODULO dstmodulo
#define COMMON_ARGS const UINT8 *srcdata,int srcwidth,int srcheight,int srcmodulo,int leftskip,int topskip,int flipx,int flipy,DATA_TYPE *dstdata,int dstwidth,int dstheight,int dstmodulo
#define COLOR_ARG {colorarg}
#define LOOKUP(n) ({lookup})
#define SETPIXELCOLOR(dest,n) {{dstdata[dest]=(n);}}
#define INCREMENT_DST(n) {{dstdata+=(n);}}
#define DECLARE_SWAP_RAW_PRI(function,args,body) static void function ## _{suffix} args body
'''
        result += adjust + blits
        result += "\n" + "\n".join("#undef " + name for name in ("DATA_TYPE","HMODULO","VMODULO","COMMON_ARGS","COLOR_ARG","LOOKUP","SETPIXELCOLOR","INCREMENT_DST","DECLARE_SWAP_RAW_PRI","ADJUST_8","ADJUST_4")) + "\n"
    return result + TEST
