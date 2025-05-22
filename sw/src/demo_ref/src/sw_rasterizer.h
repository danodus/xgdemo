// sw_rasterizer.h
// Copyright (c) 2021-2022 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef SW_RASTERIZER_H
#define SW_RASTERIZER_H

#include <stdbool.h>
#include <stdint.h>

#ifndef FIXED_POINT
#define FIXED_POINT 1
#endif

#define _FRACTION_MASK(scale) (0xffffffffffffffff >> (64 - scale))
#define _WHOLE_MASK(scale) (0xffffffffffffffff ^ FRACTION_MASK(scale))

#define _FLOAT_TO_FIXED(x, scale) ((int64_t)((x) * (float)(1L << scale)))
#define _FIXED_TO_FLOAT(x, scale) ((float)(x) / (double)(1L << scale))
#define _INT_TO_FIXED(x, scale) ((int64_t)(x) << scale)
#define _FIXED_TO_INT(x, scale) ((int64_t)(x) >> scale)
#define _FRACTION_PART(x, scale) ((int64_t)(x)&FRACTION_MASK(scale))
#define _WHOLE_PART(x, scale) ((int64_t)(x)&WHOLE_MASK(scale))

#define _MUL(x, y, scale) (int64_t)(((int64_t)(x) * (int64_t)(y)) >> scale)
#define _DIV(x, y, scale) (int64_t)(((int64_t)(x) << scale) / (y))

// #define _MUL(x, y, scale) ((int64_t)((x) >> (scale / 2)) * (int)((y) >> (scale / 2)))
// #define _DIV(x, y, scale) (((int64_t)(x) << (scale / 2)) / (int)((y) >> (scale / 2)))

#if FIXED_POINT

typedef int64_t fx32;

#define SCALE 24

#define FX(x) ((fx32)_FLOAT_TO_FIXED(x, SCALE))
#define FXI(x) ((fx32)_INT_TO_FIXED(x, SCALE))
#define INT(x) ((int)_FIXED_TO_INT(x, SCALE))
#define FLT(x) ((float)_FIXED_TO_FLOAT(x, SCALE))
#define DIV(x, y) _DIV(x, y, SCALE)
#define MUL(x, y) _MUL(x, y, SCALE)

#else

typedef float fx32;

#define FX(x) (x)
#define FXI(x) ((float)(x))
#define INT(x) ((int)(x))
#define FLT(x) (x)
#define MUL(x, y) ((x) * (y))
#define DIV(x, y) ((x) / (y))

#endif


typedef void (*draw_pixel_fn_t)(int x, int y, int color);

void sw_init_rasterizer_standard(int fb_width, int fb_height, draw_pixel_fn_t draw_pixel_fn);
void sw_init_rasterizer_standard2(int fb_width, int fb_height, draw_pixel_fn_t draw_pixel_fn);
void sw_dispose_rasterizer_standard();
void sw_dispose_rasterizer_standard2();
void sw_clear_depth_buffer_standard();
void sw_clear_depth_buffer_standard2();

void sw_init_rasterizer_barycentric(int fb_width, int fb_height, draw_pixel_fn_t draw_pixel_fn);
void sw_dispose_rasterizer_barycentric();
void sw_clear_depth_buffer_barycentric();

void sw_fragment_shader(int fb_width, int fb_height, int x, int y, fx32 z, fx32 u, fx32 v, fx32 r, fx32 g, fx32 b, fx32 a, bool clamp_s, bool clamp_t, bool depth_test, const uint16_t* tex_addr, int tex_scale_x, int tex_scale_y, fx32* depth_buffer, bool persp_correct, draw_pixel_fn_t draw_pixel_fn);

void sw_draw_triangle_standard(fx32 x0, fx32 y0, fx32 z0, fx32 u0, fx32 v0, fx32 r0, fx32 g0, fx32 b0, fx32 a0,
                      fx32 x1, fx32 y1, fx32 z1, fx32 u1, fx32 v1, fx32 r1, fx32 g1, fx32 b1, fx32 a1,
                      fx32 x2, fx32 y2, fx32 z2, fx32 u2, fx32 v2, fx32 r2, fx32 g2, fx32 b2, fx32 a2,
                      const uint16_t* tex_addr, int tex_scale_x, int tex_scale_y, bool clamp_s, bool clamp_t, bool depth_test, bool persp_correct);

void sw_draw_triangle_standard2(fx32 x0, fx32 y0, fx32 z0, fx32 u0, fx32 v0, fx32 r0, fx32 g0, fx32 b0, fx32 a0,
                      fx32 x1, fx32 y1, fx32 z1, fx32 u1, fx32 v1, fx32 r1, fx32 g1, fx32 b1, fx32 a1,
                      fx32 x2, fx32 y2, fx32 z2, fx32 u2, fx32 v2, fx32 r2, fx32 g2, fx32 b2, fx32 a2,
                      const uint16_t* tex_addr, int tex_scale_x, int tex_scale_y, bool clamp_s, bool clamp_t, bool depth_test, bool persp_correct);

void sw_draw_triangle_barycentric(fx32 x0, fx32 y0, fx32 z0, fx32 u0, fx32 v0, fx32 r0, fx32 g0, fx32 b0, fx32 a0,
                      fx32 x1, fx32 y1, fx32 z1, fx32 u1, fx32 v1, fx32 r1, fx32 g1, fx32 b1, fx32 a1,
                      fx32 x2, fx32 y2, fx32 z2, fx32 u2, fx32 v2, fx32 r2, fx32 g2, fx32 b2, fx32 a2,
                      const uint16_t* tex_addr, int tex_scale_x, int tex_scale_y, bool clamp_s, bool clamp_t, bool depth_test, bool persp_correct);

#endif  // SW_RASTERIZER_H