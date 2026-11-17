#pragma once

#include "Component.hpp"

#include <entt/entt.hpp>

namespace Ecs {

inline void simulate(entt::registry& registry, float dt) {
    const glm::vec3 gravity{0.0f, -9.8f, 0.0f};

    for (auto [entity, velocity] : registry.view<Ecs::Velocity>().each()) {
        //velocity.value += gravity * dt;
    }

    for (auto [entity, position, velocity] : registry.view<Ecs::Position, Ecs::Velocity>().each()) {
        //position.value += velocity.value * dt;
    }
}

}
