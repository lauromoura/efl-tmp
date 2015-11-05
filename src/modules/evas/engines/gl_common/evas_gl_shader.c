#include "config.h"
#include "shader/evas_gl_shaders.x"
#include "evas_gl_common.h"

#define SHADER_FLAG_SAM_BITSHIFT 3
#define SHADER_FLAG_MASKSAM_BITSHIFT 6
#define SHADER_PROG_NAME_FMT "prog_%08x"
#define SHADER_BINARY_EET_COMPRESS 1

#ifdef WORDS_BIGENDIAN
# define BASEFLAG SHADER_FLAG_BIGENDIAN;
#else
# define BASEFLAG 0
#endif

typedef enum {
   SHADER_FLAG_TEX               = (1 << 0),
   SHADER_FLAG_BGRA              = (1 << 1),
   SHADER_FLAG_MASK              = (1 << 2),
   SHADER_FLAG_SAM12             = (1 << (SHADER_FLAG_SAM_BITSHIFT + 0)),
   SHADER_FLAG_SAM21             = (1 << (SHADER_FLAG_SAM_BITSHIFT + 1)),
   SHADER_FLAG_SAM22             = (1 << (SHADER_FLAG_SAM_BITSHIFT + 2)),
   SHADER_FLAG_MASKSAM12         = (1 << (SHADER_FLAG_MASKSAM_BITSHIFT + 0)),
   SHADER_FLAG_MASKSAM21         = (1 << (SHADER_FLAG_MASKSAM_BITSHIFT + 1)),
   SHADER_FLAG_MASKSAM22         = (1 << (SHADER_FLAG_MASKSAM_BITSHIFT + 2)),
   SHADER_FLAG_IMG               = (1 << 9),
   SHADER_FLAG_BIGENDIAN         = (1 << 10),
   SHADER_FLAG_YUV               = (1 << 11),
   SHADER_FLAG_YUY2              = (1 << 12),
   SHADER_FLAG_NV12              = (1 << 13),
   SHADER_FLAG_YUV_709           = (1 << 14),
   SHADER_FLAG_EXTERNAL          = (1 << 15),
   SHADER_FLAG_AFILL             = (1 << 16),
   SHADER_FLAG_NOMUL             = (1 << 17),
   SHADER_FLAG_ALPHA             = (1 << 18),
   SHADER_FLAG_RGB_A_PAIR        = (1 << 19),
} Shader_Flag;
#define SHADER_FLAG_COUNT 20

static const char *_shader_flags[SHADER_FLAG_COUNT] = {
   "TEX",
   "BGRA",
   "MASK",
   "SAM12",
   "SAM21",
   "SAM22",
   "MASKSAM12",
   "MASKSAM21",
   "MASKSAM22",
   "IMG",
   "BIGENDIAN",
   "YUV",
   "YUY2",
   "NV12",
   "YUV_709",
   "EXTERNAL",
   "AFILL",
   "NOMUL",
   "ALPHA",
   "RGB_A_PAIR"
};

/////////////////////////////////////////////
static void
gl_compile_link_error(GLuint target, const char *action)
{
   int loglen = 0, chars = 0;
   char *logtxt;

   /* Shader info log */
   glGetShaderiv(target, GL_INFO_LOG_LENGTH, &loglen);
   if (loglen > 0)
     {
        logtxt = calloc(loglen, sizeof(char));
        if (logtxt)
          {
             glGetShaderInfoLog(target, loglen, &chars, logtxt);
             ERR("Failed to %s: %s", action, logtxt);
             free(logtxt);
          }
     }

   /* Program info log */
   glGetProgramiv(target, GL_INFO_LOG_LENGTH, &loglen);
   if (loglen > 0)
     {
        logtxt = calloc(loglen, sizeof(char));
        if (logtxt)
          {
             glGetProgramInfoLog(target, loglen, &chars, logtxt);
             ERR("Failed to %s: %s", action, logtxt);
             free(logtxt);
          }
     }
}

