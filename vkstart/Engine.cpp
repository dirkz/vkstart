#include "Engine.h"

#include "ValidationLayers.h"
#include "Vertex.h"

namespace vkstart
{

const std::vector<Vertex> Vertices = {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

constexpr uint32_t MaxFramesInFlight = 2;

const std::unordered_set<std::string> RequiredDeviceExtensions{
    vk::KHRSwapchainExtensionName, vk::KHRSpirv14ExtensionName,
    vk::KHRSynchronization2ExtensionName, vk::KHRCreateRenderpass2ExtensionName};

Engine::Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr, IWindow *window)

    : m_context{vkGetInstanceProcAddr}, m_window{window}
{
    CreateInstance();
    SetupDebugMessenger();

    m_surface = window->CreateSurface(m_instance);

    PickPhysicalDevice();
    CreateDevice();

    m_graphicsQueue = vk::raii::Queue{m_device, m_queueFamilyIndices.GraphicsIndex(), 0};
    m_presentQueue = vk::raii::Queue{m_device, m_queueFamilyIndices.PresentIndex(), 0};

    CreateSwapChain();

    CreateImageViews();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateVertexBuffer();
    CreateCommandBuffer();
    CreateSyncObjects();
}

void Engine::DrawFrame()
{
    while (vk::Result::eTimeout == m_device.waitForFences({m_inFlightFences[m_currentFrame]},
                                                          vk::True,
                                                          std::numeric_limits<uint64_t>::max()))
    {
    }

    const vk::Semaphore waitSemaphore = m_presentCompleteSemaphores[m_currentImage];

    auto [result, imageIndex] =
        m_swapchain.acquireNextImage(std::numeric_limits<uint64_t>::max(), waitSemaphore);

    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        ReCreateSwapChain();
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    m_device.resetFences({m_inFlightFences[m_currentFrame]});

    m_commandBuffers[m_currentFrame].reset();

    RecordCommandBuffer(imageIndex);

    const vk::PipelineStageFlags waitDestinationStageMask{
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    const vk::CommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];
    const vk::Semaphore signalSemaphore = m_renderFinishedSemaphores[m_currentImage];
    const vk::SubmitInfo submitInfo{waitSemaphore, waitDestinationStageMask, commandBuffer,
                                    signalSemaphore};
    m_graphicsQueue.submit(submitInfo, m_inFlightFences[m_currentFrame]);

    const vk::Semaphore waitSemaphore2 = signalSemaphore;
    const vk::PresentInfoKHR presentInfoKHR{waitSemaphore2, *m_swapchain, imageIndex};
    result = m_presentQueue.presentKHR(presentInfoKHR);

    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR ||
        m_pixelSizeChanged)
    {
        m_pixelSizeChanged = false;
        ReCreateSwapChain();
    }
    else if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error{"presentKHR failed"};
    }

    m_currentFrame = (m_currentFrame + 1) % MaxFramesInFlight;
    m_currentImage = (m_currentImage + 1) % m_swapchainImages.size();
}

void Engine::PixelSizeChanged()
{
    m_pixelSizeChanged = true;
}

void Engine::WaitIdle()
{
    m_device.waitIdle();
}

void Engine::CreateInstance()
{
    std::vector<std::string> windowInstanceExtensionStrings =
        m_window->RequiredInstanceExtensions();
    std::vector<const char *> windowInstanceExtensions;
    for (const std::string &extension : windowInstanceExtensionStrings)
    {
        windowInstanceExtensions.push_back(extension.c_str());
    }

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
            if (RequiredDeviceExtensions.contains(extension.extensionName))
            {
                foundExtensions.insert(extension.extensionName);
            }
        }
        if (foundExtensions == RequiredDeviceExtensions)
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
    vk::PhysicalDeviceFeatures2 features2 = m_physicalDevice.getFeatures2();

    vk::PhysicalDeviceVulkan13Features vulkan13Features{};
    vulkan13Features.dynamicRendering = vk::True;
    vulkan13Features.synchronization2 = vk::True;

    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{};
    extendedDynamicStateFeatures.extendedDynamicState = vk::True;

    std::vector<const char *> deviceExtensions{};
    for (const std::string &extension : RequiredDeviceExtensions)
    {
        deviceExtensions.push_back(extension.c_str());
    }
    vk::DeviceCreateInfo deviceCreateInfo{{}, queueCreateInfo, {}, deviceExtensions};

    vk::StructureChain<vk::DeviceCreateInfo, vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        createInfos{deviceCreateInfo, features2, vulkan13Features, extendedDynamicStateFeatures};
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

