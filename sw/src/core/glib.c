#include "glib.h"

#include <string.h>

#define MAX_NB_TRIANGLES    16      // maximum number of triangles produced by the clipping

void draw_triangle(vec3d p[3], vec2d t[3], vec3d c[3], const texture_t* texture, bool clamp_s, bool clamp_t, bool depth_test, bool perspective_correct);

vec3d matrix_multiply_vector(const mat4x4* m, const vec3d* i) {
    vec3d r = {i->x * m->m[0][0] + i->y * m->m[1][0] + i->z * m->m[2][0] + m->m[3][0],
               i->x * m->m[0][1] + i->y * m->m[1][1] + i->z * m->m[2][1] + m->m[3][1],
               i->x * m->m[0][2] + i->y * m->m[1][2] + i->z * m->m[2][2] + m->m[3][2],
               i->x * m->m[0][3] + i->y * m->m[1][3] + i->z * m->m[2][3] + m->m[3][3]};

    return r;
}

vec3d vector_add(const vec3d* v1, const vec3d* v2) {
    vec3d r = {v1->x + v2->x, v1->y + v2->y, v1->z + v2->z, 1.0f};
    return r;
}

vec3d vector_sub(const vec3d* v1, const vec3d* v2) {
    vec3d r = {v1->x - v2->x, v1->y - v2->y, v1->z - v2->z, 1.0f};
    return r;
}

vec3d vector_mul(const vec3d* v1, float k) {
    vec3d r = {v1->x * k, v1->y * k, v1->z * k, 1.0f};
    return r;
}

vec3d vector_div(const vec3d* v1, float k) {
    vec3d r = {v1->x / k, v1->y / k, v1->z / k, 1.0f};
    return r;
}

