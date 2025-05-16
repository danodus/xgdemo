#pragma once

extern "C" {
    #include "graphite.h"
}

#include "plane.h"

class Camera {

public:

    enum class Views {
        FOLLOW,
        COCKPIT_FORWARD,
        COCKPIT_LEFT,
        COCKPIT_RIGHT,
        TOWER
    };

    Camera(float fov);

    void update(Views view, const Plane& plane, vec3d tower_position);

    void begin_drawing();
    void end_drawing();

    mat4x4 m_mat_proj;
    vec3d m_position;
    vec3d m_target;
    vec3d m_up;

    mat4x4 m_mat_view;
};