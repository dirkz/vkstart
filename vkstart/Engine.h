#pragma once

#include "stdafx.h"

#include "DebugMessenger.h"
#include "QueueFamilyIndices.h"

namespace vkstart
{

struct Engine
{
    Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
           std::span<const char *> requiredInstanceExtensions);

    void SetupDebugMessenger();

  private:
    vk::raii::Context m_context;
    vk::raii::Instance m_instance;
    vk::raii::PhysicalDevice m_physicalDevice;
    QueueFamilyIndices m_queueFamilyIndices;
    vk::raii::Device m_device;

    std::unique_ptr<DebugMessenger> m_debugMessenger;
};

} // namespace vkstart
