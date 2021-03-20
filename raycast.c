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
        if (ray->side_dist.x.dval < ray->side_dist.y.dval) {
          ray->side_dist.x.dval += ray->delta_dist.x.dval;
          mapX += ray->step.x.ival; ray->side = 0;
        }
        else {
          ray->side_dist.y.dval += ray->delta_dist.y.dval;
          mapY += ray->step.y.ival;
          ray->side = 1;
        }
        /* Check if ray has hit a wall */
        if(map->data[mapX*map->width + mapY] > 0) {
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

void render_floor_and_ceiling_to_pixel_arr(
        int screenWidth, int screenHeight, struct camera_s *camera,
        ColorARGB *pixels,struct worldMap_s *map){
    /* delcared in main */
    extern SDL_Surface *tex_missing;
    extern SDL_Surface **textures;

    int y;
    for(y= screenHeight / 2 + camera->angle_v * (screenHeight / 2); y< screenHeight; y++) {
        struct ray_s ray0, ray1;
        ray0.dir.x.dval = camera->dir.x.dval - camera->plane.x.dval;
        ray0.dir.y.dval = camera->dir.y.dval - camera->plane.y.dval;
        ray1.dir.x.dval = camera->dir.x.dval + camera->plane.x.dval;
        ray1.dir.y.dval = camera->dir.y.dval + camera->plane.y.dval;

        /* INFO might need to shift this with mouse view */
        int screen_y_pos = (y - screenHeight / 2 - camera->angle_v * (screenHeight / 2)) + 1;
        //int screen_y_pos = ;
        printf("%d, %d\n", screen_y_pos, y);
        /* changes height */
        float cam_pos_z = 0.5 * screenHeight;

        float rowDistance = (cam_pos_z / screen_y_pos);

        float floorStepX = rowDistance * (ray1.dir.x.dval - ray0.dir.x.dval) / screenWidth;
        float floorStepY = rowDistance * (ray1.dir.y.dval - ray0.dir.y.dval) / screenWidth;

        float floorX = camera->pos.x.dval + rowDistance * ray0.dir.x.dval;
        float floorY = camera->pos.y.dval + rowDistance * ray0.dir.y.dval;

        int x;
        for(x=0;x<screenWidth; x++) {
            int cellX = (int)floorX;
            int cellY = (int)floorY;
            if(cellX < 0) cellX =0;
            if(cellX > map->width) cellX = map->width;
            if(cellY < 0) cellY = 0;
            if(cellY > map->height) cellY=map->height;

            SDL_Surface *floor_tex;
            /* floor_tex = tex_missing; */
            /* get the value of the cell as an absolute because it is negative */
            if(abs(map->data[cellX * map->width + cellY]) > map->nb_tex
               || abs(map->data[cellX * map->width + cellY]) == 0) {
                floor_tex = tex_missing;
            }
            else {
                /* shift the walls by one so that wall 1 uses texture 0 */
                floor_tex = textures[abs(map->data[cellX * map->width + cellY]) -1];
            }

            /* TODO scale to the tex's size */
            int textureX = (int)(floor_tex->w * (floorX - cellX)) & (floor_tex->w-1);
            int textureY = (int)(floor_tex->h * (floorY - cellY)) & (floor_tex->h-1);

            floorX += floorStepX;
            floorY += floorStepY;

            ColorARGB color;
            color = *(ColorARGB*)(floor_tex->pixels + floor_tex->pitch * textureY + textureX * sizeof(ColorARGB));
            /* skip redraws if the floor is outside of the screen */
            if(y < 0 || y > screenHeight) continue;
            pixels[y * screenWidth + x] = color;
        }
    }
}

void swap(void** a, void** b) {
    void* c = *a;
    *a = *b;
    *b = c;
}

int entity_partition (struct entity_s *entities, double *distances, int low, int high)
{ 
    int pivot = distances[high];
    /* smallest distance */
    int i = (low - 1);
  
    int j;
    for(j = low; j <= high- 1; j++) { 
        if (distances[j] < pivot) { 
            /* inc smallest element */
            i++;
            swap((void*)(distances + i), (void*)(distances + j));
            swap((void*)(entities + i), (void*)(entities + j)); 
        } 
    } 
    swap((void*)(distances + i + 1), (void*)(distances + high)); 
    return (i + 1); 
}

/* TODO reimplement using qsort from stdlib */
void entity_sort(struct entity_s *entities, double *distances, int low, int high) {
    if (low < high) {
        /* pi is partitioning index, arr[p] is now 
           at right place */
        int part_index = entity_partition(entities, distances, low, high);

        /* Separately sort elements before */
        /* partition and after partition */
        entity_sort(entities, distances, low, part_index - 1);
        entity_sort(entities, distances, part_index + 1, high);
    }
}

void raycast_render_to_pixels_arr(int screen_width, int screen_height, struct camera_s *camera, ColorARGB *pixels,struct worldMap_s *map
        , int nb_entities, struct entity_s *entities){

    /* declared in main */
    extern SDL_Surface *tex_missing;
    extern SDL_Surface **textures;

    /* fill the z_buffer when casting a ray */
    double ZBuffer[screen_width];

    int i;
    for(i=0; i<screen_width; i++) {
        struct ray_s ray;
        ray_init(i, screen_width, camera, &ray);

        ray_cast(camera, map, &ray);

        /* set the distance for that line */
        ZBuffer[i] = ray.result.distance;

        /* CAUTION can be larger than screen y size */
        int lineHeight = (int)(screen_height / ray.result.distance);

        /* calculate the lowest and highest pixel */
        int drawStart = -lineHeight / 2 + (screen_height / 2) + camera->angle_v * (screen_height / 2);
        if(drawStart >= screen_height) drawStart = screen_height;
        if(drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + (screen_height / 2) + camera->angle_v * (screen_height / 2);
        if(drawEnd >= screen_height) drawEnd = screen_height;
        if(drawEnd < 0)drawEnd = 0;

        /* if drawStart and drawEnd are the same dont draw anything */
        if(drawEnd == drawStart)
            continue;

        /* load a texture and if it cannot be found link it to missing tex */
        SDL_Surface *tex;
        if(*(int*)ray.result.hit > map->nb_tex) {
            tex = tex_missing;
        }
        else {
            /* shift the walls by one so that wall 1 uses texture 0 */
            tex = textures[*(int*)ray.result.hit -1];
        }
        ColorARGB color;
        double wallX;
        if (ray.side) wallX = camera->pos.x.dval + ray.result.distance * ray.dir.x.dval;
        else wallX = camera->pos.y.dval + ray.result.distance * ray.dir.y.dval;
        wallX -= floor(wallX);

        int texX = (int)(wallX * (double)tex->w);
        if(ray.side == 0 && ray.dir.x.dval > 0) texX = tex->w - texX -1;
        if(ray.side == 1 && ray.dir.y.dval < 0) texX = tex->w - texX -1;

        double step_t = 1.0 * tex->h / lineHeight;

        double texPos = ((drawStart - screen_height / 2 + lineHeight / 2) - camera->angle_v * (screen_height / 2)) * step_t;
        int k;
        for(k=drawStart; k<drawEnd; k++)
        {
            int texY = (int)texPos & (tex->h-1);
            texPos += step_t;
            color = *(ColorARGB*)(tex->pixels + texY * tex->pitch + texX * sizeof(ColorARGB));
            if (ray.side == 1)
                color = (ColorARGB){{
                    255,
                    color.data.r != 0 ? color.data.r /2: 0,
                    color.data.g != 0 ? color.data.g /2: 0,
                    color.data.b != 0 ? color.data.b /2: 0}};
            pixels[i + k * screen_width] = color;
        }
    }
    /* sort the entities */
    double distances[nb_entities];
    for(i=0; i< nb_entities; i++) {
        distances[i] = (
                (camera->pos.x.dval - (entities+i)->x)
                * (camera->pos.x.dval - (entities+i)->x)
                + (camera->pos.y.dval - (entities+i)->y)
                * (camera->pos.y.dval - (entities+i)->y));
    }
    entity_sort(entities, distances, 0, nb_entities-1);

    /* render the sprites */
    for(i=0; i<nb_entities; i++) {
        /* translate the entity's position to camera space */
        double ent_x = (entities + i)->x - camera->pos.x.dval;
        double ent_y = (entities + i)->y - camera->pos.y.dval;

        /* transform with inverse camera matrix */
        double inverse_det = 1.0 / (
                camera->plane.x.dval * camera->dir.y.dval
                - camera->dir.x.dval * camera->plane.y.dval);
        double transform_x = (
                inverse_det * (camera->dir.y.dval * ent_x
                - camera->dir.x.dval * ent_y));
        double transform_y = (
                inverse_det * (-camera->plane.y.dval * ent_x
                + camera->plane.x.dval * ent_y));

        int ent_screen_x = (int)((screen_width / 2)
                * (1 + transform_x / transform_y));

        int ent_height = abs((int)(screen_height / transform_y));

        /* lowest and highest screen space pixels of the sprite */
        int draw_start_y = -ent_height / 2 + screen_height / 2;
        if(draw_start_y < 0 ) draw_start_y = 0;
        if(draw_start_y >= screen_height) draw_start_y = screen_height-1;

        int draw_end_y = ent_height / 2 + screen_height / 2;
        if(draw_end_y < 0) draw_end_y = 0;
        if(draw_end_y >= screen_height) draw_end_y = screen_height-1;

        /* leftest and rightest pixels of the spite */
        int ent_width = abs((int)(screen_height / transform_y));
        int draw_start_x = -ent_width / 2 + ent_screen_x;
        if(draw_start_x < 0 ) draw_start_x = 0;
        /* TODO might be useless */
        if(draw_start_x >= screen_width) draw_start_x = screen_width-1;

        int draw_end_x = ent_width / 2 + ent_screen_x;
        /* TODO might be useless */
        if(draw_end_x < 0) draw_end_x = 0;
        if(draw_end_x >= screen_width) draw_end_x = screen_width-1;

        /* draw the sprite */
        int stripe;
        for(stripe = draw_start_x; stripe < draw_end_x; stripe++) {
            int texX = (int)(256 * (stripe - (-ent_width / 2 + ent_screen_x)) * (entities + i)->tex->w / ent_width) / 256;
            if(transform_y > 0
               && stripe > 0
               && stripe < screen_width
               && transform_y < ZBuffer[stripe]) {
                int y;
                for(y = draw_start_y; y < draw_end_y; y++) {
                    int d = y * 256 - screen_height * 128 + ent_height * 128;
                    int tex_y = ((d * (entities + i)->tex->h) / ent_height) / 256;
                }
            }
        }

    }
}
