#pragma once

struct ShaderFile {

constexpr static char defaultVertex[] =
{
#embed "../../shaders/default.vert"
};

constexpr static char defaultFragment[] =
{
#embed "../../shaders/default.frag"
};

};
