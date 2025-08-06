#pragma once

#include "stdafx.h"

namespace vkstart
{

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Color;
    glm::vec2 TextureCoordinates;

    Vertex(glm::vec3 &position, glm::vec3 &color, glm::vec2 &textureCoordinates);

    static vk::VertexInputBindingDescription GetBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescriptions();

    bool operator==(const Vertex &other) const
    {
        return Position == other.Position && Color == other.Color &&
               TextureCoordinates == other.TextureCoordinates;
    }
};

} // namespace vkstart

namespace std
{

template <> struct hash<vkstart::Vertex>
{
    size_t operator()(vkstart::Vertex const &vertex) const
    {
        return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec3>()(vertex.Color) << 1)) >>
                1) ^
               (hash<glm::vec2>()(vertex.TextureCoordinates) << 1);
    }
};

} // namespace std
