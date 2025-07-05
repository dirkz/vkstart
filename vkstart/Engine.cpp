#include "Engine.h"

#include "ValidationLayers.h"

namespace vkstart
{

static vk::raii::Instance CreateInstance(vk::raii::Context &context,
                                         std::span<const char *> windowInstanceExtensions)
{
    bool validationLayersSupported = ValidationLayers::CheckSupport(context);
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

    auto extensions = ValidationLayers::RequiredExtensions(context, windowInstanceExtensions);
    auto validationLayers = ValidationLayers::Required();

    vk::InstanceCreateInfo createInfo{
        .pNext = &debugMessengerCreateInfo,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
        .ppEnabledLayerNames = validationLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    return vk::raii::Instance{context, createInfo};
}

static vk::raii::PhysicalDevice PickPhysicalDevice(vk::raii::Instance &instance)
{
    std::unordered_set<std::string> requiredDeviceExtensions{
        vk::KHRSwapchainExtensionName, vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName, vk::KHRCreateRenderpass2ExtensionName};

    auto devices = instance.enumeratePhysicalDevices();
    for (const vk::raii::PhysicalDevice &device : devices)
    {
        if (device.getProperties().apiVersion < VK_API_VERSION_1_3)
        {
            continue;
        }

        auto queueFamilyProperties = device.getQueueFamilyProperties();
        bool supportsGraphics =
            std::ranges::any_of(queueFamilyProperties, [](const vk::QueueFamilyProperties &qfp) {
                vk::QueueFlags flags = qfp.queueFlags & vk::QueueFlagBits::eGraphics;
                vk::QueueFlags zero{};
                return flags != zero;
            });
        if (!supportsGraphics)
        {
            continue;
        }

        std::unordered_set<std::string> foundExtensions{};
        auto extensions = device.enumerateDeviceExtensionProperties();
        for (const vk::ExtensionProperties &extension : extensions)
        {
            if (requiredDeviceExtensions.contains(extension.extensionName))
            {
                foundExtensions.insert(extension.extensionName);
            }
        }
        if (foundExtensions == requiredDeviceExtensions)
        {
            return device;
        }
    }

    throw std::runtime_error{"no suitable physical device found"};
}

Engine::Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
               std::span<const char *> windowInstanceExtensions)
    : m_context{vkGetInstanceProcAddr},
      m_instance{CreateInstance(m_context, windowInstanceExtensions)},
      m_physicalDevice{PickPhysicalDevice(m_instance)}
{
    SetupDebugMessenger();
}

void Engine::SetupDebugMessenger()
{
    if (!ValidationLayers::Enabled())
    {
        return;
    }

    m_debugMessenger = std::make_unique<DebugMessenger>(m_instance);
}

} // namespace vkstart