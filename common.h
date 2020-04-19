#ifndef COMMON_H
#define COMMON_H 1

typedef struct ColorRGBs {
    unsigned char r, g, b;
} ColorRGB;

struct worldMap_s {
    int width, height;
    int *data;
};

#endif
