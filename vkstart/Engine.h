#pragma once

#include "stdafx.h"

#include "DebugMessenger.h"
#include "IWindow.h"
#include "QueueFamilyIndices.h"

namespace vkstart
{

struct Engine
{
    Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr, IWindow *window);

    void DrawFrame();
    void PixelSizeChanged();
    void WaitIdle();

  private:
    void CreateInstance();
    void SetupDebugMessenger();
    void PickPhysicalDevice();
    void CreateDevice();
    void CreateSwapChain();
    void CleanupSwapChain();
    void ReCreateSwapChain();
    void CreateImageViews();
    vk::raii::ShaderModule CreateShaderModule(const std::vector<char> &code) const;
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    void CreateCommandPool();

    vk::raii::CommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(vk::raii::CommandBuffer &commandBuffer);

    void TransitionImageLayout(const vk::raii::Image &image, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout);
    void CopyBufferToImage(const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width,
                           uint32_t height);
    void CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
                     vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
                     vk::raii::Image &image, vk::raii::DeviceMemory &imageMemory);
    void CreateTextureImage();
    void CreateTextureSampler();

    vk::raii::ImageView CreateImageView(vk::raii::Image &image, vk::Format format);
    void CreateTextureImageView();

    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    void CopyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size);
    void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                      vk::MemoryPropertyFlags properties, vk::raii::Buffer &buffer,
                      vk::raii::DeviceMemory &bufferMemory);
    void CreateVertexBuffer();
    void CreateIndexBuffer();

    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();

    void CreateCommandBuffer();
    void TransitionImageLayout(uint32_t imageIndex, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask,
                               vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask,
                               vk::PipelineStageFlags2 dstStageMask);
    void RecordCommandBuffer(uint32_t imageIndex);
    void CreateSyncObjects();

    void UpdateUniformBuffer(uint32_t currentImage);

    vk::raii::Context m_context;
    IWindow *m_window;
    vk::raii::Instance m_instance = nullptr;

    std::unique_ptr<DebugMessenger> m_debugMessenger = nullptr;

    vk::raii::SurfaceKHR m_surface = nullptr;
    vk::raii::PhysicalDevice m_physicalDevice = nullptr;

    QueueFamilyIndices m_queueFamilyIndices;

    vk::raii::Device m_device = nullptr;

    vk::raii::SwapchainKHR m_swapchain = nullptr;
    vk::SurfaceFormatKHR m_swapchainImageFormat;
    vk::Extent2D m_swapchainExtent;
    std::vector<vk::Image> m_swapchainImages;
    std::vector<vk::raii::ImageView> m_swapchainImageViews;

    vk::raii::DescriptorSetLayout m_descriptorSetLayout = nullptr;
    vk::raii::PipelineLayout m_pipelineLayout = nullptr;
    vk::raii::Pipeline m_graphicsPipeline = nullptr;

    vk::raii::DescriptorPool m_descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSet> m_descriptorSets;

    vk::raii::CommandPool m_commandPool = nullptr;

    vk::raii::Image m_textureImage = nullptr;
    vk::raii::DeviceMemory m_textureImageMemory = nullptr;
    vk::raii::ImageView m_textureImageView = nullptr;
    vk::raii::Sampler m_textureSampler = nullptr;

    vk::raii::Buffer m_vertexBuffer = nullptr;
    vk::raii::DeviceMemory m_vertexBufferMemory = nullptr;
    vk::raii::Buffer m_indexBuffer = nullptr;
    vk::raii::DeviceMemory m_indexBufferMemory = nullptr;

    std::vector<vk::raii::Buffer> m_uniformBuffers;
    std::vector<vk::raii::DeviceMemory> m_uniformBuffersMemory;
    std::vector<void *> m_uniformBuffersMapped;

    std::vector<vk::raii::CommandBuffer> m_commandBuffers;

    std::vector<vk::raii::Semaphore> m_presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_inFlightFences;

    uint32_t m_currentFrame = 0;
    uint32_t m_currentImage = 0;

    vk::raii::Queue m_graphicsQueue = nullptr;
    vk::raii::Queue m_presentQueue = nullptr;

    bool m_pixelSizeChanged = false;
};

} // namespace vkstart
