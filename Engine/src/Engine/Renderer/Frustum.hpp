#pragma once

#include <Engine/Core/Math.hpp>

#include <array>

class Frustum {

public:
    void intersectsBox(const Box3& box);

private:
    std::array<Plane, 6> planes;

};
