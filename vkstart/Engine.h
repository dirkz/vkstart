#pragma once

#include "stdafx.h"

namespace vkstart
{

struct Engine
{
    Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
           std::span<const char *> requiredInstanceExtensions);

    void CreateInstance(std::span<const char *> instanceExtensions);
    bool CheckValidationLayerSupport();
    std::vector<const char *> RequiredExtensions(
        std::span<const char *> requiredInstanceExtensions);
    vk::DebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo();
    void SetupDebugMessenger();

  private:
    vk::raii::Context m_context;
    std::unique_ptr<vk::raii::Instance> m_instance;
    std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> m_debugMessenger;
};

} // namespace vkstart
