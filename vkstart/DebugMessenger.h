#pragma once

#include "stdafx.h"

namespace vkstart
{

struct DebugMessenger
{
    DebugMessenger(vk::raii::Instance &instance);

    static vk::DebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo();

  private:
    std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> m_debugMessenger;
};

} // namespace vkstart
