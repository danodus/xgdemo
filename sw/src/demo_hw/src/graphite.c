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

#include "io.h"
#include "fat/fat_filelib.h"
#include "upng.h"

#define BASE_VIDEO 0x1000000

#define OP_SET_X0 0
#define OP_SET_Y0 1
#define OP_SET_Z0 2
#define OP_SET_X1 3
#define OP_SET_Y1 4
#define OP_SET_Z1 5
#define OP_SET_X2 6
#define OP_SET_Y2 7
#define OP_SET_Z2 8
#define OP_SET_R0 9
#define OP_SET_G0 10
#define OP_SET_B0 11
#define OP_SET_R1 12
#define OP_SET_G1 13
#define OP_SET_B1 14
#define OP_SET_R2 15
#define OP_SET_G2 16
#define OP_SET_B2 17
#define OP_SET_S0 18
#define OP_SET_T0 19
#define OP_SET_S1 20
#define OP_SET_T1 21
#define OP_SET_S2 22
#define OP_SET_T2 23
#define OP_CLEAR 24
#define OP_DRAW 25
#define OP_SWAP 26
#define OP_SET_TEX_ADDR 27
#define OP_SET_FB_ADDR 28

#define MEM_WRITE(_addr_, _value_) (*((volatile unsigned int *)(_addr_)) = _value_)
#define MEM_READ(_addr_) *((volatile unsigned int *)(_addr_))

#define _FLOAT_TO_FIXED(x, scale) ((int32_t)((x) * (float)(1 << scale)))
#define PARAM(x) (_FLOAT_TO_FIXED(x, 14))

struct Command {
    uint32_t opcode : 8;
    uint32_t param : 24;
};

static int g_fb_width, g_fb_height;
static uint32_t g_tex_addr;


static void send_command(struct Command *cmd) {
    while (!MEM_READ(GRAPHITE));
    MEM_WRITE(GRAPHITE, (cmd->opcode << 24) | cmd->param);
}

void draw_triangle(vec3d p[3], vec2d t[3], vec3d c[3], const texture_t* texture, bool clamp_s, bool clamp_t, bool depth_test, bool perspective_correct)
{
    struct Command cmd;

    cmd.opcode = OP_SET_X0;
    cmd.param = PARAM(p[0].x) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[0].x) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_Y0;
    cmd.param = PARAM(p[0].y) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[0].y) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_Z0;
    cmd.param = PARAM(t[0].w) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[0].w) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_X1;
    cmd.param = PARAM(p[1].x) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[1].x) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_Y1;
    cmd.param = PARAM(p[1].y) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[1].y) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_Z1;
    cmd.param = PARAM(t[1].w) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[1].w) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_X2;
    cmd.param = PARAM(p[2].x) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[2].x) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_Y2;
    cmd.param = PARAM(p[2].y) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[2].y) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_Z2;
    cmd.param = PARAM(t[2].w) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[2].z) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_S0;
    cmd.param = PARAM(t[0].u) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[0].u) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_T0;
    cmd.param = PARAM(t[0].v) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[0].v) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_S1;
    cmd.param = PARAM(t[1].u) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[1].u) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_T1;
    cmd.param = PARAM(t[1].v) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[1].v) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_S2;
    cmd.param = PARAM(t[2].u) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[2].u) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_T2;
    cmd.param = PARAM(t[2].v) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[2].v) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_R0;
    cmd.param = PARAM(c[0].x) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[0].x) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_G0;
    cmd.param = PARAM(c[0].y) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[0].y) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_B0;
    cmd.param = PARAM(c[0].z) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[0].z) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_R1;
    cmd.param = PARAM(c[1].x) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[1].x) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_G1;
    cmd.param = PARAM(c[1].y) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[1].y) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_B1;
    cmd.param = PARAM(c[1].z) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[1].z) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_R2;
    cmd.param = PARAM(c[2].x) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[2].x) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_G2;
    cmd.param = PARAM(c[2].y) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[2].y) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_B2;
    cmd.param = PARAM(c[2].z) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[2].z) >> 16);
    send_command(&cmd);

    if (texture != NULL) {
        cmd.opcode = OP_SET_TEX_ADDR;
        cmd.param = (uint32_t)texture->addr & 0xFFFF;
        send_command(&cmd);
        cmd.param = 0x10000 | ((uint32_t)texture->addr >> 16);
        send_command(&cmd);
    }

    cmd.opcode = OP_DRAW;

    cmd.param = (depth_test ? 0b01000 : 0b00000) | (clamp_s ? 0b00100 : 0b00000) | (clamp_t ? 0b00010 : 0b00000) |
              ((texture != NULL) ? 0b00001 : 0b00000) | (perspective_correct ? 0b10000 : 0xb00000);

    if (texture != NULL) {
        cmd.param |= texture->scale_x << 5;
        cmd.param |= texture->scale_y << 8;
    }

    send_command(&cmd);
}

void graphite_init(void) {
    unsigned int res = MEM_READ(CONFIG);
    g_fb_width = res >> 16;
    g_fb_height = res & 0xffff;
    g_tex_addr = (0x1000000 >> 1) + 3 * g_fb_width * g_fb_height;
}

void get_fb_dimensions(int* fb_width, int*fb_height) {
    *fb_width = g_fb_width;
    *fb_height = g_fb_height;
}

void clear(unsigned int color) {
    struct Command cmd;

    // Clear framebuffer
    cmd.opcode = OP_CLEAR;
    cmd.param = color;
    send_command(&cmd);
    // Clear depth buffer
    cmd.opcode = OP_CLEAR;
    cmd.param = 0x010000;
    send_command(&cmd);
}

void swap(void) {
    struct Command cmd;

    cmd.opcode = OP_SWAP;
    cmd.param = 0x1;
    send_command(&cmd);
}

static bool load_mesh_obj_data(mesh_t *mesh, const char *obj_filename) {
    FL_FILE* file;
    file = fl_fopen(obj_filename, "r");

    if (file == NULL)
        return false;

    memset(mesh, 0, sizeof(mesh_t));

    char line[1024];
    while (fl_fgets(line, 1024, file)) {
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

    fl_fclose(file);
    return true;
}

bool load_model(model_t *model, const char *obj_filename) {
    char path[128];
    snprintf(path, sizeof(path), "/assets/%s", obj_filename);

    if (!load_mesh_obj_data(&model->mesh, path))
        return false;
    model->triangles_to_raster = (triangle_t *)malloc(2 * model->mesh.nb_faces * sizeof(triangle_t));
    return true;
}

bool load_texture(texture_t *texture, const char *tex_filename) {
    char path[128];
    snprintf(path, sizeof(path), "/assets/%s", tex_filename);

    upng_t* png_image = upng_new_from_file(path);
    if (png_image != NULL) {
        upng_decode(png_image);
        if (upng_get_error(png_image) != UPNG_EOK) {
            return false;
        }
    }

    texture->addr = (uint16_t *)g_tex_addr;

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

    uint16_t* vram = 0;
    for (int t = 0; t < texture_height; ++t)
        for (int s = 0; s < texture_width; ++s) {

            uint8_t* tc = (uint8_t*)(&texture_buffer[(texture_width * t) + s]);
            uint16_t cr = tc[0] >> 4;
            uint16_t cg = tc[1] >> 4;
            uint16_t cb = tc[2] >> 4;
            uint16_t ca = tc[3] >> 4;
            vram[g_tex_addr++] = (ca << 12) | (cr << 8) | (cg << 4) | cb;
        }

    upng_free(png_image);

    return true;
}
