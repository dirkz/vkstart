#pragma once

#include "stdafx.h"

namespace vkstart
{

struct QueueFamilyIndices
{
    QueueFamilyIndices() {};

    QueueFamilyIndices(const vk::raii::PhysicalDevice &physicalDevice,
                       const vk::raii::SurfaceKHR &surface);

    bool IsComplete() const;

    uint32_t GraphicsIndex() const;
    uint32_t PresentIndex() const;

  private:
    std::optional<uint32_t> m_graphicsIndex;
    std::optional<uint32_t> m_presentIndex;
};

} // namespace vkstart
