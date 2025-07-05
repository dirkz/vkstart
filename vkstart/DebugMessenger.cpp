#include "DebugMessenger.h"

namespace vkstart
{

static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
{
    std::string msg{"type "};
    msg += to_string(type) + " msg: " + pCallbackData->pMessage;
    SDL_Log("VALIDATION LAYER: %s", msg.c_str());

    return vk::False;
}

DebugMessenger::DebugMessenger(vk::raii::Instance &instance)
{
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT =
        DebugMessengerCreateInfo();

    m_debugMessenger = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(
        instance, debugUtilsMessengerCreateInfoEXT);
}

vk::DebugUtilsMessengerCreateInfoEXT DebugMessenger::DebugMessengerCreateInfo()
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

} // namespace vkstart