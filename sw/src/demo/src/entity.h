#pragma once

#include "camera.h"

class Entity
{
public:
    Entity(const char* model_path, const char* texture_path);
    void draw(const Camera* camera, const light_t* lights, size_t nb_lights);

    mat4x4 m_transform, m_transform_normal;

private:
    model_t m_model;
    texture_t m_texture;
};