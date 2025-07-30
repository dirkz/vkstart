#pragma once

#include "stdafx.h"

namespace vkstart
{

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Color;
    glm::vec2 TextureCoordinates;

    static vk::VertexInputBindingDescription GetBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescriptions();
};

} // namespace vkstart
