#include "Engine.h"

#include "ValidationLayers.h"

namespace vkstart
{

Engine::Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
               std::span<const char *> windowInstanceExtensions)
    : m_context{vkGetInstanceProcAddr}
{
    for (auto extension : windowInstanceExtensions)
    {
        SDL_Log("requested instance extension: %s", extension);
    }

    CreateInstance(windowInstanceExtensions);
}

void Engine::CreateInstance(std::span<const char *> windowInstanceExtensions)
{
    bool validationLayersSupported = ValidationLayers::CheckSupport(m_context);
    if (!validationLayersSupported)
    {
        throw std::runtime_error{"required validation layers not available"};
    }

    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName = "Hello Triangles",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "vkstart",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = vk::ApiVersion14,
    };

    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo =
        DebugMessenger::DebugMessengerCreateInfo();

    auto extensions = ValidationLayers::RequiredExtensions(m_context, windowInstanceExtensions);
    auto validationLayers = ValidationLayers::Required();

    vk::InstanceCreateInfo createInfo{
        .pNext = &debugMessengerCreateInfo,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
        .ppEnabledLayerNames = validationLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    m_instance = std::make_unique<vk::raii::Instance>(m_context, createInfo);
}

void Engine::SetupDebugMessenger()
{
    if (!ValidationLayers::Enabled())
    {
        return;
    }

    m_debugMessenger = std::make_unique<DebugMessenger>(*m_instance);
}

} // namespace vkstart