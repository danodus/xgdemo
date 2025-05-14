#include "entity.h"

Entity::Entity(const char* model_path, const char* texture_path)
{
    if (!load_model(&m_model, model_path))
        return;

    if (!load_texture(&m_texture, texture_path))
        return;
}

void Entity::draw(const vec3d* camera_pos, const mat4x4* camera_mat_proj, const mat4x4* camera_mat_view, const light_t* lights, size_t nb_lights)
{
    int fb_width, fb_height;
    get_fb_dimensions(&fb_width, &fb_height);
    draw_model(fb_width, fb_height, camera_pos, &m_model, &m_transform, &m_transform_normal, camera_mat_proj, camera_mat_view, lights, nb_lights, &m_texture, false, false, true);
}