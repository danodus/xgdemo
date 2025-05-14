#pragma once

#include "common.h"
#include "plane.h"

class Camera {

public:
    Camera(float fov);

    void follow_plane(const Plane& plane);

    void begin_drawing();
    void end_drawing();

    mat4x4 m_mat_proj;
    vec3d m_position;
    vec3d m_target;
    vec3d m_up;

    mat4x4 m_mat_view;
};