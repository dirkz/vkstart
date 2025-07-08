#pragma once

#include "stdafx.h"

#include "DebugMessenger.h"
#include "QueueFamilyIndices.h"

namespace vkstart
{

struct Engine
{
    template <class T>
    Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
           std::span<const char *> windowInstanceExtensions, T surfaceCreator, int pixelWidth,
           int pixelHeight)

        : m_context{vkGetInstanceProcAddr}
    {
        CreateInstance(windowInstanceExtensions);
        SetupDebugMessenger();

        m_surface = surfaceCreator(m_instance);

        PickPhysicalDevice();
        CreateDevice();

        m_graphicsQueue = vk::raii::Queue{m_device, m_queueFamilyIndices.GraphicsIndex(), 0};
        m_presentQueue = vk::raii::Queue{m_device, m_queueFamilyIndices.PresentIndex(), 0};

        CreateSwapChain(pixelWidth, pixelHeight);
        CreateImageViews();
        CreateGraphicsPipeline();
        CreateCommandPool();
        CreateCommandBuffer();
    }

  private:
    void CreateInstance(std::span<const char *> windowInstanceExtensions);
    void SetupDebugMessenger();
    void PickPhysicalDevice();
    void CreateDevice();
    void CreateSwapChain(int pixelWidth, int pixelHeight);
    void CreateImageViews();
    vk::raii::ShaderModule CreateShaderModule(const std::vector<char> &code) const;
    void CreateGraphicsPipeline();
    void CreateCommandPool();
    void CreateCommandBuffer();

    vk::raii::Context m_context;
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

    vk::raii::PipelineLayout m_pipelineLayout = nullptr;
    vk::raii::Pipeline m_graphicsPipeline = nullptr;

    vk::raii::CommandPool m_commandPool = nullptr;
    vk::raii::CommandBuffer m_commandBuffer = nullptr;

    vk::raii::Queue m_graphicsQueue = nullptr;
    vk::raii::Queue m_presentQueue = nullptr;
};

} // namespace vkstart
