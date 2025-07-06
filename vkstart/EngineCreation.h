#pragma once

#include "stdafx.h"

#include "QueueFamilyIndices.h"

namespace vkstart
{

vk::raii::Instance CreateInstance(vk::raii::Context &context,
                                  std::span<const char *> windowInstanceExtensions);
vk::raii::PhysicalDevice PickPhysicalDevice(vk::raii::Instance &instance);
vk::raii::Device CreateDevice(vk::raii::PhysicalDevice &physicalDevice,
                              QueueFamilyIndices &queueFamilyIndices);

} // namespace vkstart
