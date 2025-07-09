#pragma once

#include "stdafx.h"

namespace vkstart
{

struct ValidationLayers
{
    static bool Enabled();
    static std::vector<const char *> Required();
    static bool CheckSupport(vk::raii::Context &context);
    static std::vector<const char *> RequiredExtensions(
        vk::raii::Context &context, std::span<const char *> windowInstanceExtensions);
};

} // namespace vkstart
