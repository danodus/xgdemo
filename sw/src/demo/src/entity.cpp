#include "entity.h"

Entity::Entity(const char* model_path, const char* texture_path)
{
    if (!load_model(&m_model, model_path))
        return;

    if (!load_texture(&m_texture, texture_path))
        return;

    m_position = {0.0f};
    m_velocity = {0.0f};
    m_rotation = {0.0f};
    m_rotation = quaternion_from_euler(0.0f, 0.0f, 0.0f);
}

void Entity::draw(const Camera* camera, const light_t* lights, size_t nb_lights)
{
    mat4x4 mat_rot = quaternion_to_matrix(&m_rotation);

    mat4x4 mat_trans = matrix_make_translation(m_position.x, m_position.y, m_position.z);
    mat4x4 mat_world, mat_normal;
    mat_world = matrix_make_identity();
    mat_world = mat_normal = matrix_multiply_matrix(&mat_world, &mat_rot);
    mat_world = matrix_multiply_matrix(&mat_world, &mat_trans);

    int fb_width, fb_height;
    get_fb_dimensions(&fb_width, &fb_height);
    draw_model(fb_width, fb_height, &camera->m_vec_camera, &m_model, &mat_world, &mat_normal, &camera->m_mat_proj, &camera->m_mat_view, lights, nb_lights, &m_texture, false, false, true);
    uint32_t t2_draw = MEM_READ(TIMER);    
}