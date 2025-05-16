#pragma once

#include "camera.h"
#include "entity.h"

#include <vector>
#include <memory>

class Scene {
public:
    void add_entity(std::shared_ptr<Entity> entity);
    void draw(const Camera* camera, const light_t* lights, size_t nb_lights);

private:
    std::vector<std::shared_ptr<Entity>> m_entities;
};