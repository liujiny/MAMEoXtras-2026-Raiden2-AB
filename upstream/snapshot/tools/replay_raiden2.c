/* Headless, deterministic Raiden II regression replay; no ROMs included.
 * Build: cc -O2 -Wall tools/replay_raiden2.c -ldl -o /tmp/replay-raiden2
 * Run: /tmp/replay-raiden2 core.so raiden2.zip NEW_OUTPUT_DIR 9000 60 1
 * R2_CHECK_STATE=1 also compares 30 rendered frames across save/load.
 * R2_MENU=1 toggles the internal MAME menu after frame 2800.
 * R2_LOAD_STATE=path loads a raw MAMESAVE after 60 warmup frames.
 * Use a fresh output directory each time (NVRAM/hiscores affect replays).
 */
#include "../src/libretro-common/include/libretro.h"
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

static struct { char *key; char *value; } opts[512];
static unsigned nopts, frame, pixel_format, capture_step = 60;
static const char *outdir;
static FILE *trace;
static void (*probe)(FILE *);
static int play;
static uint32_t video_hash;
static unsigned black_sum[513][513];
static int find_blocks;
static unsigned pixel(const void *data, unsigned x, unsigned y, size_t pitch)
{
   if (pixel_format == RETRO_PIXEL_FORMAT_XRGB8888)
      return ((const uint32_t *)((const char *)data + y*pitch))[x] & 0xffffff;
   return ((const uint16_t *)((const char *)data + y*pitch))[x] &
      (pixel_format == RETRO_PIXEL_FORMAT_RGB565 ? 0xffff : 0x7fff);
}
static unsigned black_rect(unsigned x, unsigned y, unsigned w, unsigned h)
{
   return black_sum[y+h][x+w] - black_sum[y][x+w] - black_sum[y+h][x] + black_sum[y][x];
}
static void logger(enum retro_log_level level, const char *fmt, ...)
{
   va_list args;
   if (level < RETRO_LOG_INFO && !getenv("R2_DEBUG_LOG")) return;
   va_start(args, fmt); vfprintf(stderr, fmt, args); va_end(args);
}
static bool environment(unsigned cmd, void *data)
{
   unsigned i;
   switch (cmd) {
   case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
      ((struct retro_log_callback *)data)->log = logger; return true;
   case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
   case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
      *(const char **)data = outdir; return true;
   case RETRO_ENVIRONMENT_GET_CAN_DUPE:
      *(bool *)data = true; return true;
   case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
      pixel_format = *(unsigned *)data; return pixel_format <= 2;
   case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
      *(unsigned *)data = 1; return true;
   case RETRO_ENVIRONMENT_SET_CORE_OPTIONS: {
      struct retro_core_option_definition *p = data;
      for (i = 0; i < nopts; i++) { free(opts[i].key); free(opts[i].value); }
      nopts = 0;
      for (; p->key && nopts < 512; p++) {
         opts[nopts].key = strdup(p->key);
         opts[nopts++].value = strdup(p->default_value ? p->default_value : p->values[0].value);
      }
      return true;
   }
   case RETRO_ENVIRONMENT_GET_VARIABLE: {
      struct retro_variable *v = data;
      if (strstr(v->key, "skip_disclaimer") || strstr(v->key, "skip_warnings")) { v->value = "enabled"; return true; }
      if (strstr(v->key, "frameskip")) { v->value = "0"; return true; }
      for (i = 0; i < nopts; i++) if (!strcmp(opts[i].key, v->key)) { v->value = opts[i].value; return true; }
      v->value = NULL; return false;
   }
   case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
      *(bool *)data = false; return true;
   case RETRO_ENVIRONMENT_SET_MESSAGE:
      fprintf(stderr, "message: %s\n", ((struct retro_message *)data)->msg); return true;
   case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
   case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
   case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
   case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
   case RETRO_ENVIRONMENT_SET_GEOMETRY:
      return true;
   default: return false;
   }
}
static void video(const void *data, unsigned w, unsigned h, size_t pitch)
{
   unsigned x, y;
   int block = 0;
   if (data) {
      video_hash = 2166136261U;
      for (y=0; y<h; y++) for(x=0;x<w*(pixel_format==1?4:2);x++)
         video_hash=(video_hash ^ ((const unsigned char *)data)[y*pitch+x])*16777619U;
   }
   if (probe && trace) { fprintf(trace, "FRAME %u\n", frame); probe(trace); }
   if (find_blocks && data && w<=512 && h<=512 && frame>700) {
      for(y=0;y<h;y++) for(x=0;x<w;x++)
         black_sum[y+1][x+1]=black_sum[y][x+1]+black_sum[y+1][x]-black_sum[y][x]+(pixel(data,x,y,pitch)==0);
      for(y=24;y+17<h;y++) for(x=0;x+16<=w;x++) {
         unsigned border;
         if (black_rect(x,y,16,16)!=256) continue;
         border=black_rect(x,y-1,16,1)+black_rect(x,y+16,16,1);
         if(x)border+=black_rect(x-1,y,1,16);
         if(x+16<w)border+=black_rect(x+16,y,1,16);
         if(border>16)continue;
         fprintf(stderr,"BLOCK frame=%u xy=%u,%u border=%u\n",frame,x,y,border);
         block=1;
      }
   }
   if (data && (frame % capture_step == 0 || block)) {
      char path[1024]; FILE *f;
      snprintf(path, sizeof(path), "%s/frame-%05u.ppm", outdir, frame);
      f = fopen(path, "wb"); if (!f) exit(2);
      fprintf(f, "P6\n%u %u\n255\n", w, h);
      for (y = 0; y < h; y++) for (x = 0; x < w; x++) {
         unsigned r, g, b, v; unsigned char rgb[3];
         if (pixel_format == RETRO_PIXEL_FORMAT_XRGB8888) {
            v = ((const uint32_t *)((const char *)data + y * pitch))[x]; r = v >> 16; g = v >> 8; b = v;
         } else {
            v = ((const uint16_t *)((const char *)data + y * pitch))[x];
            r = (v >> (pixel_format == RETRO_PIXEL_FORMAT_RGB565 ? 11 : 10)) & 31;
            g = (v >> 5) & (pixel_format == RETRO_PIXEL_FORMAT_RGB565 ? 63 : 31);
            b = v & 31; r = r * 255 / 31; g = g * 255 / (pixel_format == RETRO_PIXEL_FORMAT_RGB565 ? 63 : 31); b = b * 255 / 31;
         }
         rgb[0] = r; rgb[1] = g; rgb[2] = b; fwrite(rgb, 1, 3, f);
      }
      fclose(f);
   }
}
static void audio(int16_t l, int16_t r) { (void)l; (void)r; }
static size_t audio_batch(const int16_t *d, size_t n) { (void)d; return n; }
static void poll(void) {}
static int16_t input(unsigned p, unsigned dev, unsigned idx, unsigned id)
{
	if (dev == RETRO_DEVICE_KEYBOARD && id == RETROK_TAB && getenv("R2_MENU"))
		return frame >= 2800 && frame % 31 == 0;
   if (!play || p || dev != RETRO_DEVICE_JOYPAD) return 0;
   if (id == RETRO_DEVICE_ID_JOYPAD_SELECT) return frame >= 200 && frame < 205;
   if (id == RETRO_DEVICE_ID_JOYPAD_START) return frame >= 230 && frame < 235;
   if (id == RETRO_DEVICE_ID_JOYPAD_B) return frame >= 235;
   if (getenv("R2_MOVE")) {
      if (id == RETRO_DEVICE_ID_JOYPAD_LEFT) return frame>=750 && frame<780;
      if (id == RETRO_DEVICE_ID_JOYPAD_RIGHT) return frame>=1650 && frame<1710;
   }
   (void)idx; return 0;
}
#define LOAD(name) __typeof__(&name) p_##name = dlsym(lib, #name); if (!p_##name) { fprintf(stderr,"missing %s\n",#name); exit(1); }
int main(int argc, char **argv)
{
   void *lib; unsigned frames; struct retro_game_info game = {0}; char path[1024];
   if (argc < 5) { fprintf(stderr,"usage: replay core.so raiden2.zip output_dir frames [capture_step] [play]\n"); return 1; }
   outdir=argv[3]; frames=atoi(argv[4]); if(argc>5)capture_step=atoi(argv[5]); if(argc>6)play=atoi(argv[6]);
   find_blocks = getenv("R2_FIND_BLOCKS") != NULL;
   if (!frames || !capture_step) { fprintf(stderr,"frames and capture_step must be positive\n"); return 1; }
   if (mkdir(outdir,0755)) { perror("create fresh output directory"); return 1; }
   lib=dlopen(argv[1],RTLD_NOW); if(!lib){fprintf(stderr,"%s\n",dlerror());return 1;}
   LOAD(retro_set_environment); LOAD(retro_set_video_refresh); LOAD(retro_set_audio_sample); LOAD(retro_set_audio_sample_batch);
   LOAD(retro_set_input_poll); LOAD(retro_set_input_state); LOAD(retro_init); LOAD(retro_load_game); LOAD(retro_run); LOAD(retro_unload_game); LOAD(retro_deinit);
   LOAD(retro_serialize_size); LOAD(retro_serialize); LOAD(retro_unserialize);
   probe=dlsym(lib,"retro_r2_probe");
   if(probe){snprintf(path,sizeof(path),"%s/trace.txt",outdir);trace=fopen(path,"w");}
   p_retro_set_environment(environment); p_retro_set_video_refresh(video); p_retro_set_audio_sample(audio); p_retro_set_audio_sample_batch(audio_batch);
   p_retro_set_input_poll(poll); p_retro_set_input_state(input); p_retro_init(); game.path=argv[2];
   if(!p_retro_load_game(&game)){fprintf(stderr,"load failed\n");return 1;}
   if (getenv("R2_LOAD_STATE")) {
      FILE *f = fopen(getenv("R2_LOAD_STATE"), "rb");
      long size; void *state;
      if (!f || fseek(f,0,SEEK_END) || (size=ftell(f))<24 || fseek(f,0,SEEK_SET)) {
         fprintf(stderr,"cannot read state\n"); return 3;
      }
      state=malloc(size);
      if(!state || fread(state,1,size,f)!=(size_t)size) { fprintf(stderr,"state read failed\n");return 3; }
      fclose(f);
      for(frame=0;frame<60;frame++)p_retro_run();
      if ((size_t)size!=p_retro_serialize_size() || !p_retro_unserialize(state,size)) {
         fprintf(stderr,"state load failed (file %ld, core %zu)\n",size,p_retro_serialize_size());return 3;
      }
      fprintf(stderr,"loaded state: %s (%ld bytes)\n",getenv("R2_LOAD_STATE"),size);
      free(state);
      {
         void (*render)(unsigned) = dlsym(lib,"retro_r2_render");
         const unsigned masks[]={0,0x1e,0x1d,0x1b,0x17,0x0f};
         unsigned k;
         if(render)for(k=0;k<6;k++){frame=60000+k;render(masks[k]);}
      }
   }
   for(frame=0;frame<frames;frame++) {
      p_retro_run();
      if (frame == 3000 && getenv("R2_CHECK_STATE")) {
         size_t size=p_retro_serialize_size();
         void *state=malloc(size); unsigned k; uint32_t hashes[30];
         if(!state || !p_retro_serialize(state,size)) { fprintf(stderr,"serialize failed\n"); return 3; }
         for(k=0;k<30;k++){p_retro_run();hashes[k]=video_hash;}
         if(!p_retro_unserialize(state,size)){fprintf(stderr,"unserialize failed\n");return 3;}
         for(k=0;k<30;k++){
            p_retro_run();
            if(video_hash!=hashes[k]) {fprintf(stderr,"state video mismatch at %u: %08x/%08x\n",k,video_hash,hashes[k]);return 3;}
         }
         fprintf(stderr,"PASS: 30 frame hashes after save/load match; state size %zu\n",size);
         free(state);
      }
   }
   p_retro_unload_game(); p_retro_deinit(); if(trace)fclose(trace); dlclose(lib); return 0;
}
