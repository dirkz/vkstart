#pragma once

#include "stdafx.h"

namespace vkstart
{

struct Vertex
{
    glm::vec2 Position;
    glm::vec3 Color;

    static vk::VertexInputBindingDescription GetBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescriptions();
};

} // namespace vkstart
