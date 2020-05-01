#ifndef COMMON_H
#define COMMON_H 1

struct entity_s {
    int tex_id;
    double x, y, z, distance;
};

struct worldMap_s {
    int width, height, nb_tex;
    int *data;
    char **textures;
};

#endif
