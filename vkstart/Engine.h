#pragma once

#include "stdafx.h"

namespace vkstart
{

struct Engine
{
    Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
           std::span<const char *> instanceExtensions);

    void CreateInstance(std::span<const char *> instanceExtensions);

  private:
    vk::raii::Context m_context;
    std::unique_ptr<vk::raii::Instance> m_instance;
};

} // namespace vkstart
