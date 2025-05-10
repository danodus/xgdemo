#pragma once

#include "common.h"

class Camera {

public:
    Camera(float fov);

    void begin_drawing(float pitch, float yaw);
    void end_drawing();

    mat4x4 m_mat_proj;
    vec3d m_vec_camera;
    mat4x4 m_mat_view;
    vec3d m_vec_look_dir;
};