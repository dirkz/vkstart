#include "EngineCreationHelper.h"

#include "DebugMessenger.h"
#include "ValidationLayers.h"

namespace vkstart
{

vk::raii::Instance CreateInstance(vk::raii::Context &context,
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
                                              engineVersion, apiVersion};

    const std::vector<const char *> enabledExtensionNames =
        ValidationLayers::RequiredExtensions(context, windowInstanceExtensions);
    const std::vector<const char *> enabledLayerNames = ValidationLayers::Required();
    vk::InstanceCreateInfo basicInstanceCreateInfo{
        {}, &applicationInfo, enabledLayerNames, enabledExtensionNames};

    vk::InstanceCreateInfo instanceCreateInfo;
    if (ValidationLayers::Enabled())
    {
        vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo =
            DebugMessenger::DebugMessengerCreateInfo();
        const vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>
            createInfos = {basicInstanceCreateInfo, debugMessengerCreateInfo};
        instanceCreateInfo = createInfos.get<vk::InstanceCreateInfo>();
    }
    else
    {
        instanceCreateInfo = basicInstanceCreateInfo;
    }

    return vk::raii::Instance{context, instanceCreateInfo};
}

vk::raii::PhysicalDevice PickPhysicalDevice(const vk::raii::Instance &instance,
                                            const vk::raii::SurfaceKHR &surface)
{
    std::unordered_set<std::string> requiredDeviceExtensions{
        vk::KHRSwapchainExtensionName, vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName, vk::KHRCreateRenderpass2ExtensionName};

    auto devices = instance.enumeratePhysicalDevices();
    for (const vk::raii::PhysicalDevice &physicalDevice : devices)
    {
        if (physicalDevice.getProperties().apiVersion < VK_API_VERSION_1_3)
        {
            continue;
        }

        QueueFamilyIndices familiyIndices{physicalDevice, surface};
        if (!familiyIndices.IsComplete())
        {
            continue;
        }

        std::unordered_set<std::string> foundExtensions{};
        auto extensions = physicalDevice.enumerateDeviceExtensionProperties();
        for (const vk::ExtensionProperties &extension : extensions)
        {
            if (requiredDeviceExtensions.contains(extension.extensionName))
            {
                foundExtensions.insert(extension.extensionName);
            }
        }
        if (foundExtensions == requiredDeviceExtensions)
        {
            return physicalDevice;
        }
    }

    throw std::runtime_error{"no suitable physical device found"};
}

vk::raii::Device CreateDevice(vk::raii::PhysicalDevice &physicalDevice,
                              QueueFamilyIndices &queueFamilyIndices)
{
    const uint32_t graphicsIndex = queueFamilyIndices.GraphicsIndex();
    const std::array<float, 1> priorities{0.0f};
    vk::DeviceQueueCreateInfo queueCreateInfo{{}, graphicsIndex, priorities};

    // query for Vulkan 1.3 features
    vk::PhysicalDeviceFeatures2 features = physicalDevice.getFeatures2();

    vk::PhysicalDeviceVulkan13Features vulkan13Features{};
    vulkan13Features.dynamicRendering = vk::True;

    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{};
    extendedDynamicStateFeatures.extendedDynamicState = vk::True;

    const std::vector<const char *> deviceExtensions = {
        vk::KHRSwapchainExtensionName, vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName, vk::KHRCreateRenderpass2ExtensionName};
    vk::DeviceCreateInfo deviceCreateInfo{{}, queueCreateInfo, {}, deviceExtensions};

    vk::StructureChain<vk::DeviceCreateInfo, vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        createInfos{deviceCreateInfo, features, vulkan13Features, extendedDynamicStateFeatures};
    vk::DeviceCreateInfo deviceCreateInfoChained = createInfos.get<vk::DeviceCreateInfo>();

    vk::raii::Device device{physicalDevice, deviceCreateInfoChained};

    return device;
}

} // namespace vkstart