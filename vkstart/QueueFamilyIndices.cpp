#include "QueueFamilyIndices.h"

namespace vkstart
{

QueueFamilyIndices::QueueFamilyIndices(const vk::raii::PhysicalDevice &physicalDevice)
{
    vk::QueueFlags queueFlagsZero{};
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
        physicalDevice.getQueueFamilyProperties();
    for (auto i = 0; i < queueFamilyProperties.size(); ++i)
    {
        const vk::QueueFamilyProperties &properties = queueFamilyProperties[i];
        vk::QueueFlags graphicFlag = properties.queueFlags & vk::QueueFlagBits::eGraphics;
        if (graphicFlag != queueFlagsZero)
        {
            m_graphicsIndex = i;
        }
    }
}

bool QueueFamilyIndices::IsComplete() const
{
    return m_graphicsIndex.has_value();
}

std::optional<uint32_t> QueueFamilyIndices::GraphicsIndex() const
{
    return m_graphicsIndex;
}

} // namespace vkstart