static Evas_GL_Program *
_evas_gl_common_shader_program_binary_load(Eet_File *ef, unsigned int flags)
{
   int num = 0, length = 0;
   int *formats = NULL;
   void *data = NULL;
   char pname[32];
   GLint ok = 0, prg, vtx = GL_NONE, frg = GL_NONE;
   Evas_GL_Program *p = NULL;
   Eina_Bool direct = 1;

   if (!ef || !glsym_glProgramBinary) return NULL;

   sprintf(pname, SHADER_PROG_NAME_FMT, flags);
   data = (void *) eet_read_direct(ef, pname, &length);
   if (!data)
     {
        data = eet_read(ef, pname, &length);
        direct = 0;
     }
   if ((!data) || (length <= 0)) goto finish;

   glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num);
   if (num <= 0) goto finish;

   formats = calloc(num, sizeof(int));
   if (!formats) goto finish;

   glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, formats);
   if (!formats[0]) goto finish;

   prg = glCreateProgram();

#if 1
   // TODO: invalid rendering error occurs when attempting to use a
   // glProgramBinary. in order to render correctly we should create a dummy
   // vertex shader.
   vtx = glCreateShader(GL_VERTEX_SHADER);
   glAttachShader(prg, vtx);
   frg = glCreateShader(GL_FRAGMENT_SHADER);
   glAttachShader(prg, frg);
#endif
   glsym_glProgramBinary(prg, formats[0], data, length);

   glBindAttribLocation(prg, SHAD_VERTEX,  "vertex");
   glBindAttribLocation(prg, SHAD_COLOR,   "color");
   glBindAttribLocation(prg, SHAD_TEXUV,   "tex_coord");
   glBindAttribLocation(prg, SHAD_TEXUV2,  "tex_coord2");
   glBindAttribLocation(prg, SHAD_TEXUV3,  "tex_coord3");
   glBindAttribLocation(prg, SHAD_TEXA,    "tex_coorda");
   glBindAttribLocation(prg, SHAD_TEXSAM,  "tex_sample");
   glBindAttribLocation(prg, SHAD_MASK,    "mask_coord");
   glBindAttribLocation(prg, SHAD_MASKSAM, "tex_masksample");

   glGetProgramiv(prg, GL_LINK_STATUS, &ok);
   if (!ok)
     {
        gl_compile_link_error(prg, "load a program object");
        ERR("Abort load of program (%s)", pname);
        glDeleteProgram(prg);
        glDeleteShader(vtx);
        glDeleteShader(frg);
        goto finish;
     }

   p = calloc(1, sizeof(*p));
   p->flags = flags;
   p->prog = prg;
   p->vert = vtx;
   p->frag = frg;
   p->reset = EINA_TRUE;
   p->bin_saved = EINA_TRUE;
   evas_gl_common_shader_textures_bind(p);

finish:
   free(formats);
   if (!direct) free(data);
   return p;
}

static int
_evas_gl_common_shader_program_binary_save(Evas_GL_Program *p, Eet_File *ef)
{
   void* data = NULL;
   GLenum format;
   int length = 0, size = 0;
   char pname[32];

   if (!glsym_glGetProgramBinary) return 0;

   glGetProgramiv(p->prog, GL_PROGRAM_BINARY_LENGTH, &length);
   if (length <= 0) return 0;

   data = malloc(length);
   if (!data) return 0;

   glsym_glGetProgramBinary(p->prog, length, &size, &format, data);

   if (length != size)
     {
        free(data);
        return 0;
     }

   sprintf(pname, SHADER_PROG_NAME_FMT, p->flags);
   if (eet_write(ef, pname, data, length, SHADER_BINARY_EET_COMPRESS) < 0)
     {
        free(data);
        return 0;
     }

   free(data);
   p->bin_saved = 1;
   return 1;
}

static int
_evas_gl_common_shader_binary_init(Evas_GL_Shared *shared)
{
   Eet_File *ef = NULL;
   char bin_dir_path[PATH_MAX];
   char bin_file_path[PATH_MAX];

   if (!shared || !shared->info.bin_program)
     return 1;

   if (shared->shaders_cache)
     return 1;

   if (!evas_gl_common_file_cache_dir_check(bin_dir_path, sizeof(bin_dir_path)))
     return 0;

   if (!evas_gl_common_file_cache_file_check(bin_dir_path, "binary_shader", bin_file_path,
                                             sizeof(bin_dir_path)))
     return 0;

   if (!eet_init()) return 0;
   ef = eet_open(bin_file_path, EET_FILE_MODE_READ);
   if (!ef) goto error;

   shared->shaders_cache = ef;
   return 1;

error:
   if (ef) eet_close(ef);
   eet_shutdown();
   return 0;
}

