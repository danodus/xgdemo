#ifndef GLIB_H
#define GLIB_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

typedef struct {
    float u, v, w;
} vec2d;

typedef struct {
    float x, y, z, w;
} vec3d;

typedef struct {
    float m[4][4];
} mat4x4;

typedef vec3d quaternion;

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
    size_t scale_x, scale_y;
    uint16_t* addr;
} texture_t;

typedef struct {
    vec3d direction;
    vec3d ambient_color;
    vec3d diffuse_color;
} light_t;

vec3d matrix_multiply_vector(const mat4x4* m, const vec3d* i);
vec3d vector_add(const vec3d* v1, const vec3d* v2);
vec3d vector_sub(const vec3d* v1, const vec3d* v2);
vec3d vector_mul(const vec3d* v1, float k);
vec3d vector_div(const vec3d* v1, float k);
float vector_dot_product(const vec3d* v1, const vec3d* v2);
float vector_length(const vec3d* v);
vec3d vector_normalize(const vec3d* v);
vec3d vector_cross_product(const vec3d* v1, const vec3d* v2);
vec3d vector_clamp(const vec3d* v);

mat4x4 matrix_make_identity(void);
mat4x4 matrix_make_projection(int viewport_width, int viewport_height, float fov);
mat4x4 matrix_make_rotation_x(float theta);
mat4x4 matrix_make_rotation_y(float theta);
mat4x4 matrix_make_rotation_z(float theta);
mat4x4 matrix_make_translation(float x, float y, float z);
mat4x4 matrix_make_scale(float x, float y, float z);
mat4x4 matrix_multiply_matrix(mat4x4* m1, mat4x4* m2);
mat4x4 matrix_point_at(vec3d* pos, vec3d* target, vec3d* up);
mat4x4 matrix_quick_inverse(mat4x4* m);

quaternion quaternion_make_identity(void);
quaternion quaternion_from_euler(float pitch, float yaw, float roll);
mat4x4 quaternion_to_matrix(const quaternion* q);
quaternion quaternion_multiply(const quaternion* q1, const quaternion* q2);
quaternion quaternion_from_axis_angle(vec3d axis, float angle);
vec3d vector_rotate_by_quaternion(const vec3d* v, const quaternion* q);

void draw_model(int viewport_width, int viewport_height, const vec3d* vec_camera, const model_t* model, const mat4x4* mat_world,
                const mat4x4* mat_normal, const mat4x4* mat_projection, const mat4x4* mat_view, const light_t* lights, size_t nb_lights, const texture_t* texture,
                bool clamp_s, bool clamp_t, bool perspective_correct);

#endif
