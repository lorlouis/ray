#ifndef COMMON_H
#define COMMON_H 1

#include <stdint.h>

typedef struct ColorRGBs {
    unsigned char r, g, b;
} ColorRGB;

/* ARGB8888 word order */
typedef union ColorARGB_s {
    struct {
        unsigned char a, r, g, b;
    } data;
    uint32_t raw;
} ColorARGB;

struct worldMap_s {
    int width, height;
    int *data;
};

#endif
