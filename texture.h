#ifndef TEXTURE_H
#define TEXTURE_H 1

#define COLOR_RED (ColorRGB){255,0,0}
#define COLOR_GREEN (ColorRGB){0, 255, 0}
#define COLOR_BLUE (ColorRGB){0,0,255}
#define COLOR_YELLOW (ColorRGB){255, 255, 0}
#define COLOR_MAGENTA (ColorRGB){255,0,255}
#define COLOR_WHITE (ColorRGB){255, 255, 255}
#define COLOR_BLACK (ColorRGB){0,0,0}

#include "common.h"

typedef struct texture_s {
    unsigned int width, height;
    ColorRGB *data;
} Texture;

#endif
