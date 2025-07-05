#include "DebugMessenger.h"

namespace vkstart
{

static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
{
    if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
    {
        SDL_Log("%s %s", to_string(type).c_str(), pCallbackData->pMessage);
    }

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
    const vk::DebugUtilsMessageSeverityFlagsEXT messageSeverity(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    const vk::DebugUtilsMessageTypeFlagsEXT messageType(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
        {}, messageSeverity, messageType, &DebugCallback};

    return debugUtilsMessengerCreateInfoEXT;
}

} // namespace vkstart