void Engine::CreateSwapChain()
{
    int pixelWidth, pixelHeight;
    m_window->GetPixelDimensions(&pixelWidth, &pixelHeight);

    vk::SurfaceCapabilitiesKHR surfaceCapabilities =
        m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);

    vk::SurfaceFormatKHR swapChainImageFormat =
        ChooseSwapSurfaceFormat(m_physicalDevice.getSurfaceFormatsKHR(m_surface));

    m_swapchainImageFormat = swapChainImageFormat.format;
    m_swapchainExtent = ChooseSwapExtent(surfaceCapabilities, pixelWidth, pixelHeight);

    uint32_t minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    minImageCount =
        (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount)
            ? surfaceCapabilities.maxImageCount
            : minImageCount;

    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
    {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    bool separateQueues =
        m_queueFamilyIndices.GraphicsIndex() != m_queueFamilyIndices.PresentIndex();

    std::vector<uint32_t> queueFamilyIndices{};
    if (separateQueues)
    {
        queueFamilyIndices = {m_queueFamilyIndices.GraphicsIndex(),
                              m_queueFamilyIndices.PresentIndex()};
    }

    const uint32_t imageArrayLayers = 1;
    const auto clipped = vk::True;
    const auto imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    const auto imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
    const vk::PresentModeKHR presentMode =
        ChooseSwapPresentMode(m_physicalDevice.getSurfacePresentModesKHR(m_surface));
    const auto compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    const auto imageSharingMode =
        separateQueues ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
    const vk::SurfaceTransformFlagBitsKHR preTransform = surfaceCapabilities.currentTransform;
    vk::SwapchainCreateInfoKHR swapChainCreateInfo{
        {},
        m_surface,
        minImageCount,
        swapChainImageFormat.format,
        imageColorSpace,
        m_swapchainExtent,
        imageArrayLayers,
        imageUsage,
        imageSharingMode,
        queueFamilyIndices,
        preTransform,
        compositeAlpha,
        presentMode,
        clipped,
    };

    m_swapchain = vk::raii::SwapchainKHR{m_device, swapChainCreateInfo};
    m_swapchainImages = m_swapchain.getImages();
}

void Engine::CleanupSwapChain()
{
    m_swapchainImageViews.clear();
    m_swapchain = nullptr;
}

void Engine::ReCreateSwapChain()
{
    m_device.waitIdle();

    CleanupSwapChain();

    CreateSwapChain();
    CreateImageViews();
}

void Engine::CreateImageViews()
{
    m_swapchainImageViews.clear();

    const vk::ImageAspectFlags imageAspectFlags = vk::ImageAspectFlagBits::eColor;
    const uint32_t baseMipLevel = 0;
    const uint32_t levelCount = 1;
    const uint32_t baseArrayLayer = 0;
    const uint32_t layerCount = 1;
    const vk::ImageSubresourceRange subresourceRange = {imageAspectFlags, baseMipLevel, levelCount,
                                                        baseArrayLayer, layerCount};

    const auto viewType = vk::ImageViewType::e2D;
    const vk::ComponentMapping componentMapping = {
        vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};
    vk::ImageViewCreateInfo imageViewCreateInfo{{} /* flags */,   {} /* image */,
                                                viewType,         m_swapchainImageFormat.format,
                                                componentMapping, subresourceRange};

    for (const vk::Image &image : m_swapchainImages)
    {
        imageViewCreateInfo.image = image;
        m_swapchainImageViews.emplace_back(m_device, imageViewCreateInfo);
    }
}

static std::vector<char> ReadFile(const std::filesystem::path &filename)
{
    std::filesystem::path basePath{sdl::GetBasePath()};
    std::filesystem::path filePath = basePath / filename;

    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

    file.close();

    return buffer;
}

vk::raii::ShaderModule Engine::CreateShaderModule(const std::vector<char> &code) const
{
    const auto codeSize = static_cast<const uint32_t>(code.size());
    const auto *pCode = reinterpret_cast<const uint32_t *>(code.data());
    vk::ShaderModuleCreateInfo createInfo{{}, codeSize, pCode};

    vk::raii::ShaderModule shaderModule{m_device, createInfo};

    return shaderModule;
}

