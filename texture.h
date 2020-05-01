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

#include "SDL2/SDL.h"

#include <stdint.h>

/* ARGB8888 word order */
typedef union ColorARGB_s {
    struct {
        unsigned char a, r, g, b;
    } data;
    uint32_t raw;
} ColorARGB;
typedef struct texture_s {
    unsigned int width, height;
    ColorARGB *data;
} Texture;

extern SDL_Surface *tex_missing;
extern SDL_Surface **textures;

#endif