static int
_evas_gl_common_shader_binary_save(Evas_GL_Shared *shared)
{
   // FIXME: Save should happen early, before shutdown.

   char bin_dir_path[PATH_MAX];
   char bin_file_path[PATH_MAX];
   char tmp_file[PATH_MAX];
   int tmpfd, res = 0, copy;
   Eet_File *ef = NULL;
   Evas_GL_Program *p;
   Eina_Iterator *it;
   char pname[32];

   if (!evas_gl_common_file_cache_dir_check(bin_dir_path, sizeof(bin_dir_path)))
     {
        res = evas_gl_common_file_cache_mkpath(bin_dir_path);
        if (!res) return 0; /* we can't make directory */
     }

   copy = evas_gl_common_file_cache_file_check(bin_dir_path, "binary_shader", bin_file_path,
                                               sizeof(bin_dir_path));

   /* use mkstemp for writing */
   snprintf(tmp_file, sizeof(tmp_file), "%s.XXXXXX", bin_file_path);

#ifndef _WIN32
   mode_t old_umask = umask(S_IRWXG|S_IRWXO);
#endif
   tmpfd = mkstemp(tmp_file);
#ifndef _WIN32
   umask(old_umask);
#endif

   if (tmpfd < 0) return 0;
   close(tmpfd);

   /* use eet */
   if (!eet_init()) return 0;

   /* copy old file */
   if (copy)
     eina_file_copy(bin_file_path, tmp_file, EINA_FILE_COPY_DATA, NULL, NULL);

   ef = eet_open(tmp_file, EET_FILE_MODE_READ_WRITE);
   if (!ef) goto error;

   it = eina_hash_iterator_data_new(shared->shaders_hash);
   EINA_ITERATOR_FOREACH(it, p)
     {
        if (!p->bin_saved)
          {
             int len = 0;
             sprintf(pname, SHADER_PROG_NAME_FMT, p->flags);
             eet_read_direct(ef, pname, &len);
             if (len > 0)
               p->bin_saved = 1; // assume bin data is correct
             else
               _evas_gl_common_shader_program_binary_save(p, ef);
          }
     }
   eina_iterator_free(it);

   if (shared->shaders_cache)
     {
        eet_close(shared->shaders_cache);
        shared->shaders_cache = NULL;
        eet_shutdown();
     }

   if (eet_close(ef) != EET_ERROR_NONE) goto error;
   if (rename(tmp_file, bin_file_path) < 0) goto error;
   eet_shutdown();
   return 1;

error:
   if (ef) eet_close(ef);
   if (evas_gl_common_file_cache_file_exists(tmp_file)) unlink(tmp_file);
   eet_shutdown();
   return 0;
}

static void
_shaders_hash_free_cb(void *data)
{
   Evas_GL_Program *p = data;
   if (p->vert) glDeleteShader(p->vert);
   if (p->frag) glDeleteShader(p->frag);
   if (p->prog) glDeleteProgram(p->prog);
   free(p);
}

int
evas_gl_common_shader_program_init(Evas_GL_Shared *shared)
{
   /* most popular shaders */
   const int BGRA = (shared->info.bgra ? SHADER_FLAG_BGRA : 0);
   const int autoload[] = {
      /* rect */ BASEFLAG,
      /* text */ BASEFLAG | SHADER_FLAG_TEX | SHADER_FLAG_ALPHA,
      /* img1 */ BASEFLAG | SHADER_FLAG_TEX | SHADER_FLAG_IMG | BGRA,
      /* img2 */ BASEFLAG | SHADER_FLAG_TEX | SHADER_FLAG_IMG | SHADER_FLAG_NOMUL | BGRA,
   };
   unsigned i;

   shared->shaders_hash = eina_hash_int32_new(_shaders_hash_free_cb);
   if (_evas_gl_common_shader_binary_init(shared))
     {
        for (i = 0; i < (sizeof(autoload) / sizeof(autoload[0])); i++)
          {
             Evas_GL_Program *p;

             p = _evas_gl_common_shader_program_binary_load(shared->shaders_cache, autoload[i]);
             if (p) eina_hash_add(shared->shaders_hash, &autoload[i], p);
          }
     }

   return 1;
}

