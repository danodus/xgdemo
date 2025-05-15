#pragma once

#include "common.h"

class Entity
{
public:
    Entity(const char* model_path, const char* texture_path);
    void draw(const vec3d* camera_pos, const mat4x4* camera_mat_proj, const mat4x4* camera_mat_view, const light_t* lights, size_t nb_lights);

    mat4x4 m_transform, m_transform_normal;

    bool m_visible = true;

private:
    model_t m_model;
    texture_t m_texture;
};