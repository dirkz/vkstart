#pragma once

#include "stdafx.h"

namespace vkstart
{

struct Engine
{
    Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr);

  private:
    vk::raii::Context m_context;
};

} // namespace vkstart
