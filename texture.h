#ifndef TEXTURE_H
#define TEXTURE_H 1

#define COLOR_RED (ColorRGB){255,0,0}
#define COLOR_GREEN (ColorRGB){0, 255, 0}
#define COLOR_BLUE (ColorRGB){0,0,255}
#define COLOR_YELLOW (ColorRGB){255, 255, 0}
#define COLOR_MAGENTA (ColorRGB){255,0,255}
#define COLOR_WHITE (ColorRGB){255, 255, 255}
#define COLOR_BLACK (ColorRGB){0,0,0}

/* a r g b */
#define COLOR_ARED (ColorARGB){{255,255,0,0}}
#define COLOR_AGREEN (ColorARGB){{255,0, 255, 0}}
#define COLOR_ABLUE (ColorARGB){{255,0,0,255}}
#define COLOR_AYELLOW (ColorARGB){{255,255, 255, 0}}
#define COLOR_AMAGENTA (ColorARGB){{255,255,0,255}}
#define COLOR_AWHITE (ColorARGB){{255,255, 255, 255}}
#define COLOR_ABLACK (ColorARGB){{255,0,0,0}}

#define TRANSPARENT 0
#define OPAQUE 255

#include "common.h"

typedef struct texture_s {
    unsigned int width, height;
    ColorARGB *data;
} Texture;

extern Texture tex_missing;
extern Texture *textures;
extern int nb_tex;

#endif
