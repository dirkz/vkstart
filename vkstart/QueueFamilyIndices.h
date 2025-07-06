#pragma once

#include "stdafx.h"

namespace vkstart
{

struct QueueFamilyIndices
{
    QueueFamilyIndices(const vk::raii::PhysicalDevice &physicalDevice);

    bool IsComplete() const;

    uint32_t GraphicsIndex() const;

  private:
    std::optional<uint32_t> m_graphicsIndex;
};

} // namespace vkstart
