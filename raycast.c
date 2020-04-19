#include <math.h>
#include "raycast.h"
#include "common.h"

void ray_init(int x, int screen_height, struct camera_s* camera, struct ray_s* ray) {
    /* calculate ray pos and dir */
    double cameraX = 2 * x / (double)screen_height-1;
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
