#pragma once

#include "stdafx.h"

#include "DebugMessenger.h"

namespace vkstart
{

struct Engine
{
    static vk::raii::Instance CreateInstance(vk::raii::Context &context,
                                             std::span<const char *> instanceExtensions);

    Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
           std::span<const char *> requiredInstanceExtensions);

    void SetupDebugMessenger();

  private:
    vk::raii::Context m_context;
    vk::raii::Instance m_instance;
    std::unique_ptr<DebugMessenger> m_debugMessenger;
};

} // namespace vkstart
