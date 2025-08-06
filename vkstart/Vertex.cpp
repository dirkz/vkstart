#include "Vertex.h"

namespace vkstart
{

Vertex::Vertex(glm::vec3 &position, glm::vec3 &color, glm::vec2 &textureCoordinates)
    : Position{position}, Color{color}, TextureCoordinates{textureCoordinates}
{
}

vk::VertexInputBindingDescription Vertex::GetBindingDescription()
{
    return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
}

std::array<vk::VertexInputAttributeDescription, 3> Vertex::GetAttributeDescriptions()
{
    return {vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat,
                                                offsetof(Vertex, Position)),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat,
                                                offsetof(Vertex, Color)),
            vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat,
                                                offsetof(Vertex, TextureCoordinates))};
}

} // namespace vkstart