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

    const char *applicationName = "Hello Triangles";
    const uint32_t applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    const char *engineName = "vkstart";
    const uint32_t engineVersion = VK_MAKE_VERSION(0, 0, 1);
    const uint32_t apiVersion = vk::ApiVersion14;
    const vk::ApplicationInfo applicationInfo{applicationName, applicationVersion, engineName,
                                              apiVersion};

    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo =
        DebugMessenger::DebugMessengerCreateInfo();

    const std::vector<const char *> enabledExtensionNames =
        ValidationLayers::RequiredExtensions(context, windowInstanceExtensions);
    const std::vector<const char *> enabledLayerNames = ValidationLayers::Required();
    vk::InstanceCreateInfo createInfo{
        {}, &applicationInfo, enabledLayerNames, enabledExtensionNames};

    vk::InstanceCreateInfo instanceCreateInfo;
    if (ValidationLayers::Enabled())
    {

        const vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>
            createInfos = {createInfo, debugMessengerCreateInfo};
        instanceCreateInfo = createInfos.get<vk::InstanceCreateInfo>();
    }
    else
    {
        instanceCreateInfo = createInfo;
    }

    return vk::raii::Instance{context, instanceCreateInfo};
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

        vk::QueueFlags queueFlagsZero{};
        auto queueFamilyProperties = device.getQueueFamilyProperties();
        bool supportsGraphics = std::ranges::any_of(
            queueFamilyProperties, [&queueFlagsZero](const vk::QueueFamilyProperties &qfp) {
                vk::QueueFlags flags = qfp.queueFlags & vk::QueueFlagBits::eGraphics;
                return flags != queueFlagsZero;
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