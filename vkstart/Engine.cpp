#include "Engine.h"

namespace vkstart
{

const std::vector ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool EnableValidationLayers = false;
#else
constexpr bool EnableValidationLayers = true;
#endif

static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
{
    std::string msg{"type "};
    msg += to_string(type) + " msg: " + pCallbackData->pMessage;
    SDL_Log("VALIDATION LAYER: %s", msg.c_str());

    return vk::False;
}

Engine::Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
               std::span<const char *> requiredInstanceExtensions)
    : m_context{vkGetInstanceProcAddr}
{
    for (auto extension : requiredInstanceExtensions)
    {
        SDL_Log("requested instance extension: %s", extension);
    }

    CreateInstance(requiredInstanceExtensions);
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

    auto extensions = RequiredExtensions(instanceExtensions);
    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = DebugMessengerCreateInfo();

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size()),
        .ppEnabledLayerNames = ValidationLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> createInfos{
        createInfo, debugMessengerCreateInfo};
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

std::vector<const char *> Engine::RequiredExtensions(
    std::span<const char *> requiredInstanceExtensions)
{
    std::vector<const char *> extensions{requiredInstanceExtensions.begin(),
                                         requiredInstanceExtensions.end()};

    if (EnableValidationLayers)
    {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    auto extensionProperties = m_context.enumerateInstanceExtensionProperties();

    for (auto extensionName : extensions)
    {
        bool supported = std::ranges::any_of(
            extensionProperties, [&extensionName](vk::ExtensionProperties const &ep) {
                return strcmp(ep.extensionName, extensionName) == 0;
            });
        if (!supported)
        {
            std::string msg{"extension "};
            throw std::runtime_error{msg + extensionName + " is requested, but not supported"};
        }
    }

    return extensions;
}

vk::DebugUtilsMessengerCreateInfoEXT Engine::DebugMessengerCreateInfo()
{
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
        .flags = {},
        .messageSeverity = severityFlags,
        .messageType = messageTypeFlags,
        .pfnUserCallback = &DebugCallback};

    return debugUtilsMessengerCreateInfoEXT;
}

void Engine::SetupDebugMessenger()
{
    if (!EnableValidationLayers)
    {
        return;
    }

    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT =
        DebugMessengerCreateInfo();

    m_debugMessenger = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(
        *m_instance, debugUtilsMessengerCreateInfoEXT);
}

} // namespace vkstart