#include <math.h>
#include "raycast.h"
#include "common.h"
#include "texture.h"

void ray_init(int x, int screen_width, struct camera_s* camera, struct ray_s* ray) {
    /* calculate ray pos and dir */
    double cameraX = 2 * x / (double)screen_width-1;
    /* ray init */
    ray->dir.x.dval = camera->dir.x.dval + camera->plane.x.dval * cameraX;
    ray->dir.y.dval = camera->dir.y.dval + camera->plane.y.dval * cameraX;

    /* set the map box in which we are standing */
    int mapX = (int)camera->pos.x.dval;
    int mapY = (int)camera->pos.y.dval;

    /* len of a segment from one x or y side to next y or x side */
    ray->delta_dist.x.dval = (ray->dir.y.dval == 0) ? 0 : ((ray->dir.x.dval == 0) ? 1 : fabs(1 / ray->dir.x.dval));
    ray->delta_dist.y.dval = (ray->dir.x.dval == 0) ? 0 : ((ray->dir.y.dval == 0) ? 1 : fabs(1 / ray->dir.y.dval));

    /* did the ray hit a wall */
    ray->hit = 0;

    /* calculate step and initial sideDist */
    if (ray->dir.x.dval < 0)
    {
      ray->step.x.ival = -1;
      ray->side_dist.x.dval = (camera->pos.x.dval - mapX) * ray->delta_dist.x.dval;
    }
    else
    {
      ray->step.x.ival = 1;
      ray->side_dist.x.dval = (mapX + 1.0 - camera->pos.x.dval) * ray->delta_dist.x.dval;
    }
    if (ray->dir.y.dval < 0)
    {
      ray->step.y.ival = -1;
      ray->side_dist.y.dval = (camera->pos.y.dval - mapY) * ray->delta_dist.y.dval;
    }
    else
    {
      ray->step.y.ival = 1;
      ray->side_dist.y.dval = (mapY + 1.0 - camera->pos.y.dval) * ray->delta_dist.y.dval;
    }
}

void ray_cast(struct camera_s *camera, struct worldMap_s *map, struct ray_s *ray){
    /* set the map box in which we are standing */
    int mapX = (int)camera->pos.x.dval;
    int mapY = (int)camera->pos.y.dval;
    while (ray->hit == 0)
    {
        /* jump to next map square, OR in x-direction, OR in y-direction */
        if (ray->side_dist.x.dval < ray->side_dist.y.dval)
        {
          ray->side_dist.x.dval += ray->delta_dist.x.dval;
          mapX += ray->step.x.ival;
          ray->side = 0;
        }
        else
        {
          ray->side_dist.y.dval += ray->delta_dist.y.dval;
          mapY += ray->step.y.ival;
          ray->side = 1;
        }
        /* Check if ray has hit a wall */
        if(map->data[mapX*map->width + mapY] > 0){
            ray->hit = 1;
            ray->result.hit = &(map->data[mapX*map->width + mapY]);
        }
    } 
    /* calculate the distance from the camera plane to prevent fish eye effect */
    double perpWallDist;
    if(ray->side == 0)
        perpWallDist = (mapX - camera->pos.x.dval + (1 - ray->step.x.ival) / 2) / ray->dir.x.dval;
    else 
        perpWallDist = (mapY - camera->pos.y.dval + (1 - ray->step.y.ival) / 2) / ray->dir.y.dval;
    ray->result.distance = perpWallDist;
}

void raycast_render(int screen_width, int screen_height, struct camera_s *camera, SDL_Renderer *sdl_renderer,struct worldMap_s *map, Texture *tex_missing){
    int i;
    for(i=0; i<screen_width; i++) {
        struct ray_s ray;
        ray_init(i, screen_width, camera, &ray);

        ray_cast(camera, map, &ray);
        /* CAUTION can be larger than screen y size */
        int lineHeight = (int)(screen_height / ray.result.distance);
        
        /* calculate the lowest and highest pixel */
        int drawStart = -lineHeight / 2 + (screen_height / 2) + camera->angle_v * (screen_height / 2);
        if(drawStart >= screen_height) drawStart = screen_height - 1;
        if(drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + (screen_height / 2) + camera->angle_v * (screen_height / 2);
        if(drawEnd >= screen_height) drawEnd = screen_height - 1;
        if(drawEnd < 0)drawEnd = 0;

        /* if drawStart and drawEnd are the same dont draw anything */
        if(drawEnd == drawStart)
            continue;

        /* give x and y sides a diferent color*/
        Texture tex;
        switch(*(int*)ray.result.hit)
        {
            default: tex = *tex_missing;
        }
        ColorRGB color;
        double wallX;
        if (ray.side) wallX = camera->pos.x.dval + ray.result.distance * ray.dir.x.dval;
        else wallX = camera->pos.y.dval + ray.result.distance * ray.dir.y.dval;
        wallX -= floor(wallX);

        int texX = (int)(wallX * (double)tex.width);
        if(ray.side == 0 && ray.dir.x.dval > 0) texX = tex.width - texX -1;
        if(ray.side == 1 && ray.dir.y.dval < 0) texX = tex.width - texX -1;
        
        /* TODO fix the texture moving with the camera */
        double step_t = 1.0 * tex.height / lineHeight;

        double texPos = ((drawStart - screen_height / 2 + lineHeight / 2) - camera->angle_v * (screen_height / 2)) * step_t;
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
}
