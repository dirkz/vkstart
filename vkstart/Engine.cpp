#include "Engine.h"

namespace vkstart
{

const std::vector ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool EnableValidationLayers = false;
#else
constexpr bool EnableValidationLayers = true;
#endif

Engine::Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
               std::span<const char *> instanceExtensions)
    : m_context{vkGetInstanceProcAddr}
{
    for (auto extension : instanceExtensions)
    {
        SDL_Log("requested instance extension: %s", extension);
    }

    CreateInstance(instanceExtensions);
}

void Engine::CreateInstance(std::span<const char *> instanceExtensions)
{
    bool validationLayersSupported = CheckValidationLayerSupport();
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

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size()),
        .ppEnabledLayerNames = ValidationLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data(),
    };

    m_instance = std::make_unique<vk::raii::Instance>(m_context, createInfo);
}

bool Engine::CheckValidationLayerSupport()
{
    if (!EnableValidationLayers)
    {
        return true;
    }

    auto instanceLayers = m_context.enumerateInstanceLayerProperties();

    // require that ALL our validation layers (all_of) are found in the instance layers (any_of)
    bool all = std::ranges::all_of(ValidationLayers, [&instanceLayers](const char *pLayerName) {
        return std::ranges::any_of(instanceLayers, [&pLayerName](vk::LayerProperties const &lp) {
            return strcmp(pLayerName, lp.layerName) == 0;
        });
    });

    return all;
}

} // namespace vkstart