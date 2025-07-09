#include "Engine.h"

#include "ValidationLayers.h"

namespace vkstart
{

const std::unordered_set<std::string> RequiredDeviceExtensions{
    vk::KHRSwapchainExtensionName, vk::KHRSpirv14ExtensionName,
    vk::KHRSynchronization2ExtensionName, vk::KHRCreateRenderpass2ExtensionName};

void Engine::DrawFrame()
{
    m_presentQueue.waitIdle();

    auto [result, imageIndex] = m_swapchain.acquireNextImage(std::numeric_limits<uint64_t>::max(),
                                                             *m_presentCompleteSemaphore);
    RecordCommandBuffer(imageIndex);

    m_device.resetFences(*m_drawFence);
    const vk::PipelineStageFlags waitDestinationStageMask{
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    const vk::SubmitInfo submitInfo{*m_presentCompleteSemaphore, waitDestinationStageMask,
                                    *m_commandBuffer, *m_renderFinishedSemaphore};
    m_graphicsQueue.submit(submitInfo, *m_drawFence);

    while (vk::Result::eTimeout ==
           m_device.waitForFences(*m_drawFence, vk::True, std::numeric_limits<uint64_t>::max()))
    {
    }

    const vk::PresentInfoKHR presentInfoKHR{*m_renderFinishedSemaphore, *m_swapchain, imageIndex};
    result = m_presentQueue.presentKHR(presentInfoKHR);

    switch (result)
    {
    case vk::Result::eSuccess:
        break;
    case vk::Result::eSuboptimalKHR:
        SDL_Log("vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR!");
        break;
    default:
        break; // an unexpected result is returned!
    }
}

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

void Engine::CreateSwapChain(int pixelWidth, int pixelHeight)
{
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

    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};

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

void Engine::CreateCommandBuffer()
{
    const uint32_t commandBufferCount = 1;
    vk::CommandBufferAllocateInfo allocInfo{m_commandPool, vk::CommandBufferLevel::ePrimary,
                                            commandBufferCount};
    m_commandBuffer = std::move(vk::raii::CommandBuffers{m_device, allocInfo}.front());
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

    m_commandBuffer.pipelineBarrier2(dependencyInfo);
}

void Engine::RecordCommandBuffer(uint32_t imageIndex)
{
    m_commandBuffer.begin({});

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

    m_commandBuffer.beginRendering(renderingInfo);

    m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);

    m_commandBuffer.setViewport(
        0, vk::Viewport{0.0f, 0.0f, static_cast<float>(m_swapchainExtent.width),
                        static_cast<float>(m_swapchainExtent.height), 0.0f, 1.0f});
    m_commandBuffer.setScissor(0, vk::Rect2D{vk::Offset2D{0, 0}, m_swapchainExtent});

    const uint32_t vertexCount = 3;
    const uint32_t instanceCount = 1;
    const uint32_t firstVertex = 0;
    const uint32_t firstInstance = 0;
    m_commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);

    m_commandBuffer.endRendering();

    // After rendering, transition the swapchain image to PRESENT_SRC
    TransitionImageLayout(imageIndex, vk::ImageLayout::eColorAttachmentOptimal,
                          vk::ImageLayout::ePresentSrcKHR,
                          vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
                          {},                                                 // dstAccessMask
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
                          vk::PipelineStageFlagBits2::eBottomOfPipe           // dstStage
    );

    m_commandBuffer.end();
}

void Engine::CreateSyncObjects()
{
    const vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    m_presentCompleteSemaphore = vk::raii::Semaphore{m_device, semaphoreCreateInfo};
    m_renderFinishedSemaphore = vk::raii::Semaphore{m_device, semaphoreCreateInfo};
    m_drawFence = vk::raii::Fence{m_device, {vk::FenceCreateFlagBits::eSignaled}};
}

} // namespace vkstart