void Engine::CreateGraphicsPipeline()
{
    auto shaderCode = ReadFile(std::filesystem::path{"shaders"} / "shader.slang.spv");
    vk::raii::ShaderModule shaderModule = CreateShaderModule(shaderCode);

    const auto vertexStageFlags = vk::ShaderStageFlagBits::eVertex;
    vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo{
        {}, vertexStageFlags, shaderModule, "VertexMain"};

    const auto fragmentStageFlags = vk::ShaderStageFlagBits::eFragment;
    vk::PipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{
        {}, fragmentStageFlags, shaderModule, "FragmentMain"};

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageCreateInfos{
        vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

    std::vector dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{{}, dynamicStates};

    vk::VertexInputBindingDescription bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
        {}, {bindingDescription}, attributeDescriptions};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
        {}, vk::PrimitiveTopology::eTriangleList};

    vk::Viewport viewport{0.0f,
                          0.0f,
                          static_cast<float>(m_swapchainExtent.width),
                          static_cast<float>(m_swapchainExtent.height),
                          0.0f,
                          1.0f};

    vk::Rect2D scissorRect{vk::Offset2D{0, 0}, m_swapchainExtent};

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{{}, {viewport}, {scissorRect}};

    const auto depthClampEnable = vk::False;
    const auto rasterizerDiscardEnable = vk::False;
    const auto polygonMode = vk::PolygonMode::eFill;
    const auto cullMode = vk::CullModeFlagBits::eBack;
    const auto frontFace = vk::FrontFace::eClockwise;
    const auto depthBiasEnable = vk::False;
    const float depthBiasSlopeFactor = 1.0f;
    const float lineWidth = 1.0f;
    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
        {},        depthClampEnable, rasterizerDiscardEnable, polygonMode, cullMode,
        frontFace, depthBiasEnable,  depthBiasSlopeFactor,    lineWidth};

    const auto rasterizationSamples = vk::SampleCountFlagBits::e1;
    const auto sampleShadingEnabled = vk::False;
    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
        {}, rasterizationSamples, sampleShadingEnabled};

    const auto blendEnabled = vk::False;
    const auto colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    const auto srcColorBlendFactor = vk::BlendFactor::eZero;
    const auto dstColorBlendFactor = vk::BlendFactor::eZero;
    const auto colorBlendOp = vk::BlendOp::eAdd;
    const auto srcAlphaBlendFactor = vk::BlendFactor::eZero;
    const auto dstAlphaBlendFactor = vk::BlendFactor::eZero;
    const auto alphaBlendOp = vk::BlendOp::eAdd;
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        blendEnabled,        srcColorBlendFactor, dstColorBlendFactor, colorBlendOp,
        srcAlphaBlendFactor, dstAlphaBlendFactor, alphaBlendOp,        colorWriteMask};

    const auto logicOpEnabled = vk::False;
    const auto logicOp = vk::LogicOp::eCopy;
    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
        {}, logicOpEnabled, logicOp, {colorBlendAttachment}};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{{}, {}, {}};
    m_pipelineLayout = vk::raii::PipelineLayout{m_device, pipelineLayoutInfo};

    const uint32_t viewMask = 0;
    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{viewMask,
                                                                {m_swapchainImageFormat.format}};

    const std::array<vk::PipelineShaderStageCreateInfo, 2> stages{vertexShaderStageCreateInfo,
                                                                  fragmentShaderStageCreateInfo};
    const vk::PipelineTessellationStateCreateInfo *tesselationStateCreateInfo = nullptr;
    const vk::PipelineDepthStencilStateCreateInfo *depthStencilStateCreateInfo = nullptr;
    vk::GraphicsPipelineCreateInfo pipelineCreateInfo{{},
                                                      stages,
                                                      &vertexInputStateCreateInfo,
                                                      &inputAssemblyStateCreateInfo,
                                                      tesselationStateCreateInfo,
                                                      &viewportStateCreateInfo,
                                                      &rasterizationStateCreateInfo,
                                                      &multisampleStateCreateInfo,
                                                      depthStencilStateCreateInfo,
                                                      &colorBlendStateCreateInfo,
                                                      &dynamicStateCreateInfo,
                                                      m_pipelineLayout};

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> createInfos{
        pipelineCreateInfo, pipelineRenderingCreateInfo};
    vk::GraphicsPipelineCreateInfo createInfo = createInfos.get<vk::GraphicsPipelineCreateInfo>();

    m_graphicsPipeline = vk::raii::Pipeline{m_device, nullptr, createInfo};
}

