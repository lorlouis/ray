#ifndef RAYCAST_H
#define RAYCAST_H 1

#include "common.h"
#include "texture.h"
#include <SDL2/SDL.h>

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

/* render using a raycaster */
void raycast_render(int screen_height, int screen_width, struct camera_s *camera, SDL_Renderer *sdl_renderer,struct worldMap_s *map);
/* render to a pixel array */
void raycast_render_to_pixels_arr(int screen_width, int screen_height, struct camera_s *camera, ColorARGB *pixels,struct worldMap_s *map
        , int nb_entities, struct entity_s *entities);
/* render floor and ceiling */
void render_floor_and_ceiling_to_pixel_arr(int screenWidth, int screenHeight, struct camera_s *camera, ColorARGB *pixels,struct worldMap_s *map);
#endif
