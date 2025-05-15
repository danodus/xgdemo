#include "camera.h"

Camera::Camera(float fov)
{
    int fb_width, fb_height;
    get_fb_dimensions(&fb_width, &fb_height);
    m_mat_proj = matrix_make_projection(fb_width, fb_height, fov);
    m_position = {0.0f, 5.0f, -30.0f, 1.0f};
    m_target = {0.0f, 0.0f, 0.0f, 1.0f};
    m_up = {0.0f, 1.0f, 0.0f, 1.0f};
}

void Camera::update(Views view, const Plane& plane, vec3d tower_position)
{
    if (view == Camera::Views::TOWER) {
        m_position = tower_position;
        m_target = plane.m_position;
        m_up = {0.0f, 1.0f, 0.0f, 1.0f};
    } else {
        vec3d position = plane.transform_point((view == Camera::Views::FOLLOW) ? vec3d{0.0f, 1.0f, -5.0f, 1.0f} : vec3d{0.0f, 0.2f, 1.2f, 1.0f});
        auto dir = (view == Camera::Views::COCKPIT_LEFT) ? plane.get_left() :
            (view == Camera::Views::COCKPIT_RIGHT ) ? plane.get_right() :
            plane.get_forward();

        dir = vector_mul(&dir, 10.0f);
        vec3d target = vector_add(&plane.m_position, &dir);
        vec3d up = plane.get_up();

        m_position = position;
        m_target = target;
        m_up = up;
    }
}

void Camera::begin_drawing()
{
    mat4x4 mat_camera = matrix_point_at(&m_position, &m_target, &m_up);

    // make view matrix from camera
    m_mat_view = matrix_quick_inverse(&mat_camera);    
}

void Camera::end_drawing()
{
}