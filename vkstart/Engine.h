#pragma once

#include "stdafx.h"

#include "DebugMessenger.h"
#include "EngineCreationHelper.h"
#include "QueueFamilyIndices.h"

namespace vkstart
{

struct Engine
{
    template <class T>
    Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
           std::span<const char *> windowInstanceExtensions, T surfaceCreator)

        : m_context{vkGetInstanceProcAddr},
          m_instance{CreateInstance(m_context, windowInstanceExtensions)},
          m_surface{surfaceCreator(m_instance)},
          m_physicalDevice{PickPhysicalDevice(m_instance, m_surface)},
          m_queueFamilyIndices{m_physicalDevice, m_surface},
          m_device{CreateDevice(m_physicalDevice, m_queueFamilyIndices)},
          m_graphicsQueue{m_device, m_queueFamilyIndices.GraphicsIndex(), 0},
          m_presentQueue{m_device, m_queueFamilyIndices.PresentIndex(), 0}
    {
        SetupDebugMessenger();
    }

    void SetupDebugMessenger();

  private:
    vk::raii::Context m_context;
    vk::raii::Instance m_instance;
    std::unique_ptr<DebugMessenger> m_debugMessenger;
    vk::raii::SurfaceKHR m_surface;
    vk::raii::PhysicalDevice m_physicalDevice;
    QueueFamilyIndices m_queueFamilyIndices;
    vk::raii::Device m_device;
    vk::raii::Queue m_graphicsQueue;
    vk::raii::Queue m_presentQueue;
};

} // namespace vkstart
