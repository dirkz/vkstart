#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#define VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL 0
#include <vulkan/vulkan_raii.hpp>

#include <cassert>
#include <exception>
#include <memory>
#include <span>
#include <string>
#include <unordered_set>
#include <vector>

#include <SDL.hpp>
