// graphite.h
// Copyright (c) 2021-2024 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef GRAPHITE_H
#define GRAPHITE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    float u, v, w;
} vec2d;

typedef struct {
    float x, y, z, w;
} vec3d;

typedef struct {
    float m[4][4];
} mat4x4;

typedef struct {
    int indices[3];
    int tex_indices[3];
    int col_indices[3];
    int norm_indices[3];
} face_t;

typedef struct {
    vec3d p[3];
    vec2d t[3];
    vec3d c[3];
    vec3d n[3];
} triangle_t;

typedef struct {
    size_t nb_vertices;
    size_t nb_texcoords;
    size_t nb_colors;
    size_t nb_normals;
    size_t nb_faces;
    vec3d* vertices;
    vec2d* texcoords;
    vec3d* colors;
    vec3d* normals;
    face_t* faces;
} mesh_t;

typedef struct {
    mesh_t mesh;

    // Internal buffers
    triangle_t* triangles_to_raster;
} model_t;

typedef struct {
    size_t width, height;
    unsigned char* data;
} texture_t;

typedef struct {
    vec3d direction;
    vec3d ambient_color;
    vec3d diffuse_color;
} light_t;

vec3d matrix_multiply_vector(mat4x4* m, vec3d* i);
vec3d vector_add(vec3d* v1, vec3d* v2);
vec3d vector_sub(vec3d* v1, vec3d* v2);
vec3d vector_mul(vec3d* v1, float k);
vec3d vector_div(vec3d* v1, float k);
float vector_dot_product(vec3d* v1, vec3d* v2);
float vector_length(vec3d* v);
vec3d vector_normalize(vec3d* v);
vec3d vector_cross_product(vec3d* v1, vec3d* v2);
vec3d vector_clamp(vec3d* v);

mat4x4 matrix_make_identity();
mat4x4 matrix_make_projection(int viewport_width, int viewport_height, float fov);
mat4x4 matrix_make_rotation_x(float theta);
mat4x4 matrix_make_rotation_y(float theta);
mat4x4 matrix_make_rotation_z(float theta);
mat4x4 matrix_make_translation(float x, float y, float z);
mat4x4 matrix_make_scale(float x, float y, float z);
mat4x4 matrix_multiply_matrix(mat4x4* m1, mat4x4* m2);
mat4x4 matrix_point_at(vec3d* pos, vec3d* target, vec3d* up);
mat4x4 matrix_quick_inverse(mat4x4* m);

void draw_line(vec3d v0, vec3d v1, vec2d uv0, vec2d uv1, vec3d c0, vec3d c1, float thickness, texture_t* texture,
                bool clamp_s, bool clamp_t, int texture_scale_x, int texture_scale_y, bool perspective_correct);

void draw_model(int viewport_width, int viewport_height, vec3d* vec_camera, model_t* model, mat4x4* mat_world,
                mat4x4* mat_normal, mat4x4* mat_projection, mat4x4* mat_view, light_t* lights, size_t nb_lights, bool is_wireframe, texture_t* texture,
                bool clamp_s, bool clamp_t, int texture_scale_x, int texture_scale_y, bool perspective_correct);

#endif