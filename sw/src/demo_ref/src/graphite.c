// graphite.c
// Copyright (c) 2021-2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Ref.: - One Lone Coder's 3D Graphics Engine tutorial available on YouTube
//       - https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation
//       - https://github.com/raysan5/raylib/blob/master/src/raymath.h

#include "graphite.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sw_rasterizer.h"
#include "upng.h"

static SDL_Renderer* g_renderer;
static int g_fb_width, g_fb_height;

//bool g_rasterizer_barycentric = true;
extern bool g_rasterizer_barycentric;
void draw_triangle(vec3d p[3], vec2d t[3], vec3d c[3], texture_t* tex, bool clamp_s, bool clamp_t,
                      bool depth_test, bool perspective_correct)
{
    if (g_rasterizer_barycentric) {
        sw_draw_triangle_barycentric(FX(p[0].x), FX(p[0].y), FX(t[0].w), FX(t[0].u), FX(t[0].v), FX(c[0].x), FX(c[0].y), FX(c[0].z), FX(c[0].w), FX(p[1].x), FX(p[1].y), FX(t[1].w), FX(t[1].u), FX(t[1].v), FX(c[1].x), FX(c[1].y), FX(c[1].z), FX(c[1].w), FX(p[2].x), FX(p[2].y), FX(t[2].w), FX(t[2].u), FX(t[2].v), FX(c[2].x), FX(c[2].y), FX(c[2].z), FX(c[2].w), tex->addr, tex->scale_x, tex->scale_y, clamp_s, clamp_t, depth_test, perspective_correct);
    } else {
        sw_draw_triangle_standard(FX(p[0].x), FX(p[0].y), FX(t[0].w), FX(t[0].u), FX(t[0].v), FX(c[0].x), FX(c[0].y), FX(c[0].z), FX(c[0].w), FX(p[1].x), FX(p[1].y), FX(t[1].w), FX(t[1].u), FX(t[1].v), FX(c[1].x), FX(c[1].y), FX(c[1].z), FX(c[1].w), FX(p[2].x), FX(p[2].y), FX(t[2].w), FX(t[2].u), FX(t[2].v), FX(c[2].x), FX(c[2].y), FX(c[2].z), FX(c[2].w), tex->addr, tex->scale_x, tex->scale_y, clamp_s, clamp_t, depth_test, perspective_correct);
    }
}

void graphite_init(SDL_Renderer* renderer, int fb_width, int fb_height) {
    g_renderer = renderer;
    g_fb_width = fb_width;
    g_fb_height = fb_height;
}

void get_fb_dimensions(int* fb_width, int* fb_height) {
    *fb_width = g_fb_width;
    *fb_height = g_fb_height;
}

void clear(unsigned int color) {

    // Constants taken from https://stackoverflow.com/a/9069480

    int r5 = color >> 11;
    int g6 = (color >> 5) & 0x3F;
    int b5 = color & 0x1F;

    int r = (r5 * 527 + 23) >> 6;
    int g = (g6 * 259 + 33) >> 6;
    int b = (b5 * 527 + 23) >> 6;

    SDL_SetRenderDrawColor(g_renderer, r, g, b, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(g_renderer);
    if (g_rasterizer_barycentric)
        sw_clear_depth_buffer_barycentric();
    else
        sw_clear_depth_buffer_standard();
}

void swap(void) {
    SDL_RenderPresent(g_renderer);
}

static bool load_mesh_obj_data(mesh_t *mesh, const char *obj_filename) {
    FILE* file;
    file = fopen(obj_filename, "r");

    if (file == NULL)
        return false;

    memset(mesh, 0, sizeof(mesh_t));

    char line[1024];
    while (fgets(line, 1024, file)) {
        // Vertex information
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            vec3d v = {
                .x = x,
                .y = y,
                .z = z,
                .w = 1.0f
            };
            array_push(mesh->vertices, v);
            mesh->nb_vertices++;
        }
        // Vertex normal information
        if (strncmp(line, "vn ", 3) == 0) {
            float x, y, z;
            sscanf(line, "vn %f %f %f", &x, &y, &z);
            vec3d v = {
                .x = x,
                .y = y,
                .z = z,
                .w = 0.0f
            };
            array_push(mesh->normals, v);
            mesh->nb_normals++;
        }        
        // Texture coordinate information
        if (strncmp(line, "vt ", 3) == 0) {
            float u, v;
            sscanf(line, "vt %f %f", &u, &v);
            vec2d t = {
                .u = u,
                .v = 1.0f - v,
                .w = 1.0f
            };
            array_push(mesh->texcoords, t);
            mesh->nb_texcoords++;
        }
        // Face information
        if (strncmp(line, "f ", 2) == 0) {
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];
            sscanf(line,
                "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertex_indices[0], &texture_indices[0], &normal_indices[0],
                &vertex_indices[1], &texture_indices[1], &normal_indices[1],
                &vertex_indices[2], &texture_indices[2], &normal_indices[2]
            );
            face_t face = {0};
            for (int i = 0; i < 3; i++) {
                face.indices[i] = vertex_indices[i] - 1;
                face.col_indices[i] = 0;
                face.tex_indices[i] = texture_indices[i] - 1;
                face.norm_indices[i] = normal_indices[i] - 1;
            }
            array_push(mesh->faces, face);
            mesh->nb_faces++;
        }
    }

    fclose(file);
    return true;
}

bool load_model(model_t *model, const char *obj_filename) {
    char path[128];
    snprintf(path, sizeof(path), "../../assets/%s", obj_filename);

    if (!load_mesh_obj_data(&model->mesh, path)) {
        printf("Unable to load the model %s\n", path);
        return false;
    }
    model->triangles_to_raster = (triangle_t *)malloc(2 * model->mesh.nb_faces * sizeof(triangle_t));
    return true;
}

bool load_texture(texture_t *texture, const char *tex_filename) {
    char path[128];
    snprintf(path, sizeof(path), "../../assets/%s", tex_filename);

    upng_t* png_image = upng_new_from_file(path);
    if (png_image != NULL) {
        upng_decode(png_image);
        if (upng_get_error(png_image) != UPNG_EOK) {
            printf("Unable to load the texture %s\n", path);
            return false;
        }
    }

    int texture_width = upng_get_width(png_image);
    int texture_height = upng_get_height(png_image);

    int t;

    texture->scale_x = -5;
    t = texture_width;
    while (t >>= 1)
        texture->scale_x++;

    texture->scale_y = -5;
    t = texture_height;
    while (t >>= 1) 
        texture->scale_y++;

    if (texture->scale_x < 0 || texture->scale_y < 0)
        return false;

    uint32_t* texture_buffer = (uint32_t *)upng_get_buffer(png_image);

    texture->addr = malloc(texture_width * texture_height * sizeof(uint16_t));
    uint16_t* tex = texture->addr;
    for (int t = 0; t < texture_height; ++t)
        for (int s = 0; s < texture_width; ++s) {

            uint8_t* tc = (uint8_t*)(&texture_buffer[(texture_width * t) + s]);
            uint16_t cr = tc[0] >> 4;
            uint16_t cg = tc[1] >> 4;
            uint16_t cb = tc[2] >> 4;
            uint16_t ca = tc[3] >> 4;
            *tex = (ca << 12) | (cr << 8) | (cg << 4) | cb;
            tex++;
        }

    upng_free(png_image);

    return true;
}
