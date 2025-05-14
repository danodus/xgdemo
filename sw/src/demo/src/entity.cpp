#include "entity.h"

Entity::Entity(const char* model_path, const char* texture_path)
{
    if (!load_model(&m_model, model_path))
        return;

    if (!load_texture(&m_texture, texture_path))
        return;
}

void Entity::draw(const Camera* camera, const light_t* lights, size_t nb_lights)
{
    int fb_width, fb_height;
    get_fb_dimensions(&fb_width, &fb_height);
    draw_model(fb_width, fb_height, &camera->m_vec_camera, &m_model, &m_transform, &m_transform_normal, &camera->m_mat_proj, &camera->m_mat_view, lights, nb_lights, &m_texture, false, false, true);
}