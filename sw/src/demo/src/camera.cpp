#include "camera.h"

Camera::Camera(float fov)
{
    int fb_width, fb_height;
    get_fb_dimensions(&fb_width, &fb_height);
    m_mat_proj = matrix_make_projection(fb_width, fb_height, fov);
    m_vec_camera = {0.0f, 5.0f, -30.0f, 1.0f};
}

void Camera::begin_drawing(float pitch, float yaw)
{
    vec3d vec_up = {0.0f, 1.0f, 0.0f, 1.0f};

    vec3d vec_target = {0.0f, 0.0f, 1.0f, 1.0f};

    // quaternion quat_rotation = quaternion_make_identity();
    // quaternion quat_rotation_x = quaternion_from_axis_angle({1.0f, 0.0f, 0.0f}, pitch);
    // quaternion quat_rotation_y = quaternion_from_axis_angle({0.0f, 1.0f, 0.0f}, yaw);
    // quat_rotation = quaternion_multiply(&quat_rotation, &quat_rotation_y);
    // quat_rotation = quaternion_multiply(&quat_rotation, &quat_rotation_x);
    // mat4x4 mat_camera_rot = quaternion_to_matrix(&quat_rotation);

    mat4x4 mat_camera_rot_y = matrix_make_rotation_y(yaw);
    mat4x4 mat_camera_rot_x = matrix_make_rotation_x(pitch);
    mat4x4 mat_camera_rot = matrix_multiply_matrix(&mat_camera_rot_x, &mat_camera_rot_y);

    m_vec_look_dir = matrix_multiply_vector(&mat_camera_rot, &vec_target);
    vec_target = vector_add(&m_vec_camera, &m_vec_look_dir);

    mat4x4 mat_camera = matrix_point_at(&m_vec_camera, &vec_target, &vec_up);

    // make view matrix from camera
    m_mat_view = matrix_quick_inverse(&mat_camera);    
}

void Camera::end_drawing()
{
}