void Engine::CreateCommandPool()
{
    vk::CommandPoolCreateInfo poolCreateInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                             m_queueFamilyIndices.GraphicsIndex()};
    m_commandPool = vk::raii::CommandPool{m_device, poolCreateInfo};
}

uint32_t Engine::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = m_physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("no suitable memory type found");
}

void Engine::CopyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer,
                        vk::DeviceSize size)
{
    vk::CommandBufferAllocateInfo allocInfo{m_commandPool, vk::CommandBufferLevel::ePrimary, 1};
    vk::raii::CommandBuffer commandCopyBuffer =
        std::move(m_device.allocateCommandBuffers(allocInfo).front());

    commandCopyBuffer.begin(
        vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    commandCopyBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy{0, 0, size});
    commandCopyBuffer.end();

    vk::SubmitInfo submitInfo{{}, {}, *commandCopyBuffer, {}};
    m_graphicsQueue.submit(submitInfo, {});
    m_graphicsQueue.waitIdle();
}

void Engine::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                          vk::MemoryPropertyFlags properties, vk::raii::Buffer &buffer,
                          vk::raii::DeviceMemory &bufferMemory)
{
    vk::BufferCreateInfo bufferCreateInfo{{}, size, usage, vk::SharingMode::eExclusive};
    buffer = vk::raii::Buffer(m_device, bufferCreateInfo);
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    uint32_t memoryType = FindMemoryType(memRequirements.memoryTypeBits, properties);
    vk::MemoryAllocateInfo allocInfo{memRequirements.size, memoryType};
    bufferMemory = vk::raii::DeviceMemory{m_device, allocInfo};
    buffer.bindMemory(*bufferMemory, 0);
}

void Engine::CreateVertexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(Vertices[0]) * Vertices.size();

    vk::BufferCreateInfo stagingCreateInfo{
        {}, bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive};
    vk::raii::Buffer stagingBuffer(m_device, stagingCreateInfo);

    vk::MemoryRequirements memRequirementsStaging = stagingBuffer.getMemoryRequirements();
    const uint32_t stagingMemoryType = FindMemoryType(
        memRequirementsStaging.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    vk::MemoryAllocateInfo memoryAllocateInfoStaging{memRequirementsStaging.size,
                                                     stagingMemoryType};
    vk::raii::DeviceMemory stagingBufferMemory{m_device, memoryAllocateInfoStaging};
    stagingBuffer.bindMemory(stagingBufferMemory, 0);

    void *dataStaging = stagingBufferMemory.mapMemory(0, stagingCreateInfo.size);
    memcpy(dataStaging, Vertices.data(), stagingCreateInfo.size);
    stagingBufferMemory.unmapMemory();

    vk::BufferCreateInfo bufferInfo{{},
                                    bufferSize,
                                    vk::BufferUsageFlagBits::eVertexBuffer |
                                        vk::BufferUsageFlagBits::eTransferDst,
                                    vk::SharingMode::eExclusive};
    m_vertexBuffer = vk::raii::Buffer(m_device, bufferInfo);

    vk::MemoryRequirements memRequirements = m_vertexBuffer.getMemoryRequirements();
    const uint32_t memoryType =
        FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
    vk::MemoryAllocateInfo memoryAllocateInfo{memRequirements.size, memoryType};
    m_vertexBufferMemory = vk::raii::DeviceMemory(m_device, memoryAllocateInfo);

    m_vertexBuffer.bindMemory(*m_vertexBufferMemory, 0);

    CopyBuffer(stagingBuffer, m_vertexBuffer, stagingCreateInfo.size);
}

void Engine::CreateCommandBuffer()
{
    const uint32_t commandBufferCount = MaxFramesInFlight;
    vk::CommandBufferAllocateInfo allocInfo{m_commandPool, vk::CommandBufferLevel::ePrimary,
                                            commandBufferCount};
    m_commandBuffers = vk::raii::CommandBuffers{m_device, allocInfo};
}

