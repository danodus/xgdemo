#include "scene.h"

void Scene::add_entity(std::shared_ptr<Entity> entity) {
    m_entities.emplace_back(entity);
}

void Scene::draw(const Camera* camera, const light_t* lights, size_t nb_lights) {
    for (auto entity : m_entities)
        entity->draw(camera, lights, nb_lights);
}