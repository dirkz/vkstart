#include "QueueFamilyIndices.h"

namespace vkstart
{

QueueFamilyIndices::QueueFamilyIndices(const vk::raii::PhysicalDevice &physicalDevice,
                                       const vk::raii::SurfaceKHR &surface)
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

        VkBool32 presentSupport = physicalDevice.getSurfaceSupportKHR(i, *surface);
        if (presentSupport)
        {
            m_presentIndex = i;
        }
    }
}

bool QueueFamilyIndices::IsComplete() const
{
    return m_graphicsIndex.has_value() && m_presentIndex.has_value();
}

uint32_t QueueFamilyIndices::GraphicsIndex() const
{
    return m_graphicsIndex.value();
}

uint32_t QueueFamilyIndices::PresentIndex() const
{
    return m_presentIndex.value();
}

} // namespace vkstart