void
evas_gl_common_shader_program_init_done(void)
{
#warning FIXME: Disabled compiler unload for now.
   return;
#ifdef GL_GLES
   glReleaseShaderCompiler();
#else
   if (glsym_glReleaseShaderCompiler)
     glsym_glReleaseShaderCompiler();
#endif
}

void
evas_gl_common_shader_program_shutdown(Evas_GL_Shared *shared)
{
   if (!shared) return;

   if (shared->info.bin_program)
     _evas_gl_common_shader_binary_save(shared);

   if (shared->shaders_cache)
     {
        eet_close(shared->shaders_cache);
        shared->shaders_cache = NULL;
        eet_shutdown();
     }

   eina_hash_free(shared->shaders_hash);
   shared->shaders_hash = NULL;
}

static inline unsigned int
evas_gl_common_shader_flags_get(Evas_GL_Shared *shared, Shader_Type type,
                                RGBA_Map_Point *map_points, int npoints,
                                int r, int g, int b, int a,
                                int sw, int sh, int w, int h, Eina_Bool smooth,
                                Evas_GL_Texture *tex, Eina_Bool tex_only,
                                Evas_GL_Texture *mtex, Eina_Bool mask_smooth,
                                int mw, int mh,
                                Shader_Sampling *psam, int *pnomul, Shader_Sampling *pmasksam)
{
   Shader_Sampling sam = SHD_SAM11, masksam = SHD_SAM11;
   int nomul = 1, bgra = 0, afill = 0, k;
   unsigned int flags = BASEFLAG;

   // image downscale sampling
   if (smooth && ((type == SHD_IMAGE) || (type == SHD_IMAGENATIVE)))
     {
        if ((sw >= (w * 2)) && (sh >= (h * 2)))
          sam = SHD_SAM22;
        else if (sw >= (w * 2))
          sam = SHD_SAM21;
        else if (sh >= (h * 2))
          sam = SHD_SAM12;
        flags |= (1 << (SHADER_FLAG_SAM_BITSHIFT + sam - 1));
     }

   // mask downscale sampling
   if (mtex && mask_smooth)
     {
        if ((mtex->w >= (mw * 2)) && (mtex->h >= (mh * 2)))
          masksam = SHD_SAM22;
        else if (mtex->w >= (mw * 2))
          masksam = SHD_SAM21;
        else if (mtex->h >= (mh * 2))
          masksam = SHD_SAM12;
        flags |= (1 << (SHADER_FLAG_MASKSAM_BITSHIFT + masksam - 1));
     }

   switch (type)
     {
      case SHD_RECT:
      case SHD_LINE:
        goto end;
      case SHD_FONT:
        flags |= (SHADER_FLAG_ALPHA | SHADER_FLAG_TEX);
        goto end;
      case SHD_IMAGE:
        flags |= SHADER_FLAG_IMG;
        break;
      case SHD_IMAGENATIVE:
        break;
      case SHD_YUV:
        flags |= SHADER_FLAG_YUV;
        break;
      case SHD_YUY2:
        flags |= SHADER_FLAG_YUY2;
        break;
      case SHD_NV12:
        flags |= SHADER_FLAG_NV12;
        break;
      case SHD_YUV_709:
        flags |= (SHADER_FLAG_YUV_709 | SHADER_FLAG_YUV);
        break;
      case SHD_RGB_A_PAIR:
      case SHD_MAP:
        break;
      default:
        CRI("Impossible shader type.");
        return 0;
     }

   // color mul
   if ((a == 255) && (r == 255) && (g == 255) && (b == 255))
     {
        if (map_points)
          {
             for (k = 0; k < npoints; k++)
               if (map_points[k].col != 0xffffffff)
                 {
                    nomul = 0;
                    break;
                 }
          }
     }
   else
     nomul = 0;

   if (nomul)
     flags |= SHADER_FLAG_NOMUL;

   // bgra
   if (tex_only)
     {
        if (tex->pt->dyn.img)
          {
             afill = !tex->alpha;
             bgra = 1;
          }
        else if (tex->im && tex->im->native.target == GL_TEXTURE_EXTERNAL_OES)
          {
             flags |= SHADER_FLAG_EXTERNAL;
             afill = !tex->alpha;
          }
        else
          bgra = 1;
     }
   else
     bgra = shared->info.bgra;

   if (tex)
     flags |= SHADER_FLAG_TEX;

   if (mtex)
     flags |= SHADER_FLAG_MASK;

   if (afill)
     flags |= SHADER_FLAG_AFILL;

   if (bgra)
     flags |= SHADER_FLAG_BGRA;

end:
   if (psam) *psam = sam;
   if (pnomul) *pnomul = nomul;
   if (pmasksam) *pmasksam = masksam;
   return flags;
}

