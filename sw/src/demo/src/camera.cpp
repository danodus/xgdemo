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

void Camera::follow_plane(const Plane& plane)
{
    vec3d position = plane.transform_point({0.0f, 1.0f, -5.0f});
    auto forward = plane.get_forward();
    vec3d target = vector_add(&plane.m_position, &forward);
    vec3d up = plane.get_up();

    m_position = position;
    m_target = target;
    m_up = up;        
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