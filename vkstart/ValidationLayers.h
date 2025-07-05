#pragma once

#include "stdafx.h"

namespace vkstart
{

struct ValidationLayers
{
    static bool EnableValidationLayers();
    static std::vector<const char *> RequiredValidationLayers();
    static bool CheckValidationLayerSupport(vk::raii::Context &context);
    static std::vector<const char *> RequiredExtensions(
        vk::raii::Context &context, std::span<const char *> requiredInstanceExtensions);
};

} // namespace vkstart
