#pragma once

#include "camera.h"

class Entity
{
public:
    Entity(const char* model_path, const char* texture_path);
    void draw(const Camera* camera, const light_t* lights, size_t nb_lights);

    vec3d m_position;
    vec3d m_velocity;
    vec3d m_rotation;
    //quaternion m_rotation;

private:
    model_t m_model;
    texture_t m_texture;
};