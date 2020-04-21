#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

#include "raycast.h"
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

int usleep(double usec) {
        struct timespec req, rem;
        int r;

        req.tv_sec = 0;
        req.tv_nsec = usec * 1000;
        while((r = nanosleep(&req, &rem)) == -1)
                req = rem;
        return r;
}
double angle;
int main() {
    int time, old_time;
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
    /* missing texture */
    ColorRGB missing_tex_arr[4] = {
        COLOR_MAGENTA, COLOR_BLACK,
        COLOR_BLUE, COLOR_MAGENTA
    };
    Texture tex_missing = {2,2, missing_tex_arr};

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
    SDL_SetRelativeMouseMode(1);
    /* it is now safe to use the window */

    /* main loop */

    for(;;) {
        /* clean */
        SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(sdl_renderer);
        int i;
        for(i=0; i<XSIZE; i++) {
            struct ray_s ray;
            ray_init(i, XSIZE, &camera, &ray);

            ray_cast(&camera, &map, &ray);
            /* CAUTION can be larger than screen y size */
            int lineHeight = (int)(YSIZE / ray.result.distance);
            
            /* calculate the lowest and highest pixel */
            int drawStart = -lineHeight / 2 + (YSIZE / 2) + camera.angle_v * (YSIZE / 2);
            if(drawStart >= YSIZE) drawStart = YSIZE - 1;
            if(drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + (YSIZE / 2) + camera.angle_v * (YSIZE / 2);
            if(drawEnd >= YSIZE) drawEnd = YSIZE - 1;
            if(drawEnd < 0)drawEnd = 0;

            /* if drawStart and drawEnd are the same dont draw anything */
            if(drawEnd == drawStart)
                continue;

            /* give x and y sides a diferent color*/
            Texture tex;
            switch(*(int*)ray.result.hit)
            {
                default: tex = tex_missing;
            }
            ColorRGB color;
            double wallX;
            if (ray.side) wallX = camera.pos.x.dval + ray.result.distance * ray.dir.x.dval;
            else wallX = camera.pos.y.dval + ray.result.distance * ray.dir.y.dval;
            wallX -= floor(wallX);

            int texX = (int)(wallX * (double)tex.width);
            if(ray.side == 0 && ray.dir.x.dval > 0) texX = tex.width - texX -1;
            if(ray.side == 1 && ray.dir.y.dval < 0) texX = tex.width - texX -1;
            
            /* TODO fix the texture moving with the camera */
            double step_t = 1.0 * tex.height / lineHeight;

            double texPos = ((drawStart - YSIZE / 2 + lineHeight / 2) - camera.angle_v * (YSIZE / 2)) * step_t;
            int k;
            for(k=drawStart; k<drawEnd; k++)
            {
                int texY = (int)texPos & (tex.height-1);
                texPos += step_t;
                color = tex.data[texY + texX * tex.width];
                if (ray.side == 1)
                    color = (ColorRGB){
                        color.r != 0 ? color.r /2: 0,
                        color.g != 0 ? color.g /2: 0,
                        color.b != 0 ? color.b /2: 0};
                SDL_SetRenderDrawColor(sdl_renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
                SDL_RenderDrawPoint(sdl_renderer, i, k);
            }
         }
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
        double moveSpeed = frameTime * 5.0;
        /* the constant value is in radians/second */
        double rotSpeed = frameTime * 1.5;
        double VrotSpeed = frameTime * 0.5;

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
            if(map.data[(int)(camera.pos.x.dval + camera.dir.x.dval * moveSpeed)* map.width + (int)camera.pos.y.dval] == 0)
                camera.pos.x.dval += camera.dir.x.dval * moveSpeed;
            if(map.data[(int)camera.pos.x.dval * map.width + (int)(camera.pos.y.dval + camera.dir.y.dval * moveSpeed)] == 0)
                camera.pos.y.dval += camera.dir.y.dval * moveSpeed;
        }
        if(state[SDL_SCANCODE_S]) {
            if(map.data[(int)(camera.pos.x.dval - camera.dir.x.dval * moveSpeed)* map.width + (int)camera.pos.y.dval] == 0)
                camera.pos.x.dval -= camera.dir.x.dval * moveSpeed;
            if(map.data[(int)camera.pos.x.dval * map.width + (int)(camera.pos.y.dval - camera.dir.y.dval * moveSpeed)] == 0)
                camera.pos.y.dval -= camera.dir.y.dval * moveSpeed;
        }
        if(state[SDL_SCANCODE_D]) {
            if(map.data[(int)(camera.pos.x.dval + camera.dir.y.dval * moveSpeed)* map.width + (int)camera.pos.y.dval] == 0)
                camera.pos.x.dval += camera.dir.y.dval * moveSpeed;
            if(map.data[(int)camera.pos.x.dval * map.width + (int)(camera.pos.y.dval - camera.dir.x.dval * moveSpeed)] == 0)
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
