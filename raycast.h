#ifndef RAYCAST_H
#define RAYCAST_H 1

#include "common.h"

union num_u {
    double dval;
    int ival;
};


struct vec_2d_s {
    union num_u x, y;
};

struct ray_result_s {
    double distance;
    void* hit;
    int side;
};

struct ray_s {
    struct  vec_2d_s dir,
        /* len of the ray */
            side_dist,
        /* len from one side to the other */
            delta_dist,
        /* step dir (-1 or 1) */
            step;
    /* side either 0 N or 1 S */
    int side, hit;
    struct ray_result_s result;
};

struct camera_s {
    struct vec_2d_s plane, pos, dir;
    double angle_v;
};

/* initialise a ray */
void ray_init(int x, int screen_height, struct camera_s* camera, struct ray_s* ray);

/* cast the ray and fill the result */
void ray_cast(struct camera_s *camera, struct worldMap_s *map, struct ray_s *ray);
#endif
