#include "plane.h"

Plane::Plane(const char* model_path, const char* texture_path) : Entity(model_path, texture_path) {
    m_position = {0.0f};
    m_rotation = quaternion_make_identity();
}

void Plane::update(float delta_time) {
    float roll = m_input_roll * delta_time;
    float pitch = m_input_pitch * delta_time;
    float yaw = m_input_yaw * delta_time;
    float velocity = m_input_forward * delta_time;

    auto vec_forward = get_forward();
    auto velocityVector = vector_mul(&vec_forward, velocity);
    m_position = vector_add(&m_position, &velocityVector);

    auto q = quaternion_from_axis_angle({0.0f, 0.0f, 1.0f}, roll);
    m_rotation = quaternion_multiply(&m_rotation, &q);
    q = quaternion_from_axis_angle({1.0f, 0.0f, 0.0f}, pitch);
    m_rotation = quaternion_multiply(&m_rotation, &q);
    q = quaternion_from_axis_angle({0.0f, 1.0f, 0.0f}, yaw);
    m_rotation = quaternion_multiply(&m_rotation, &q);

    auto transform = matrix_make_translation(m_position.x, m_position.y, m_position.z);
    auto m = quaternion_to_matrix(&m_rotation);
    transform = matrix_multiply_matrix(&m, &transform);

    // The model orientation in Blender is positive X instead of negative Y
    // Apply a rotation to fix this
    m = matrix_make_rotation_y(-(PI / 2));
    transform = matrix_multiply_matrix(&m, &transform);

    m_transform = transform;
    m_transform_normal = quaternion_to_matrix(&m_rotation);  
}

vec3d Plane::transform_point(vec3d point) const {
    auto pos_mat = matrix_make_translation(m_position.x, m_position.y, m_position.z);
    auto rot_mat = quaternion_to_matrix(&m_rotation);
    auto mat = matrix_multiply_matrix(&rot_mat, &pos_mat);
    return matrix_multiply_vector(&mat, &point);
}