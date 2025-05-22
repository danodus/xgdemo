// sw_rasterizer_standard2.c
// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Ref.: https://github.com/Haggarman/Software-3D-Perspective-Correct-Textured-Triangles

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "sw_rasterizer.h"

typedef struct {
    fx32 x, y, w;
    fx32 u, v;
    fx32 r, g, b;
} vertex8;

static int g_fb_width, g_fb_height;
static draw_pixel_fn_t g_draw_pixel_fn;

static fx32* g_depth_buffer;

void sw_init_rasterizer_standard2(int fb_width, int fb_height, draw_pixel_fn_t draw_pixel_fn) {
    g_fb_width = fb_width;
    g_fb_height = fb_height;
    g_depth_buffer = (fx32*)malloc(fb_width * fb_height * sizeof(fx32));
    g_draw_pixel_fn = draw_pixel_fn;
}

void sw_dispose_rasterizer_standard2() { free(g_depth_buffer); }

void sw_clear_depth_buffer_standard2() { memset(g_depth_buffer, FX(0.0f), g_fb_width * g_fb_height * sizeof(fx32)); }

static void swapv8(vertex8* a, vertex8* b) {
    vertex8 t = *a;
    *a = *b;
    *b = t;
}

void sw_draw_triangle_standard2(
    fx32 x0, fx32 y0, fx32 w0, fx32 u0, fx32 v0, fx32 r0, fx32 g0, fx32 b0, fx32 a0,
    fx32 x1, fx32 y1, fx32 w1, fx32 u1, fx32 v1, fx32 r1, fx32 g1, fx32 b1, fx32 a1,
    fx32 x2, fx32 y2, fx32 w2, fx32 u2, fx32 v2, fx32 r2, fx32 g2, fx32 b2, fx32 a2,
    const uint16_t* tex_addr, int tex_scale_x, int tex_scale_y, bool clamp_s, bool clamp_t, bool depth_test, bool persp_correct)
{
    vertex8 a = {x0, y0, w0, u0, v0, r0, g0, b0};
    vertex8 b = {x1, y1, w1, u1, v1, r1, g1, b1};
    vertex8 c = {x2, y2, w2, u2, v2, r2, g2, b2};
    
    // Sort so that vertex A is on top and C is on the bottom.

    if (b.y < a.y)
        swapv8(&a, &b);
    if (c.y < a.y)
        swapv8(&a, &c);
    if (c.y < b.y)
        swapv8(&b, &c);
    
    int draw_min_y, draw_max_y;

    // Integer window clipping
    draw_min_y = ceilf(FLT(a.y));
    if (draw_min_y < 0)
        draw_min_y = 0;
    draw_max_y = ceilf(FLT(c.y)) - 1;
    if (draw_max_y > g_fb_height - 1)
        draw_max_y = g_fb_height - 1;
    if (draw_max_y - draw_min_y < 0)
        return;

    vertex8 delta1, delta2;

    // Determine the deltas (length)
    // delta2 is from A to C (the full triangle height)
    delta2.x = c.x - a.x;
    delta2.y = c.y - a.y;
    delta2.w = c.w - a.w;
    delta2.u = c.u - a.u;
    delta2.v = c.v - a.v;
    delta2.r = c.r - a.r;
    delta2.g = c.g - a.g;
    delta2.b = c.b - a.b;

    // Avoid div by 0
    // Entire Y height less than 1/256 would not have meaningful pixel color change
    if (delta2.y <= FX(1.0 / 256.0f))
        return;

    // Determine vertical Y steps for DDA style math
    // DDA is Digital Differential Analyzer
    // It is an accumulator that counts from a known start point to an end point, in equal increments defined by the number of steps in-between.
    // Probably faster nowadays to do the one division at the start, instead of Bresenham, anyway.
    fx32 legx1_step;
    fx32 legw1_step, legu1_step, legv1_step;
    fx32 legr1_step, legg1_step, legb1_step;

    fx32 legx2_step;
    fx32 legw2_step, legu2_step, legv2_step;
    fx32 legr2_step, legg2_step, legb2_step;

    // Leg 2 steps from A to C (the full triangle height)
    legx2_step = DIV(delta2.x, delta2.y);
    legw2_step = DIV(delta2.w, delta2.y);
    legu2_step = DIV(delta2.u, delta2.y);
    legv2_step = DIV(delta2.v, delta2.y);
    legr2_step = DIV(delta2.r, delta2.y);
    legg2_step = DIV(delta2.g, delta2.y);
    legb2_step = DIV(delta2.b, delta2.y);

    // Leg 1, Draw top to middle
    // For most triangles, draw downward from the apex A to a knee B.
    // That knee could be on either the left or right side, but that is handled much later.
    int draw_middle_y;
    draw_middle_y = ceilf(FLT(b.y));
    // Do not clip B to max_y. Let the y count expire before reaching the knee if it is past bottom of screen.
    if (draw_middle_y < 0)
        draw_middle_y = 0;

    // Leg 1 is from A to B (right now)
    delta1.x = b.x - a.x;
    delta1.y = b.y - a.y;
    delta1.w = b.w - a.w;
    delta1.u = b.u - a.u;
    delta1.v = b.v - a.v;
    delta1.r = b.r - a.r;
    delta1.g = b.g - a.g;
    delta1.b = b.b - a.b;

    // If the triangle has no knee, this section gets skipped to avoid divide by 0.
    // That is okay, because the recalculate Leg 1 from B to C triggers before actually drawing.
    if (delta1.y > FX(1.0f / 256.0f)) {
        // Find Leg 1 steps in the y direction from A to B
        legx1_step = DIV(delta1.x, delta1.y);
        legw1_step = DIV(delta1.w, delta1.y);
        legu1_step = DIV(delta1.u, delta1.y);
        legv1_step = DIV(delta1.v, delta1.y);
        legr1_step = DIV(delta1.r, delta1.y);
        legg1_step = DIV(delta1.g, delta1.y);
        legb1_step = DIV(delta1.b, delta1.y);
    }

    // Y accumulators
    fx32 leg_x1;
    fx32 leg_w1, leg_u1, leg_v1;
    fx32 leg_r1, leg_g1, leg_b1;

    fx32 leg_x2;
    fx32 leg_w2, leg_u2, leg_v2;
    fx32 leg_r2, leg_g2, leg_b2;

    fx32 prestep_y1;
    // Basically we are sampling pixels on integer exact rows.
    // But we only are able to know the next row by way of forward interpolation. So always round up.
    // To get to that next row, we have to prestep by the fractional forward distance from A. _Ceil(A.y) - A.y
    prestep_y1 = FXI(draw_min_y) - a.y;

    leg_x1 = a.x + MUL(prestep_y1, legx1_step);
    leg_w1 = a.w + MUL(prestep_y1, legw1_step);
    leg_u1 = a.u + MUL(prestep_y1, legu1_step);
    leg_v1 = a.v + MUL(prestep_y1, legv1_step);
    leg_r1 = a.r + MUL(prestep_y1, legr1_step);
    leg_g1 = a.g + MUL(prestep_y1, legg1_step);
    leg_b1 = a.b + MUL(prestep_y1, legb1_step);

    leg_x2 = a.x + MUL(prestep_y1, legx2_step);
    leg_w2 = a.w + MUL(prestep_y1, legw2_step);
    leg_u2 = a.u + MUL(prestep_y1, legu2_step);
    leg_v2 = a.v + MUL(prestep_y1, legv2_step);
    leg_r2 = a.r + MUL(prestep_y1, legr2_step);
    leg_g2 = a.g + MUL(prestep_y1, legg2_step);
    leg_b2 = a.b + MUL(prestep_y1, legb2_step);
    
    // Inner loop vars
    int row;
    int col;
    int draw_max_x;
    fx32 tex_z; // 1/w helper (multiply by inverse is faster than dividing each time)

    // Stepping along the X direction
    fx32 delta_x;
    fx32 prestep_x;
    fx32 tex_w_step, tex_u_step, tex_v_step;
    fx32 tex_r_step, tex_g_step, tex_b_step;

    // X Accumulators
    fx32 tex_w, tex_u, tex_v;
    fx32 tex_r, tex_g, tex_b;

    row = draw_min_y;
    while (row <= draw_max_y) {
        if (row == draw_middle_y) {
            // Reached Leg 1 knee at B, recalculate Leg 1.
            // This overwrites Leg 1 to be from B to C. Leg 2 just keeps continuing from A to C.
            delta1.x = c.x - b.x;
            delta1.y = c.y - b.y;
            delta1.w = c.w - b.w;
            delta1.u = c.u - b.u;
            delta1.v = c.v - b.v;
            delta1.r = c.r - b.r;
            delta1.g = c.g - b.g;
            delta1.b = c.b - b.b;

            if (delta1.y == FX(0.0f))
                return;
            
            // Full steps in the y direction from B to C
            legx1_step = DIV(delta1.x, delta1.y);
            legw1_step = DIV(delta1.w, delta1.y);
            legu1_step = DIV(delta1.u, delta1.y);
            legv1_step = DIV(delta1.v, delta1.y);
            legr1_step = DIV(delta1.r, delta1.y);
            legg1_step = DIV(delta1.g, delta1.y);
            legb1_step = DIV(delta1.b, delta1.y);

            // Most cases has B lower downscreen than A.
            // B > A usually. Only one case where B = A.
            prestep_y1 = FXI(draw_middle_y) - b.y;

            // Re-Initialize DDA start values
            leg_x1 = b.x + MUL(prestep_y1, legx1_step);
            leg_w1 = b.w + MUL(prestep_y1, legw1_step);
            leg_u1 = b.u + MUL(prestep_y1, legu1_step);
            leg_v1 = b.v + MUL(prestep_y1, legv1_step);
            leg_r1 = b.r + MUL(prestep_y1, legr1_step);
            leg_g1 = b.g + MUL(prestep_y1, legg1_step);
            leg_b1 = b.b + MUL(prestep_y1, legb1_step);
        }

        // Horizontal Scanline
        delta_x = FX(fabs(FLT(leg_x2 - leg_x1)));
        // Avoid div/0, this gets tiring.
        if (delta_x >= FX(1.0f / 2048.0f)) {
            // Calculate step, start, and end values.
            // Drawing left to right, as in incrementing from a lower to higher memory address, is usually fastest.
            if (leg_x1 < leg_x2) {
                // leg 1 is on the left
                tex_w_step = DIV((leg_w2 - leg_w1), delta_x);
                tex_u_step = DIV((leg_u2 - leg_u1), delta_x);
                tex_v_step = DIV((leg_v2 - leg_v1), delta_x);
                tex_r_step = DIV((leg_r2 - leg_r1), delta_x);
                tex_g_step = DIV((leg_g2 - leg_g1), delta_x);
                tex_b_step = DIV((leg_b2 - leg_b1), delta_x);

                // Set the horizontal starting point to (1)
                col = ceilf(FLT(leg_x1));
                if (col < 0)
                    col = 0;

                // Prestep to find pixel starting point
                prestep_x = FXI(col) - leg_x1;
                tex_w = leg_w1 + MUL(prestep_x, tex_w_step);
                tex_u = leg_u1 + MUL(prestep_x, tex_u_step);
                tex_v = leg_v1 + MUL(prestep_x, tex_v_step);
                tex_r = leg_r1 + MUL(prestep_x, tex_r_step);
                tex_g = leg_g1 + MUL(prestep_x, tex_g_step);
                tex_b = leg_b1 + MUL(prestep_x, tex_b_step);

                // ending point is (2)
                draw_max_x = ceilf(FLT(leg_x2));
                if (draw_max_x > g_fb_width - 1)
                    draw_max_x = g_fb_width - 1;
            } else {
                // Things are flipped. leg 1 is on the right.
                tex_w_step = DIV((leg_w1 - leg_w2), delta_x);
                tex_u_step = DIV((leg_u1 - leg_u2), delta_x);
                tex_v_step = DIV((leg_v1 - leg_v2), delta_x);
                tex_r_step = DIV((leg_r1 - leg_r2), delta_x);
                tex_g_step = DIV((leg_g1 - leg_g2), delta_x);
                tex_b_step = DIV((leg_b1 - leg_b2), delta_x);

                // Set the horizontal starting point to (2)
                col = ceilf(FLT(leg_x2));
                if (col < 0)
                    col = 0;

                // Prestep to find pixel starting point
                prestep_x = FXI(col) - leg_x2;
                tex_w = leg_w2 + MUL(prestep_x, tex_w_step);
                tex_u = leg_u2 + MUL(prestep_x, tex_u_step);
                tex_v = leg_v2 + MUL(prestep_x, tex_v_step);
                tex_r = leg_r2 + MUL(prestep_x, tex_r_step);
                tex_g = leg_g2 + MUL(prestep_x, tex_g_step);
                tex_b = leg_b2 + MUL(prestep_x, tex_b_step);

                // ending point is (1)
                draw_max_x = ceilf(FLT(leg_x1));
                if (draw_max_x > g_fb_width)
                    draw_max_x = g_fb_width - 1;
            }

            // Draw the Horizontal Scanline
            while (col < draw_max_x) {
                sw_fragment_shader(g_fb_width, g_fb_height, col, row, tex_w, tex_u, tex_v, tex_r, tex_g, tex_b, FX(1.0f), clamp_s, clamp_t, depth_test, tex_addr, tex_scale_x, tex_scale_y, g_depth_buffer, persp_correct, g_draw_pixel_fn);
                tex_w = tex_w + tex_w_step;
                tex_u = tex_u + tex_u_step;
                tex_v = tex_v + tex_v_step;
                tex_r = tex_r + tex_r_step;
                tex_g = tex_g + tex_g_step;
                tex_b = tex_b + tex_b_step;
                col = col + 1;
            } // col

        } // end div/0 avoidance

        // DDA next step
        leg_x1 = leg_x1 + legx1_step;
        leg_w1 = leg_w1 + legw1_step;
        leg_u1 = leg_u1 + legu1_step;
        leg_v1 = leg_v1 + legv1_step;
        leg_r1 = leg_r1 + legr1_step;
        leg_g1 = leg_g1 + legg1_step;
        leg_b1 = leg_b1 + legb1_step;

        leg_x2 = leg_x2 + legx2_step;
        leg_w2 = leg_w2 + legw2_step;
        leg_u2 = leg_u2 + legu2_step;
        leg_v2 = leg_v2 + legv2_step;
        leg_r2 = leg_r2 + legr2_step;
        leg_g2 = leg_g2 + legg2_step;
        leg_b2 = leg_b2 + legb2_step;

        row = row + 1;
    }
}
