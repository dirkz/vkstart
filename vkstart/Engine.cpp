#include "Engine.h"

#include "ValidationLayers.h"

namespace vkstart
{

void Engine::CreateInstance(std::span<const char *> windowInstanceExtensions)
{
    bool validationLayersSupported = ValidationLayers::CheckSupport(m_context);
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
        ValidationLayers::RequiredExtensions(m_context, windowInstanceExtensions);
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

    m_instance = vk::raii::Instance{m_context, instanceCreateInfo};
}

void Engine::SetupDebugMessenger()
{
    if (!ValidationLayers::Enabled())
    {
        return;
    }

    m_debugMessenger = std::make_unique<DebugMessenger>(m_instance);
}

void Engine::PickPhysicalDevice()
{
    std::unordered_set<std::string> requiredDeviceExtensions{
        vk::KHRSwapchainExtensionName, vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName, vk::KHRCreateRenderpass2ExtensionName};

    auto devices = m_instance.enumeratePhysicalDevices();
    for (const vk::raii::PhysicalDevice &physicalDevice : devices)
    {
        if (physicalDevice.getProperties().apiVersion < VK_API_VERSION_1_3)
        {
            continue;
        }

        QueueFamilyIndices familiyIndices{physicalDevice, m_surface};
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
            m_physicalDevice = physicalDevice;
            m_queueFamilyIndices = familiyIndices;

            return;
        }
    }

    throw std::runtime_error{"no suitable physical device found"};
}

void Engine::CreateDevice()
{
    const uint32_t graphicsIndex = m_queueFamilyIndices.GraphicsIndex();
    const std::array<float, 1> priorities{0.0f};
    vk::DeviceQueueCreateInfo queueCreateInfo{{}, graphicsIndex, priorities};

    // query for Vulkan 1.3 features
    vk::PhysicalDeviceFeatures2 features = m_physicalDevice.getFeatures2();

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

    m_device = vk::raii::Device{m_physicalDevice, deviceCreateInfoChained};
}

static vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

static vk::PresentModeKHR ChooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
        {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

static vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, int pixelWidth,
                                     int pixelHeight)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    return {std::clamp<uint32_t>(pixelWidth, capabilities.minImageExtent.width,
                                 capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(pixelHeight, capabilities.minImageExtent.height,
                                 capabilities.maxImageExtent.height)};
}

void Engine::CreateSwapChain(int pixelWidth, int pixelHeight)
{
    auto surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
    auto swapChainImageFormat =
        ChooseSwapSurfaceFormat(m_physicalDevice.getSurfaceFormatsKHR(m_surface));
    auto swapChainExtent = ChooseSwapExtent(surfaceCapabilities, pixelWidth, pixelHeight);
    auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    minImageCount =
        (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount)
            ? surfaceCapabilities.maxImageCount
            : minImageCount;
}

} // namespace vkstart