void Engine::TransitionImageLayout(uint32_t imageIndex, vk::ImageLayout oldLayout,
                                   vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask,
                                   vk::AccessFlags2 dstAccessMask,
                                   vk::PipelineStageFlags2 srcStageMask,
                                   vk::PipelineStageFlags2 dstStageMask)
{
    const auto aspectMask = vk::ImageAspectFlagBits::eColor;
    const uint32_t baseMipLevel = 0;
    const uint32_t levelCount = 1;
    const uint32_t baseArrayLayer = 0;
    const uint32_t layerCount = 1;
    vk::ImageSubresourceRange subresourceRange = {aspectMask, baseMipLevel, levelCount,
                                                  baseArrayLayer, layerCount};

    vk::ImageMemoryBarrier2 barrier{srcStageMask,
                                    srcAccessMask,
                                    dstStageMask,
                                    dstAccessMask,
                                    oldLayout,
                                    newLayout,
                                    VK_QUEUE_FAMILY_IGNORED,
                                    VK_QUEUE_FAMILY_IGNORED,
                                    m_swapchainImages[imageIndex],
                                    subresourceRange};

    vk::DependencyInfo dependencyInfo = {{}, {}, {}, {barrier}};

    m_commandBuffers[m_currentFrame].pipelineBarrier2(dependencyInfo);
}

void Engine::RecordCommandBuffer(uint32_t imageIndex)
{
    m_commandBuffers[m_currentFrame].begin({});

    // Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
    TransitionImageLayout(imageIndex, vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eColorAttachmentOptimal,
                          {}, // srcAccessMask (no need to wait for previous operations)
                          vk::AccessFlagBits2::eColorAttachmentWrite,        // dstAccessMask
                          vk::PipelineStageFlagBits2::eTopOfPipe,            // srcStage
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput // dstStage
    );

    const auto clearValue = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
    const auto &imageView = m_swapchainImageViews[imageIndex];
    const auto imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    const auto resolveMode = vk::ResolveModeFlagBits::eNone;
    const vk::ImageView resolveImageView = {};
    const auto resolveImageLayout = vk::ImageLayout::eUndefined;
    const auto loadOp = vk::AttachmentLoadOp::eClear;
    const auto storeOp = vk::AttachmentStoreOp::eStore;
    vk::RenderingAttachmentInfo attachmentInfo{imageView,        imageLayout,        resolveMode,
                                               resolveImageView, resolveImageLayout, loadOp,
                                               storeOp,          clearValue};

    const vk::Rect2D renderArea{{0, 0}, m_swapchainExtent};
    const uint32_t layerCount = 1;
    const uint32_t viewMask = 0;
    vk::RenderingInfo renderingInfo = {{}, renderArea, layerCount, viewMask, {attachmentInfo}};

    m_commandBuffers[m_currentFrame].beginRendering(renderingInfo);

    m_commandBuffers[m_currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics,
                                                  m_graphicsPipeline);

    m_commandBuffers[m_currentFrame].bindVertexBuffers(0, {m_vertexBuffer}, {0});

    m_commandBuffers[m_currentFrame].setViewport(
        0, vk::Viewport{0.0f, 0.0f, static_cast<float>(m_swapchainExtent.width),
                        static_cast<float>(m_swapchainExtent.height), 0.0f, 1.0f});
    m_commandBuffers[m_currentFrame].setScissor(0,
                                                vk::Rect2D{vk::Offset2D{0, 0}, m_swapchainExtent});

    const uint32_t vertexCount = static_cast<uint32_t>(Vertices.size());
    const uint32_t instanceCount = 1;
    const uint32_t firstVertex = 0;
    const uint32_t firstInstance = 0;
    m_commandBuffers[m_currentFrame].draw(vertexCount, instanceCount, firstVertex, firstInstance);

    m_commandBuffers[m_currentFrame].endRendering();

    // After rendering, transition the swapchain image to PRESENT_SRC
    TransitionImageLayout(imageIndex, vk::ImageLayout::eColorAttachmentOptimal,
                          vk::ImageLayout::ePresentSrcKHR,
                          vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
                          {},                                                 // dstAccessMask
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
                          vk::PipelineStageFlagBits2::eBottomOfPipe           // dstStage
    );

    m_commandBuffers[m_currentFrame].end();
}

void Engine::CreateSyncObjects()
{
    m_inFlightFences.clear();
    const vk::FenceCreateInfo fenceCreateInfo{vk::FenceCreateFlagBits::eSignaled};
    for (auto i = 0; i < MaxFramesInFlight; ++i)
    {
        m_inFlightFences.emplace_back(m_device, fenceCreateInfo);
    }

    m_presentCompleteSemaphores.clear();
    m_renderFinishedSemaphores.clear();
    const vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    for (auto i = 0; i < m_swapchainImages.size(); ++i)
    {
        m_presentCompleteSemaphores.emplace_back(m_device, semaphoreCreateInfo);
        m_renderFinishedSemaphores.emplace_back(m_device, semaphoreCreateInfo);
    }
}

} // namespace vkstart