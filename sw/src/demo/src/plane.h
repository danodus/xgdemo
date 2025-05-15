#pragma once

#include "entity.h"

class Plane : public Entity {

public:
    Plane(const char* model_path, const char* texture_path);

    void update(float delta_time);
    vec3d transform_point(vec3d point) const;

    float m_input_roll = 0.0f;
    float m_input_pitch = 0.0f;
    float m_input_yaw = 0.0f;
    float m_input_forward = 0.0f;

    vec3d m_position;
    quaternion m_rotation;    

    vec3d get_forward() const {
        vec3d v = {0.0f, 0.0f, 1.0f, 0.0f};
        return vector_rotate_by_quaternion(&v, &m_rotation);
    };

    vec3d get_left() const {
        vec3d v = {-1.0f, 0.0f, 0.0f, 0.0f};
        return vector_rotate_by_quaternion(&v, &m_rotation);
    };

    vec3d get_right() const {
        vec3d v = {1.0f, 0.0f, 0.0f, 0.0f};
        return vector_rotate_by_quaternion(&v, &m_rotation);
    };

    vec3d get_up() const {
        vec3d v = {0.0f, 1.0f, 0.0f, 0.0f};
        return vector_rotate_by_quaternion(&v, &m_rotation);
    };
};