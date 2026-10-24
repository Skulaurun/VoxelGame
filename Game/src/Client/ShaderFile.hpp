#pragma once

namespace Client {

struct ShaderFile {

constexpr static char defaultVertex[] =
{
#embed "../../shaders/default.vert"
};

constexpr static char defaultFragment[] =
{
#embed "../../shaders/default.frag"
};

constexpr static char lightingVertex[] =
{
#embed "../../shaders/lighting.vert"
};

constexpr static char lightingFragment[] =
{
#embed "../../shaders/lighting.frag"
};

constexpr static char shadowVertex[] =
{
#embed "../../shaders/shadow.vert"
};

constexpr static char debugFragment[] =
{
#embed "../../shaders/debug.frag"
};

};

}
