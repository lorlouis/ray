#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

#include "raycast.h"
#include "common.h"
#include "texture.h"

#define MS_TO_US(x) (x*1000)
/* 1 for inverse mouse mov
 * -1 for normal mouse */
#define MOUSE_DIR -1

#define XSIZE 640
#define YSIZE 480
#define FRAMEDELAY 16

#define XTEX 32
#define YTEX 32

/* missing texture */
ColorARGB missing_tex_arr[4] = {
    COLOR_AMAGENTA, COLOR_ABLACK,
    COLOR_ABLACK, COLOR_AMAGENTA
};
Texture tex_missing = {2,2, missing_tex_arr};
/* an array of loaded textures */
Texture *textures;
int nb_tex = 0;

/* screen buffer */
ColorARGB pixels[XSIZE * YSIZE] = {0};

int main() {
    int time, old_time;
    /* prevent freeze on load */
    time = SDL_GetTicks();
    int data[24][24] = {
        {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,7,7,7,7,7,7,7,7},
        {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
        {4,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
        {4,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
        {4,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
        {4,0,4,0,0,0,0,5,5,5,5,5,5,5,5,5,7,7,0,7,7,7,7,7},
        {4,0,5,0,0,0,0,5,0,5,0,5,0,5,0,5,7,0,0,0,7,7,7,1},
        {4,0,6,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
        {4,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,1},
        {4,0,8,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
        {4,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,7,7,7,1},
        {4,0,0,0,0,0,0,5,5,5,5,0,5,5,5,5,7,7,7,7,7,7,7,1},
        {6,6,6,6,6,6,6,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
        {6,6,6,6,6,6,0,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
        {4,4,4,4,4,4,0,4,4,4,6,0,6,2,2,2,2,2,2,2,3,3,3,3},
        {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
        {4,0,0,0,0,0,0,0,0,0,0,0,6,2,0,0,5,0,0,2,0,0,0,2},
        {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
        {4,0,6,0,6,0,0,0,0,4,6,0,0,0,0,0,5,0,0,0,0,0,0,2},
        {4,0,0,5,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
        {4,0,6,0,6,0,0,0,0,4,6,0,6,2,0,0,5,0,0,2,0,0,0,2},
        {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
        {4,4,4,4,4,4,4,4,4,4,1,1,1,2,2,2,2,2,2,3,3,3,3,3}
    };

    struct worldMap_s map;
    map.width = 24;
    map.height = 24;
    map.data = &data[0][0];

    struct camera_s camera;
    camera.pos.x.dval = 22;
    camera.pos.y.dval = 11.5;
    camera.dir.x.dval = -1;
    camera.dir.y.dval = 0;

    /* 2d raycast camera plane*/
    camera.plane.x.dval = 0;
    camera.plane.y.dval = 0.66;

    /* init win and texture*/
    SDL_Window *sdl_window = 0;
    SDL_Texture *sdl_tex = 0;
    SDL_Renderer *sdl_renderer = 0;
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialise:\n%s\n", SDL_GetError());
        return 1;
    }

    sdl_window = SDL_CreateWindow(
            "A test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            XSIZE, YSIZE, SDL_WINDOW_SHOWN);
    if(!sdl_window) {
        printf("Window could no be created:\n%s\n", SDL_GetError());
        return 1;
    }

    sdl_renderer = SDL_CreateRenderer(
            sdl_window, -1, SDL_RENDERER_SOFTWARE);
    if(!sdl_renderer) {
        printf("renderer could no be created:\n%s\n", SDL_GetError());
        return 1;
    }

    sdl_tex = SDL_CreateTexture(
            sdl_renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STREAMING,
            XSIZE, YSIZE);
    if(!sdl_tex) {
        printf("The screen texture buffer could not be instanciated:\n%s\n", SDL_GetError());
        return 1;
    }
    SDL_SetRelativeMouseMode(1);
    /* it is now safe to use the window */

    /* main loop */

    for(;;) {
        /* clean */
        /* 255 is opaque */
        /* SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); */
        /* SDL_RenderClear(sdl_renderer);*/
        /* clean the buffer */
        memset(pixels, 0, sizeof(ColorARGB)*XSIZE*YSIZE);


        /* render */
        raycast_render_to_pixels_arr(XSIZE, YSIZE, &camera, pixels, &map);

        /* raycast_render(XSIZE, YSIZE, &camera, sdl_renderer, &map); */
        SDL_UpdateTexture(sdl_tex, NULL, pixels, 4*XSIZE);
        SDL_RenderCopy(sdl_renderer, sdl_tex, NULL, NULL);
        SDL_RenderPresent(sdl_renderer);

        old_time = time;
        time = SDL_GetTicks();
        /* frameTime is the time this frame has taken, in seconds */
        double frameTime = (time - old_time) / 1000.0;

        /* fix the fps */
        if (FRAMEDELAY > time - old_time) {
            SDL_Delay(FRAMEDELAY - (time - old_time));
            frameTime = FRAMEDELAY / 1000.0;
        }


        /* the constant value is in squares/second */
        double moveSpeed = frameTime * 4.0;
        /* the constant value is in radians/second */
        double rotSpeed = frameTime * 1.0;
        double VrotSpeed = frameTime * 0.2;

        printf("V angle %f\n", camera.angle_v);
        printf("X: %f, Y: %f\n", camera.pos.x.dval, camera.pos.y.dval);
        printf("plane X: %f, plane Y: %f\n", camera.plane.x.dval, camera.plane.y.dval);
        printf("FPS: %f\n", 1.0 / frameTime);
        printf("dirx %f\n", camera.dir.x.dval);
        printf("diry %f\n", camera.dir.y.dval);
        
        /* place the old values outside of the loop
         * becaused it caused a race condition */
        double oldDirX = camera.dir.x.dval;
        double oldDirY = camera.dir.y.dval;
        double oldPlaneX = camera.plane.x.dval;
        double oldPlaneY = camera.plane.y.dval;
        SDL_Event events;
        while(SDL_PollEvent(&events)) {
            switch(events.type) {
                case SDLK_q:
                case SDL_QUIT:
                    goto end;
                    return 0;
                case SDL_KEYDOWN:
                    /* single press stuff */
                    switch (events.key.keysym.sym) {
                        case 'q':
                            goto end;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    camera.dir.x.dval = oldDirX * cos(-rotSpeed * events.motion.xrel) - oldDirY * sin(-rotSpeed * events.motion.xrel);
                    camera.dir.y.dval = oldDirX * sin(-rotSpeed * events.motion.xrel) + oldDirY * cos(-rotSpeed * events.motion.xrel);
                    camera.plane.x.dval = oldPlaneX * cos(-rotSpeed * events.motion.xrel) - oldPlaneY * sin(-rotSpeed * events.motion.xrel);
                    camera.plane.y.dval = oldPlaneX * sin(-rotSpeed * events.motion.xrel) + oldPlaneY * cos(-rotSpeed * events.motion.xrel);
                    /* change the camera's yaw */
                    camera.angle_v += VrotSpeed * events.motion.yrel * MOUSE_DIR;
                break;
            }
        }
        /* prevent the camera from being more than 0.5 screen up or down */
        if(camera.angle_v < -2) camera.angle_v = -2;
        if(camera.angle_v > 2) camera.angle_v = 2;

        const unsigned char *state = SDL_GetKeyboardState(0);
        if(state[SDL_SCANCODE_W]) {
            if(map.data[(int)(camera.pos.x.dval + camera.dir.x.dval * (moveSpeed + 0.1))* map.width + (int)camera.pos.y.dval] == 0)
                camera.pos.x.dval += camera.dir.x.dval * moveSpeed;
            if(map.data[(int)camera.pos.x.dval * map.width + (int)(camera.pos.y.dval + camera.dir.y.dval * (moveSpeed + 0.1))] == 0)
                camera.pos.y.dval += camera.dir.y.dval * moveSpeed;
        }
        if(state[SDL_SCANCODE_S]) {
            if(map.data[(int)(camera.pos.x.dval - camera.dir.x.dval * (moveSpeed + 0.1))* map.width + (int)camera.pos.y.dval] == 0)
                camera.pos.x.dval -= camera.dir.x.dval * moveSpeed;
            if(map.data[(int)camera.pos.x.dval * map.width + (int)(camera.pos.y.dval - camera.dir.y.dval * (moveSpeed + 0.1))] == 0)
                camera.pos.y.dval -= camera.dir.y.dval * moveSpeed;
        }
        if(state[SDL_SCANCODE_D]) {
            if(map.data[(int)(camera.pos.x.dval + camera.dir.y.dval * (moveSpeed + 0.1))* map.width + (int)camera.pos.y.dval] == 0)
                camera.pos.x.dval += camera.dir.y.dval * moveSpeed;
            if(map.data[(int)camera.pos.x.dval * map.width + (int)(camera.pos.y.dval - camera.dir.x.dval * (moveSpeed + 0.1))] == 0)
                camera.pos.y.dval -= camera.dir.x.dval * moveSpeed;
        }
        if(state[SDL_SCANCODE_A]) {
            if(map.data[(int)(camera.pos.x.dval - camera.dir.y.dval * moveSpeed)* map.width + (int)camera.pos.y.dval] == 0)
                camera.pos.x.dval -= camera.dir.y.dval * moveSpeed;
            if(map.data[(int)camera.pos.x.dval * map.width + (int)(camera.pos.y.dval + camera.dir.x.dval * moveSpeed)] == 0)
                camera.pos.y.dval += camera.dir.x.dval * moveSpeed;
        }

    }
end:
    SDL_DestroyWindow(sdl_window);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_Quit();
    return 0;
}