float vector_dot_product(const vec3d* v1, const vec3d* v2) {
    return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

float vector_length(const vec3d* v) {
    return sqrtf(vector_dot_product(v, v));
}

vec3d vector_normalize(const vec3d* v) {
    float l = vector_length(v);
    if (l > 0.0f) {
        vec3d r = {v->x / l, v->y / l, v->z / l, 1.0f};
        return r;
    } else {
        vec3d r = {0.0f, 0.0f, 0.0f, 1.0f};
        return r;
    }
}

vec3d vector_cross_product(const vec3d* v1, const vec3d* v2) {
    vec3d r = {v1->y * v2->z - v1->z * v2->y, v1->z * v2->x - v1->x * v2->z,
               v1->x * v2->y - v1->y * v2->x, 1.0f};
    return r;
}

vec3d vector_clamp(const vec3d* v) {
    vec3d r = *v;
    if (r.x > 1.0f)
        r.x = 1.0f;
    if (r.y > 1.0f)
        r.y = 1.0f;
    if (r.z > 1.0f)
        r.z = 1.0f;
    return r;
}

static vec3d vector_intersect_plane(const vec3d* plane_p, const vec3d* plane_n, const vec3d* line_start, const vec3d* line_end, float* t) {
    float plane_d = -vector_dot_product(plane_n, plane_p);
    float ad = vector_dot_product(line_start, plane_n);
    float bd = vector_dot_product(line_end, plane_n);
    *t = (-plane_d - ad) / (bd - ad);
    vec3d line_start_to_end = vector_sub(line_end, line_start);
    vec3d line_to_intersect = vector_mul(&line_start_to_end, *t);
    return vector_add(line_start, &line_to_intersect);
}

// return signed shortest distance from point to plane, plane normal must be normalized
static float dist_point_to_plane(const vec3d* plane_p, const vec3d* plane_n, const vec3d* p) {
    return (plane_n->x * p->x + plane_n->y * p->y + plane_n->z * p->z -
            vector_dot_product(plane_n, plane_p));
}

static int triangle_clip_against_plane(const vec3d* plane_p, const vec3d* plane_n, triangle_t* in_tri, triangle_t* out_tri1,
                                triangle_t* out_tri2) {
    // create two temporary storage arrays to classify points either side of the plane
    // if distance sign is positive, point lies on the inside of plane
    vec3d* inside_points[3];
    int nb_inside_points = 0;
    vec3d* outside_points[3];
    int nb_outside_points = 0;
    vec2d* inside_texcoords[3];
    int nb_inside_texcoords = 0;
    vec2d* outside_texcoords[3];
    int nb_outside_texcoords = 0;
    vec3d* inside_colors[3];
    int nb_inside_colors = 0;
    vec3d* outside_colors[3];
    int nb_outside_colors = 0;
    vec3d* inside_normals[3];
    int nb_inside_normals = 0;
    vec3d* outside_normals[3];
    int nb_outside_normals = 0;

    // get signed distance of each point in triangle to plane
    float d0 = dist_point_to_plane(plane_p, plane_n, &in_tri->p[0]);
    float d1 = dist_point_to_plane(plane_p, plane_n, &in_tri->p[1]);
    float d2 = dist_point_to_plane(plane_p, plane_n, &in_tri->p[2]);

    if (d0 >= 0.0f) {
        inside_points[nb_inside_points++] = &in_tri->p[0];
        inside_texcoords[nb_inside_texcoords++] = &in_tri->t[0];
        inside_colors[nb_inside_colors++] = &in_tri->c[0];
        inside_normals[nb_inside_normals++] = &in_tri->n[0];
    } else {
        outside_points[nb_outside_points++] = &in_tri->p[0];
        outside_texcoords[nb_outside_texcoords++] = &in_tri->t[0];
        outside_colors[nb_outside_colors++] = &in_tri->c[0];
        outside_normals[nb_outside_normals++] = &in_tri->n[0];
    }
    if (d1 >= 0.0f) {
        inside_points[nb_inside_points++] = &in_tri->p[1];
        inside_texcoords[nb_inside_texcoords++] = &in_tri->t[1];
        inside_colors[nb_inside_colors++] = &in_tri->c[1];
        inside_normals[nb_inside_normals++] = &in_tri->n[1];
    } else {
        outside_points[nb_outside_points++] = &in_tri->p[1];
        outside_texcoords[nb_outside_texcoords++] = &in_tri->t[1];
        outside_colors[nb_outside_colors++] = &in_tri->c[1];
        outside_normals[nb_outside_normals++] = &in_tri->n[1];
    }
    if (d2 >= 0.0f) {
        inside_points[nb_inside_points++] = &in_tri->p[2];
        inside_texcoords[nb_inside_texcoords++] = &in_tri->t[2];
        inside_colors[nb_inside_colors++] = &in_tri->c[2];
        inside_normals[nb_inside_normals++] = &in_tri->n[2];
    } else {
        outside_points[nb_outside_points++] = &in_tri->p[2];
        outside_texcoords[nb_outside_texcoords] = &in_tri->t[2];
        outside_colors[nb_outside_colors++] = &in_tri->c[2];
        outside_normals[nb_outside_normals++] = &in_tri->n[2];
    }

    // classify triangle points and break the input triangle into smaller output triangles if required

    if (nb_inside_points == 0) {
        // all points lie on the outside of the plane, so clip whole triangle
        return 0;  // no returned triangles are valid
    }

    if (nb_inside_points == 3) {
        // all points lie in the inside of plane, so do nothing and allow the triangle to simply pass through
        *out_tri1 = *in_tri;

        return 1;  // just the one returned original triangle is valid
    }

    if (nb_inside_points == 1 && nb_outside_points == 2) {
        // Triangle should be clipped. As two points lie outside the plane, the triangle simply becomes a smaller
        // triangle.

        // the inside point is valid, so keep that
        out_tri1->p[0] = *inside_points[0];
        out_tri1->t[0] = *inside_texcoords[0];
        out_tri1->c[0] = *inside_colors[0];
        out_tri1->n[0] = *inside_normals[0];

        // but the two new points are at the location where the original sides of the triangle (lines) intersect with
        // the plane
        float t;
        out_tri1->p[1] = vector_intersect_plane(plane_p, plane_n, inside_points[0], outside_points[0], &t);
        out_tri1->t[1].u = t * (outside_texcoords[0]->u - inside_texcoords[0]->u) + inside_texcoords[0]->u;
        out_tri1->t[1].v = t * (outside_texcoords[0]->v - inside_texcoords[0]->v) + inside_texcoords[0]->v;
        out_tri1->t[1].w = t * (outside_texcoords[0]->w - inside_texcoords[0]->w) + inside_texcoords[0]->w;
        out_tri1->c[1].x = t * (outside_colors[0]->x - inside_colors[0]->x) + inside_colors[0]->x;
        out_tri1->c[1].y = t * (outside_colors[0]->y - inside_colors[0]->y) + inside_colors[0]->y;
        out_tri1->c[1].z = t * (outside_colors[0]->z - inside_colors[0]->z) + inside_colors[0]->z;
        out_tri1->c[1].w = t * (outside_colors[0]->w - inside_colors[0]->w) + inside_colors[0]->w;
        out_tri1->n[1].x = t * (outside_normals[0]->x - inside_normals[0]->x) + inside_normals[0]->x;
        out_tri1->n[1].y = t * (outside_normals[0]->y - inside_normals[0]->y) + inside_normals[0]->y;
        out_tri1->n[1].z = t * (outside_normals[0]->z - inside_normals[0]->z) + inside_normals[0]->z;
        out_tri1->n[1].w = t * (outside_normals[0]->w - inside_normals[0]->w) + inside_normals[0]->w;

        out_tri1->p[2] = vector_intersect_plane(plane_p, plane_n, inside_points[0], outside_points[1], &t);
        out_tri1->t[2].u = t * (outside_texcoords[1]->u - inside_texcoords[0]->u) + inside_texcoords[0]->u;
        out_tri1->t[2].v = t * (outside_texcoords[1]->v - inside_texcoords[0]->v) + inside_texcoords[0]->v;
        out_tri1->t[2].w = t * (outside_texcoords[1]->w - inside_texcoords[0]->w) + inside_texcoords[0]->w;
        out_tri1->c[2].x = t * (outside_colors[1]->x - inside_colors[0]->x) + inside_colors[0]->x;
        out_tri1->c[2].y = t * (outside_colors[1]->y - inside_colors[0]->y) + inside_colors[0]->y;
        out_tri1->c[2].z = t * (outside_colors[1]->z - inside_colors[0]->z) + inside_colors[0]->z;
        out_tri1->c[2].w = t * (outside_colors[1]->w - inside_colors[0]->w) + inside_colors[0]->w;
        out_tri1->n[2].x = t * (outside_normals[1]->x - inside_normals[0]->x) + inside_normals[0]->x;
        out_tri1->n[2].y = t * (outside_normals[1]->y - inside_normals[0]->y) + inside_normals[0]->y;
        out_tri1->n[2].z = t * (outside_normals[1]->z - inside_normals[0]->z) + inside_normals[0]->z;
        out_tri1->n[2].w = t * (outside_normals[1]->w - inside_normals[0]->w) + inside_normals[0]->w;

        return 1;  // return the newly formed single triangle
    }

    if (nb_inside_points == 2 && nb_outside_points == 1) {
        // Triangle should be clipped. As two points lie inside the plane, the clipped triangle becomes a "quad".
        // Fortunately, we can represent a quad with two new triangles.

        float t;

        // The first triangle consists of the two inside points and a new point determined by the location where one
        // side of the triangle intersects with the plane
        out_tri1->p[0] = *inside_points[0];
        out_tri1->t[0] = *inside_texcoords[0];
        out_tri1->c[0] = *inside_colors[0];
        out_tri1->p[1] = *inside_points[1];
        out_tri1->t[1] = *inside_texcoords[1];
        out_tri1->c[1] = *inside_colors[1];
        out_tri1->p[2] = vector_intersect_plane(plane_p, plane_n, inside_points[0], outside_points[0], &t);
        out_tri1->t[2].u = t * (outside_texcoords[0]->u - inside_texcoords[0]->u) + inside_texcoords[0]->u;
        out_tri1->t[2].v = t * (outside_texcoords[0]->v - inside_texcoords[0]->v) + inside_texcoords[0]->v;
        out_tri1->t[2].w = t * (outside_texcoords[0]->w - inside_texcoords[0]->w) + inside_texcoords[0]->w;
        out_tri1->c[2].x = t * (outside_colors[0]->x - inside_colors[0]->x) + inside_colors[0]->x;
        out_tri1->c[2].y = t * (outside_colors[0]->y - inside_colors[0]->y) + inside_colors[0]->y;
        out_tri1->c[2].z = t * (outside_colors[0]->z - inside_colors[0]->z) + inside_colors[0]->z;
        out_tri1->c[2].w = t * (outside_colors[0]->w - inside_colors[0]->w) + inside_colors[0]->w;
        out_tri1->n[2].x = t * (outside_normals[0]->x - inside_normals[0]->x) + inside_normals[0]->x;
        out_tri1->n[2].y = t * (outside_normals[0]->y - inside_normals[0]->y) + inside_normals[0]->y;
        out_tri1->n[2].z = t * (outside_normals[0]->z - inside_normals[0]->z) + inside_normals[0]->z;
        out_tri1->n[2].w = t * (outside_normals[0]->w - inside_normals[0]->w) + inside_normals[0]->w;

        // The second triangle is composed of one the the inside points, a new point determined by the intersection
        // of the other side of the triangle and the plane, and the newly created point above
        out_tri2->p[0] = *inside_points[1];
        out_tri2->t[0] = *inside_texcoords[1];
        out_tri2->c[0] = *inside_colors[1];
        out_tri2->p[1] = vector_intersect_plane(plane_p, plane_n, inside_points[1], outside_points[0], &t);
        out_tri2->t[1].u = t * (outside_texcoords[0]->u - inside_texcoords[1]->u) + inside_texcoords[1]->u;
        out_tri2->t[1].v = t * (outside_texcoords[0]->v - inside_texcoords[1]->v) + inside_texcoords[1]->v;
        out_tri2->t[1].w = t * (outside_texcoords[0]->w - inside_texcoords[1]->w) + inside_texcoords[1]->w;
        out_tri2->c[1].x = t * (outside_colors[0]->x - inside_colors[1]->x) + inside_colors[1]->x;
        out_tri2->c[1].y = t * (outside_colors[0]->y - inside_colors[1]->y) + inside_colors[1]->y;
        out_tri2->c[1].z = t * (outside_colors[0]->z - inside_colors[1]->z) + inside_colors[1]->z;
        out_tri2->c[1].w = t * (outside_colors[0]->w - inside_colors[1]->w) + inside_colors[1]->w;
        out_tri2->n[1].x = t * (outside_normals[0]->x - inside_normals[1]->x) + inside_normals[1]->x;
        out_tri2->n[1].y = t * (outside_normals[0]->y - inside_normals[1]->y) + inside_normals[1]->y;
        out_tri2->n[1].z = t * (outside_normals[0]->z - inside_normals[1]->z) + inside_normals[1]->z;
        out_tri2->n[1].w = t * (outside_normals[0]->w - inside_normals[1]->w) + inside_normals[1]->w;
        out_tri2->p[2] = out_tri1->p[2];
        out_tri2->t[2] = out_tri1->t[2];
        out_tri2->c[2] = out_tri1->c[2];
        out_tri2->n[2] = out_tri1->n[2];

        return 2;  // return two newly formed triangles which form a quad
    }

    return 0;  // should not happen
}

mat4x4 matrix_make_identity(void) {
    mat4x4 mat;
    memset(&mat, 0, sizeof(mat4x4));
    mat.m[0][0] = 1.0f;
    mat.m[1][1] = 1.0f;
    mat.m[2][2] = 1.0f;
    mat.m[3][3] = 1.0f;
    return mat;
}

mat4x4 matrix_make_projection(int viewport_width, int viewport_height, float fov) {
    mat4x4 mat_proj;

    // projection matrix
    float near = 0.3f;
    float far = 1000.0f;
    float aspect_ratio = (float)viewport_height / (float)viewport_width;
    float fov_rad = 1.0f / tanf(fov * 0.5f / 180.0f * 3.14159f);

    memset(&mat_proj, 0, sizeof(mat4x4));
    mat_proj.m[0][0] = aspect_ratio * fov_rad;
    mat_proj.m[1][1] = fov_rad;
    mat_proj.m[2][2] = far / (far - near);
    mat_proj.m[3][2] = (-far * near) / (far - near);
    mat_proj.m[2][3] = 1.0f;
    mat_proj.m[3][3] = 0.0f;

    return mat_proj;
}

mat4x4 matrix_make_rotation_x(float theta) {
    mat4x4 mat_rot_x;
    memset(&mat_rot_x, 0, sizeof(mat4x4));

    // rotation X
    mat_rot_x.m[0][0] = 1.0f;
    mat_rot_x.m[1][1] = cosf(theta);
    mat_rot_x.m[1][2] = sinf(theta);
    mat_rot_x.m[2][1] = -sinf(theta);
    mat_rot_x.m[2][2] = cosf(theta);
    mat_rot_x.m[3][3] = 1.0f;

    return mat_rot_x;
}

mat4x4 matrix_make_rotation_y(float theta) {
    mat4x4 mat_rot_y;
    memset(&mat_rot_y, 0, sizeof(mat4x4));

    // rotation Y
    mat_rot_y.m[0][0] = cos(theta);
    mat_rot_y.m[0][2] = -sinf(theta);
    mat_rot_y.m[1][1] = 1.0f;
    mat_rot_y.m[2][0] = sinf(theta);
    mat_rot_y.m[2][2] = cosf(theta);
    mat_rot_y.m[3][3] = 1.0f;

    return mat_rot_y;
}

mat4x4 matrix_make_rotation_z(float theta) {
    mat4x4 mat_rot_z;
    memset(&mat_rot_z, 0, sizeof(mat4x4));

    // rotation Z
    mat_rot_z.m[0][0] = cosf(theta);
    mat_rot_z.m[0][1] = sinf(theta);
    mat_rot_z.m[1][0] = -sinf(theta);
    mat_rot_z.m[1][1] = cosf(theta);
    mat_rot_z.m[2][2] = 1.0f;
    mat_rot_z.m[3][3] = 1.0f;

    return mat_rot_z;
}

mat4x4 matrix_make_translation(float x, float y, float z) {
    mat4x4 mat;
    memset(&mat, 0, sizeof(mat4x4));

    mat.m[0][0] = 1.0f;
    mat.m[1][1] = 1.0f;
    mat.m[2][2] = 1.0f;
    mat.m[3][3] = 1.0f;
    mat.m[3][0] = x;
    mat.m[3][1] = y;
    mat.m[3][2] = z;

    return mat;
}

mat4x4 matrix_make_scale(float x, float y, float z) {
    mat4x4 mat;
    memset(&mat, 0, sizeof(mat4x4));

    mat.m[0][0] = x;
    mat.m[1][1] = y;
    mat.m[2][2] = z;
    mat.m[3][3] = 1.0f;

    return mat;
}

mat4x4 matrix_multiply_matrix(mat4x4* m1, mat4x4* m2) {
    mat4x4 mat;
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            mat.m[c][r] = m1->m[c][0] * m2->m[0][r] + m1->m[c][1] * m2->m[1][r] +
                          m1->m[c][2] * m2->m[2][r] + m1->m[c][3] * m2->m[3][r];
    return mat;
}

mat4x4 matrix_point_at(vec3d* pos, vec3d* target, vec3d* up) {
    // calculate new forward direction
    vec3d new_forward = vector_sub(target, pos);
    new_forward = vector_normalize(&new_forward);

    // calculate new up direction
    float dp = vector_dot_product(up, &new_forward);
    vec3d a = vector_mul(&new_forward, dp);
    vec3d new_up = vector_sub(up, &a);
    new_up = vector_normalize(&new_up);

    // new right direction is easy, just cross product
    vec3d new_right = vector_cross_product(&new_up, &new_forward);

    // construct dimensioning and translation matrix
    mat4x4 mat;
    mat.m[0][0] = new_right.x;
    mat.m[0][1] = new_right.y;
    mat.m[0][2] = new_right.z;
    mat.m[0][3] = 0.0f;
    mat.m[1][0] = new_up.x;
    mat.m[1][1] = new_up.y;
    mat.m[1][2] = new_up.z;
    mat.m[1][3] = 0.0f;
    mat.m[2][0] = new_forward.x;
    mat.m[2][1] = new_forward.y;
    mat.m[2][2] = new_forward.z;
    mat.m[2][3] = 0.0f;
    mat.m[3][0] = pos->x;
    mat.m[3][1] = pos->y;
    mat.m[3][2] = pos->z;
    mat.m[3][3] = 1.0f;
    return mat;
}

mat4x4 matrix_quick_inverse(mat4x4* m) {
    mat4x4 mat;
    mat.m[0][0] = m->m[0][0];
    mat.m[0][1] = m->m[1][0];
    mat.m[0][2] = m->m[2][0];
    mat.m[0][3] = 0.0f;
    mat.m[1][0] = m->m[0][1];
    mat.m[1][1] = m->m[1][1];
    mat.m[1][2] = m->m[2][1];
    mat.m[1][3] = 0.0f;
    mat.m[2][0] = m->m[0][2];
    mat.m[2][1] = m->m[1][2];
    mat.m[2][2] = m->m[2][2];
    mat.m[2][3] = 0.0f;
    mat.m[3][0] = -(m->m[3][0] * mat.m[0][0] + m->m[3][1] * mat.m[1][0] + m->m[3][2] * mat.m[2][0]);
    mat.m[3][1] = -(m->m[3][0] * mat.m[0][1] + m->m[3][1] * mat.m[1][1] + m->m[3][2] * mat.m[2][1]);
    mat.m[3][2] = -(m->m[3][0] * mat.m[0][2] + m->m[3][1] * mat.m[1][2] + m->m[3][2] * mat.m[2][2]);
    mat.m[3][3] = 1.0f;
    return mat;
}

quaternion quaternion_make_identity(void) {
    quaternion quat = { 0.0f, 0.0f, 0.0f, 1.0f };
    return quat;
}

quaternion quaternion_from_euler(float pitch, float yaw, float roll) {
    quaternion quat;

    float x0 = cosf(pitch * 0.5f);
    float x1 = sinf(pitch * 0.5f);
    float y0 = cosf(yaw * 0.5f);
    float y1 = sinf(yaw * 0.5f);
    float z0 = cosf(roll * 0.5f);
    float z1 = sinf(roll * 0.5f);

    quat.x = x1 * y0 * z0 - x0 * y1 * z1;
    quat.y = x0 * y1 * z0 + x1 * y0 * z1;
    quat.z = x0 * y0 * z1 - x1 * y1 * z0;
    quat.w = x0 * y0 * z0 + x1 * y1 * z1;

    return quat;
}

mat4x4 quaternion_to_matrix(const quaternion* q) {
    mat4x4 mat = matrix_make_identity();

    float a2 = q->x * q->x;
    float b2 = q->y * q->y;
    float c2 = q->z * q->z;
    float ac = q->x * q->z;
    float ab = q->x * q->y;
    float bc = q->y * q->z;
    float ad = q->w * q->x;
    float bd = q->w * q->y;
    float cd = q->w * q->z;

    mat.m[0][0] = 1 - 2 * (b2 + c2);
    mat.m[0][1] = 2 * (ab + cd);
    mat.m[0][2] = 2 * (ac - bd);

    mat.m[1][0] = 2 * (ab - cd);
    mat.m[1][1] = 1 - 2 * (a2 + c2);
    mat.m[1][2] = 2 * (bc + ad);

    mat.m[2][0] = 2 * (ac + bd);
    mat.m[2][1] = 2 * (bc - ad);
    mat.m[2][2] = 1 - 2 * (a2 + b2);

    return mat;
}

quaternion quaternion_multiply(const quaternion* q1, const quaternion* q2) {
    quaternion quat = {0};

    float qax = q1->x, qay = q1->y, qaz = q1->z, qaw = q1->w;
    float qbx = q2->x, qby = q2->y, qbz = q2->z, qbw = q2->w;

    quat.x = qax * qbw + qaw * qbx + qay * qbz - qaz * qby;
    quat.y = qay * qbw + qaw * qby + qaz * qbx - qax * qbz;
    quat.z = qaz * qbw + qaw * qbz + qax * qby - qay * qbx;
    quat.w = qaw * qbw - qax * qbx - qay * qby - qaz * qbz;

    return quat;    
}

quaternion quaternion_from_axis_angle(vec3d axis, float angle)
{
    quaternion quat = {0.0f, 0.0f, 0.0f, 1.0f};

    float axisLength = sqrtf(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);

    if (axisLength != 0.0f)
    {
        angle *= 0.5f;

        float length = 0.0f;
        float ilength = 0.0f;

        // Normalize
        length = axisLength;
        if (length == 0.0f) length = 1.0f;
        ilength = 1.0f / length;
        axis.x *= ilength;
        axis.y *= ilength;
        axis.z *= ilength;

        float sinres = sinf(angle);
        float cosres = cosf(angle);

        quat.x = axis.x * sinres;
        quat.y = axis.y * sinres;
        quat.z = axis.z * sinres;
        quat.w = cosres;

        // Normalize
        quaternion q = quat;
        length = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
        if (length == 0.0f) length = 1.0f;
        ilength = 1.0f / length;
        quat.x = q.x * ilength;
        quat.y = q.y * ilength;
        quat.z = q.z * ilength;
        quat.w = q.w * ilength;
    }

    return quat;
}

vec3d vector_rotate_by_quaternion(const vec3d* v, const quaternion* q) {
    vec3d vec = {0};

    vec.x = v->x * (q->x * q->x + q->w * q->w - q->y * q->y - q->z * q->z) + v->y * (2 * q->x * q->y - 2 * q->w * q->z) + v->z * (2 * q->x * q->z + 2 * q->w * q->y);
    vec.y = v->x * (2 * q->w * q->z + 2 * q->x * q->y) + v->y * (q->w * q->w - q->x * q->x + q->y * q->y - q->z * q->z) + v->z * (-2 * q->w * q->x + 2 * q->y * q->z);
    vec.z = v->x * (-2 * q->w * q->y + 2 * q->x * q->z) + v->y * (2 * q->w * q->x + 2 * q->y * q->z)+ v->z * (q->w * q->w - q->x * q->x - q->y * q->y + q->z * q->z);

    return vec;
}

static float clamp(float x) {
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

void draw_model(int viewport_width, int viewport_height, const vec3d* vec_camera, const model_t* model, const mat4x4* mat_world,
    const mat4x4* mat_normal, const mat4x4* mat_projection, const mat4x4* mat_view, const light_t* lights, size_t nb_lights, const texture_t* texture,
    bool clamp_s, bool clamp_t, bool perspective_correct) {
    size_t triangle_to_raster_index = 0;

    // draw faces
    for (size_t i = 0; i < model->mesh.nb_faces; ++i) {
        face_t* face = &model->mesh.faces[i];
        triangle_t tri;
        tri.p[0] = model->mesh.vertices[face->indices[0]];
        tri.p[1] = model->mesh.vertices[face->indices[1]];
        tri.p[2] = model->mesh.vertices[face->indices[2]];
        if (model->mesh.nb_texcoords > 0) {
            tri.t[0] = model->mesh.texcoords[face->tex_indices[0]];
            tri.t[1] = model->mesh.texcoords[face->tex_indices[1]];
            tri.t[2] = model->mesh.texcoords[face->tex_indices[2]];
        } else {
            tri.t[0] = (vec2d){0.0f, 0.0f};
            tri.t[1] = (vec2d){0.0f, 0.0f};
            tri.t[2] = (vec2d){0.0f, 0.0f};
        }
        if (model->mesh.nb_colors > 0) {
            tri.c[0] = model->mesh.colors[face->col_indices[0]];
            tri.c[1] = model->mesh.colors[face->col_indices[1]];
            tri.c[2] = model->mesh.colors[face->col_indices[2]];
        } else {
            tri.c[0] = (vec3d){1.0f, 1.0f, 1.0f, 1.0f};
            tri.c[1] = (vec3d){1.0f, 1.0f, 1.0f, 1.0f};
            tri.c[2] = (vec3d){1.0f, 1.0f, 1.0f, 1.0f};
        }
        if (model->mesh.nb_normals > 0) {
            tri.n[0] = model->mesh.normals[face->norm_indices[0]];
            tri.n[1] = model->mesh.normals[face->norm_indices[1]];
            tri.n[2] = model->mesh.normals[face->norm_indices[2]];
        } else {
            tri.n[0] = (vec3d){0.0f, 0.0f, 0.0f, 0.0f};
            tri.n[1] = (vec3d){0.0f, 0.0f, 0.0f, 0.0f};
            tri.n[2] = (vec3d){0.0f, 0.0f, 0.0f, 0.0f};
        }

        triangle_t tri_viewed, tri_projected, tri_transformed;

        tri_transformed.p[0] = matrix_multiply_vector(mat_world, &tri.p[0]);
        tri_transformed.p[1] = matrix_multiply_vector(mat_world, &tri.p[1]);
        tri_transformed.p[2] = matrix_multiply_vector(mat_world, &tri.p[2]);
        tri_transformed.t[0] = tri.t[0];
        tri_transformed.t[1] = tri.t[1];
        tri_transformed.t[2] = tri.t[2];
        tri_transformed.c[0] = tri.c[0];
        tri_transformed.c[1] = tri.c[1];
        tri_transformed.c[2] = tri.c[2];
        if (mat_normal != NULL) {
            tri_transformed.n[0] = matrix_multiply_vector(mat_normal, &tri.n[0]);
            tri_transformed.n[1] = matrix_multiply_vector(mat_normal, &tri.n[1]);
            tri_transformed.n[2] = matrix_multiply_vector(mat_normal, &tri.n[2]);
        }

        // calculate the normal
        vec3d normal, line1, line2;
        line1.x = tri_transformed.p[1].x - tri_transformed.p[0].x;
        line1.y = tri_transformed.p[1].y - tri_transformed.p[0].y;
        line1.z = tri_transformed.p[1].z - tri_transformed.p[0].z;

        line2.x = tri_transformed.p[2].x - tri_transformed.p[0].x;
        line2.y = tri_transformed.p[2].y - tri_transformed.p[0].y;
        line2.z = tri_transformed.p[2].z - tri_transformed.p[0].z;

        // take the cross product of lines to get normal to triangle surface
        normal = vector_cross_product(&line1, &line2);

        // get ray from triangle to camera
        vec3d vec_camera_ray = vector_sub(&tri_transformed.p[0], vec_camera);

        // if ray is aligned with normal, then triangle is visible
        if (vector_dot_product(&normal, &vec_camera_ray) < 0.0f) {
    
            // illumination
            vec3d color[3] = {
                {0.0f, 0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, 0.0f, 1.0f}
            };

            for (size_t light_index = 0; light_index < nb_lights; ++light_index) {
                vec3d light_direction = lights[light_index].direction;
                float diffuse_intensity[3];

                if ((model->mesh.nb_normals > 0) && (mat_normal != NULL)) {

                    //
                    // Gouraud shading
                    //

                    for (int j = 0; j < 3; ++j) {
                        vec3d n = tri_transformed.n[j];
                        float dp = -vector_dot_product(&light_direction, &n);
                        if (dp < 0.0f) dp = 0.0f;
                        diffuse_intensity[j] = dp;
                    }

                } else {

                    //
                    // Flat shading
                    //

                    // how "aligned" are light direction and triangle surface normal?
                    vec3d n = normal;
                    n = vector_normalize(&n);
                    float dp = -vector_dot_product(&light_direction, &n);

                    if (dp < 0.0f) dp = 0.0f;

                    // note: alpha is currently forced to 1 here
                    diffuse_intensity[0] = dp;
                    diffuse_intensity[1] = dp;
                    diffuse_intensity[2] = dp;
                }

                // apply light color
                vec3d ambient_color = lights[light_index].ambient_color;
                for (int j = 0; j < 3; ++j) {
                    vec3d diffuse_color = lights[light_index].diffuse_color;
                    diffuse_color = vector_mul(&diffuse_color, diffuse_intensity[j]);
                    color[j] = vector_add(&color[j], &ambient_color);
                    color[j] = vector_add(&color[j], &diffuse_color);
                }
            } // for each light

            if (nb_lights > 0) {
                for (int j = 0; j < 3; ++j) {
                    color[j] = vector_clamp(&color[j]);
                    tri_transformed.c[j].x = tri_transformed.c[j].x * color[j].x;
                    tri_transformed.c[j].y = tri_transformed.c[j].y * color[j].y;
                    tri_transformed.c[j].z = tri_transformed.c[j].z * color[j].z;
                }
            }

            // convert world space to view space
            tri_viewed.p[0] = matrix_multiply_vector(mat_view, &tri_transformed.p[0]);
            tri_viewed.p[1] = matrix_multiply_vector(mat_view, &tri_transformed.p[1]);
            tri_viewed.p[2] = matrix_multiply_vector(mat_view, &tri_transformed.p[2]);
            tri_viewed.t[0] = tri_transformed.t[0];
            tri_viewed.t[1] = tri_transformed.t[1];
            tri_viewed.t[2] = tri_transformed.t[2];
            tri_viewed.c[0] = tri_transformed.c[0];
            tri_viewed.c[1] = tri_transformed.c[1];
            tri_viewed.c[2] = tri_transformed.c[2];

            // clip viewed triangle against near plane, this could form two additional triangles
            int nb_clipped_triangles = 0;
            triangle_t clipped[2];
            const float z_near = 0.3f;
            vec3d plane_p = {0.0f, 0.0f, z_near, 1.0f};
            vec3d plane_n = {0.0f, 0.0f, 1.0f, 1.0f};
            nb_clipped_triangles = triangle_clip_against_plane(&plane_p, &plane_n, &tri_viewed, &clipped[0], &clipped[1]);

            for (int n = 0; n < nb_clipped_triangles; ++n) {
                // project triangles from 3D to 2D
                tri_projected.p[0] = matrix_multiply_vector(mat_projection, &clipped[n].p[0]);
                tri_projected.p[1] = matrix_multiply_vector(mat_projection, &clipped[n].p[1]);
                tri_projected.p[2] = matrix_multiply_vector(mat_projection, &clipped[n].p[2]);
                tri_projected.t[0] = clipped[n].t[0];
                tri_projected.t[1] = clipped[n].t[1];
                tri_projected.t[2] = clipped[n].t[2];
                tri_projected.c[0] = clipped[n].c[0];
                tri_projected.c[1] = clipped[n].c[1];
                tri_projected.c[2] = clipped[n].c[2];

                float recip_w[3] = {
                    1.0f / tri_projected.p[0].w,
                    1.0f / tri_projected.p[1].w,
                    1.0f / tri_projected.p[2].w
                };

                if (perspective_correct) {
                    tri_projected.t[0].u = tri_projected.t[0].u * recip_w[0];
                    tri_projected.t[1].u = tri_projected.t[1].u * recip_w[1];
                    tri_projected.t[2].u = tri_projected.t[2].u * recip_w[2];

                    tri_projected.t[0].v = tri_projected.t[0].v * recip_w[0];
                    tri_projected.t[1].v = tri_projected.t[1].v * recip_w[1];
                    tri_projected.t[2].v = tri_projected.t[2].v * recip_w[2];

                    tri_projected.c[0].x = tri_projected.c[0].x * recip_w[0];
                    tri_projected.c[1].x = tri_projected.c[1].x * recip_w[1];
                    tri_projected.c[2].x = tri_projected.c[2].x * recip_w[2];

                    tri_projected.c[0].y = tri_projected.c[0].y * recip_w[0];
                    tri_projected.c[1].y = tri_projected.c[1].y * recip_w[1];
                    tri_projected.c[2].y = tri_projected.c[2].y * recip_w[2];

                    tri_projected.c[0].z = tri_projected.c[0].z * recip_w[0];
                    tri_projected.c[1].z = tri_projected.c[1].z * recip_w[1];
                    tri_projected.c[2].z = tri_projected.c[2].z * recip_w[2];

                    tri_projected.c[0].w = tri_projected.c[0].w * recip_w[0];
                    tri_projected.c[1].w = tri_projected.c[1].w * recip_w[1];
                    tri_projected.c[2].w = tri_projected.c[2].w * recip_w[2];
                }

                tri_projected.t[0].w = recip_w[0];
                tri_projected.t[1].w = recip_w[1];
                tri_projected.t[2].w = recip_w[2];

                // scale into view
                tri_projected.p[0] = vector_mul(&tri_projected.p[0], recip_w[0]);
                tri_projected.p[1] = vector_mul(&tri_projected.p[1], recip_w[1]);
                tri_projected.p[2] = vector_mul(&tri_projected.p[2], recip_w[2]);

                // Invert the y values to account for flipped screen y coordinate
                tri_projected.p[0].y = -tri_projected.p[0].y;
                tri_projected.p[1].y = -tri_projected.p[1].y;
                tri_projected.p[2].y = -tri_projected.p[2].y;

                // offset vertices into visible normalized space
                vec3d vec_offset_view = {1.0f, 1.0f, 0.0f, 1.0f};
                tri_projected.p[0] = vector_add(&tri_projected.p[0], &vec_offset_view);
                tri_projected.p[1] = vector_add(&tri_projected.p[1], &vec_offset_view);
                tri_projected.p[2] = vector_add(&tri_projected.p[2], &vec_offset_view);

                float w = (float)viewport_width / 2.0f;
                float h = (float)viewport_height / 2.0f;
                tri_projected.p[0].x = tri_projected.p[0].x * w;
                tri_projected.p[0].y = tri_projected.p[0].y * h;
                tri_projected.p[1].x = tri_projected.p[1].x * w;
                tri_projected.p[1].y = tri_projected.p[1].y * h;
                tri_projected.p[2].x = tri_projected.p[2].x * w;
                tri_projected.p[2].y = tri_projected.p[2].y * h;

                // store triangle for sorting
                model->triangles_to_raster[triangle_to_raster_index] = tri_projected;
                triangle_to_raster_index++;
            }
        }
    }

    for (size_t i = 0; i < triangle_to_raster_index; ++i) {
        triangle_t tri_to_raster = model->triangles_to_raster[triangle_to_raster_index - i - 1];

        // clip triangles against all four screen edges, this could yield a bunch of triangles
        triangle_t clipped[2];
        triangle_t triangles[MAX_NB_TRIANGLES];
        int nb_triangles = 0;
        triangles[nb_triangles++] = tri_to_raster;

        int nb_new_triangles = 1;

        for (int p = 0; p < 4; ++p) {
            int nb_tris_to_add = 0;
            while (nb_new_triangles > 0) {
                // pop front
                triangle_t test = triangles[0];
                for (int j = 1; j < nb_triangles; ++j) triangles[j - 1] = triangles[j];
                nb_triangles--;

                nb_new_triangles--;

                switch (p) {
                    case 0: {
                        vec3d p = {0.0f, 0.0f, 0.0f, 1.0f};
                        vec3d n = {0.0f, 1.0f, 0.0f, 1.0f};
                        nb_tris_to_add = triangle_clip_against_plane(&p, &n, &test, &clipped[0], &clipped[1]);
                        break;
                    }
                    case 1: {
                        vec3d p = {0.0f, (float)(viewport_height - 1), 0.0f, 1.0f};
                        vec3d n = {0.0f, -1.0f, 0.0f, 1.0f};
                        nb_tris_to_add = triangle_clip_against_plane(&p, &n, &test, &clipped[0], &clipped[1]);
                        break;
                    }
                    case 2: {
                        vec3d p = {0.0f, 0.0f, 0.0f, 1.0f};
                        vec3d n = {1.0f, 0.0f, 0.0f, 1.0f};
                        nb_tris_to_add = triangle_clip_against_plane(&p, &n, &test, &clipped[0], &clipped[1]);
                        break;
                    }
                    case 3: {
                        vec3d p = {(float)(viewport_width - 1), 0.0f, 0.0f, 1.0f};
                        vec3d n = {-1.0f, 0.0f, 0.0f, 1.0f};
                        nb_tris_to_add = triangle_clip_against_plane(&p, &n, &test, &clipped[0], &clipped[1]);
                        break;
                    }
                }

                // push back
                if (nb_triangles + nb_tris_to_add > MAX_NB_TRIANGLES) break;    // safety net
                for (int w = 0; w < nb_tris_to_add; ++w) triangles[nb_triangles++] = clipped[w];
            }
            nb_new_triangles = nb_triangles;
        }

        for (int i = 0; i < nb_triangles; ++i) {
            triangle_t* t = &triangles[i];

            // calculate the normal
            vec3d normal, line1, line2;
            line1.x = t->p[1].x - t->p[0].x;
            line1.y = t->p[1].y - t->p[0].y;
            line1.z = t->p[1].z - t->p[0].z;

            line2.x = t->p[2].x - t->p[0].x;
            line2.y = t->p[2].y - t->p[0].y;
            line2.z = t->p[2].z - t->p[0].z;

            // take the cross product of lines to get normal to triangle surface
            normal = vector_cross_product(&line1, &line2);

            if (normal.z > 0.0f) {
                vec3d tp = t->p[0];
                vec2d tt = t->t[0];
                vec3d tc = t->c[0];
                t->p[0] = t->p[1];
                t->t[0] = t->t[1];
                t->c[0] = t->c[1];
                t->p[1] = tp;
                t->t[1] = tt;
                t->c[1] = tc;
            }
            draw_triangle(t->p, t->t, t->c, texture, clamp_s, clamp_t, true, perspective_correct);
        }
    }
}