static char *
evas_gl_common_shader_glsl_get(unsigned int flags, const char *base)
{
   Eina_Strbuf *s = eina_strbuf_new();
   unsigned int k;
   char *str;

   for (k = 0; k < SHADER_FLAG_COUNT; k++)
     {
        if (flags & (1 << k))
          eina_strbuf_append_printf(s, "#define SHD_%s\n", _shader_flags[k]);
     }

   eina_strbuf_append(s, base);
   str = eina_strbuf_string_steal(s);
   eina_strbuf_free(s);
   return str;
}

static Evas_GL_Program *
evas_gl_common_shader_compile(unsigned int flags, const char *vertex,
                              const char *fragment)
{
   Evas_GL_Program *p;
   GLuint vtx, frg, prg;
   GLint ok = 0;

   vtx = glCreateShader(GL_VERTEX_SHADER);
   frg = glCreateShader(GL_FRAGMENT_SHADER);

   glShaderSource(vtx, 1, &vertex, NULL);
   glCompileShader(vtx);
   glGetShaderiv(vtx, GL_COMPILE_STATUS, &ok);
   if (!ok)
     {
        gl_compile_link_error(vtx, "compile vertex shader");
        ERR("Abort compile of vertex shader:\n%s", vertex);
        glDeleteShader(vtx);
        return NULL;
     }
   ok = 0;

   glShaderSource(frg, 1, &fragment, NULL);
   glCompileShader(frg);
   glGetShaderiv(frg, GL_COMPILE_STATUS, &ok);
   if (!ok)
     {
        gl_compile_link_error(frg, "compile fragment shader");
        ERR("Abort compile of fragment shader:\n%s", fragment);
        glDeleteShader(vtx);
        glDeleteShader(frg);
        return NULL;
     }
   ok = 0;

   prg = glCreateProgram();
#ifndef GL_GLES
   if ((glsym_glGetProgramBinary) && (glsym_glProgramParameteri))
     glsym_glProgramParameteri(prg, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
#endif
   glAttachShader(prg, vtx);
   glAttachShader(prg, frg);

   glBindAttribLocation(prg, SHAD_VERTEX,  "vertex");
   glBindAttribLocation(prg, SHAD_COLOR,   "color");
   glBindAttribLocation(prg, SHAD_TEXUV,   "tex_coord");
   glBindAttribLocation(prg, SHAD_TEXUV2,  "tex_coord2");
   glBindAttribLocation(prg, SHAD_TEXUV3,  "tex_coord3");
   glBindAttribLocation(prg, SHAD_TEXA,    "tex_coorda");
   glBindAttribLocation(prg, SHAD_TEXSAM,  "tex_sample");
   glBindAttribLocation(prg, SHAD_MASK,    "mask_coord");
   glBindAttribLocation(prg, SHAD_MASKSAM, "tex_masksample");

   glLinkProgram(prg);
   glGetProgramiv(prg, GL_LINK_STATUS, &ok);
   if (!ok)
     {
        gl_compile_link_error(prg, "link fragment and vertex shaders");
        ERR("Abort compile of shader (flags: %#x)", flags);
        glDeleteShader(vtx);
        glDeleteShader(frg);
        glDeleteProgram(prg);
        return 0;
     }

   p = calloc(1, sizeof(*p));
   p->flags = flags;
   p->prog = prg;
   p->vert = vtx;
   p->frag = frg;
   p->reset = EINA_TRUE;

   return p;
}

void
evas_gl_common_shader_textures_bind(Evas_GL_Program *p)
{
   struct {
      const char *name;
      int enabled;
   } textures[] = {
      { "tex", 0 },
      { "texm", 0 },
      { "texa", 0 },
      { "texu", 0 },
      { "texv", 0 },
      { "texuv", 0 },
      { NULL, 0 }
   };
   Eina_Bool hastex = 0;
   GLint loc;
   int i;

   if (!p || (p->tex_count > 0)) return;

   if ((p->flags & SHADER_FLAG_TEX) != 0)
     {
        textures[0].enabled = 1;
        hastex = 1;
     }
   if ((p->flags & SHADER_FLAG_MASK) != 0)
     {
        textures[1].enabled = 1;
        hastex = 1;
     }
   if ((p->flags & SHADER_FLAG_RGB_A_PAIR) != 0)
     {
        textures[2].enabled = 1;
        hastex = 1;
     }
   if (p->flags & SHADER_FLAG_YUV)
     {
        textures[3].enabled = 1;
        textures[4].enabled = 1;
        hastex = 1;
     }
   else if ((p->flags & SHADER_FLAG_NV12) || (p->flags & SHADER_FLAG_YUY2))
     {
        textures[5].enabled = 1;
        hastex = 1;
     }

   if (hastex)
     {
        glUseProgram(p->prog); // is this necessary??
        for (i = 0; textures[i].name; i++)
          {
             if (!textures[i].enabled) continue;
             loc = glGetUniformLocation(p->prog, textures[i].name);
             if (loc < 0)
               {
                  ERR("Couldn't find uniform '%s' (shader: %#x)",
                      textures[i].name, p->flags);
               }
             glUniform1i(loc, p->tex_count++);
          }
     }
}

Evas_GL_Program *
evas_gl_common_shader_program_get(Evas_Engine_GL_Context *gc,
                                  Shader_Type type,
                                  RGBA_Map_Point *map_points, int npoints,
                                  int r, int g, int b, int a,
                                  int sw, int sh, int w, int h, Eina_Bool smooth,
                                  Evas_GL_Texture *tex, Eina_Bool tex_only,
                                  Evas_GL_Texture *mtex, Eina_Bool mask_smooth,
                                  int mw, int mh,
                                  Shader_Sampling *psam, int *pnomul,
                                  Shader_Sampling *pmasksam)
{
   unsigned int flags;
   Evas_GL_Program *p;

   flags = evas_gl_common_shader_flags_get(gc->shared, type, map_points, npoints, r, g, b, a,
                                           sw, sh, w, h, smooth, tex, tex_only,
                                           mtex, mask_smooth, mw, mh,
                                           psam, pnomul, pmasksam);
   p = eina_hash_find(gc->shared->shaders_hash, &flags);
   if (!p)
     {
        char *vertex, *fragment;

        _evas_gl_common_shader_binary_init(gc->shared);
        if (gc->shared->shaders_cache)
          {
             char pname[32];
             sprintf(pname, SHADER_PROG_NAME_FMT, flags);
             p = _evas_gl_common_shader_program_binary_load(gc->shared->shaders_cache, flags);
             if (p)
               {
                  eina_hash_add(gc->shared->shaders_hash, &flags, p);
                  goto end;
               }
          }

        vertex = evas_gl_common_shader_glsl_get(flags, vertex_glsl);
        fragment = evas_gl_common_shader_glsl_get(flags, fragment_glsl);

        p = evas_gl_common_shader_compile(flags, vertex, fragment);
        evas_gl_common_shader_textures_bind(p);
        eina_hash_add(gc->shared->shaders_hash, &flags, p);

        free(vertex);
        free(fragment);
     }
end:
   if (p->hitcount < PROGRAM_HITCOUNT_MAX)
     p->hitcount++;
   return p;
}
