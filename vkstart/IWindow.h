#pragma once

#include "stdafx.h"

namespace vkstart
{

struct IWindow
{
    virtual vk::raii::SurfaceKHR CreateSurface(const vk::raii::Instance &instance) = 0;
    virtual void GetWindowPixelDimensions(int &width, int &height) = 0;
};

} // namespace vkstart
