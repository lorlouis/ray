#ifndef COMMON_H
#define COMMON_H 1

#include "texture.h"

struct entity_s {
    SDL_Surface* tex;
    double x, y, z;
};

struct worldMap_s {
    int width, height, nb_tex;
    int *data;
    char **textures;
};

#endif
