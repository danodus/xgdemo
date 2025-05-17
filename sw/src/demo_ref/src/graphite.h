// graphite.h
// Copyright (c) 2021-2024 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef GRAPHITE_H
#define GRAPHITE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <array.h>
#include <glib.h>

#include <SDL.h>

void graphite_init(SDL_Renderer* renderer, int fb_width, int fb_height);
void get_fb_dimensions(int* fb_width, int* fb_height);

bool load_model(model_t *model, const char *obj_filename);
bool load_texture(texture_t *texture, const char *tex_filename);

void clear(unsigned int color);
void swap(void);

#endif