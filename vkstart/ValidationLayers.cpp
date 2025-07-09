#include "ValidationLayers.h"

namespace vkstart
{

const std::vector ValidationLayersList = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool ValidationLayersEnabled = false;
#else
constexpr bool ValidationLayersEnabled = true;
#endif

bool ValidationLayers::Enabled()
{
    return ValidationLayersEnabled;
}

std::vector<const char *> ValidationLayers::Required()
{
    return ValidationLayersList;
}

bool ValidationLayers::CheckSupport(vk::raii::Context &context)
{
    if (!ValidationLayersEnabled)
    {
        return true;
    }

    std::vector<vk::LayerProperties> instanceLayerProperties =
        context.enumerateInstanceLayerProperties();

    // require that ALL our validation layers (all_of) are found in the instance layers (any_of)
    bool all = std::ranges::all_of(
        ValidationLayersList, [&instanceLayerProperties](const char *pLayerName) {
            return std::ranges::any_of(instanceLayerProperties,
                                       [&pLayerName](vk::LayerProperties const &lp) {
                                           return strcmp(pLayerName, lp.layerName) == 0;
                                       });
        });

    return all;
}

std::vector<const char *> ValidationLayers::RequiredExtensions(
    vk::raii::Context &context, std::span<const char *> requiredInstanceExtensions)
{
    std::vector<const char *> extensions{requiredInstanceExtensions.begin(),
                                         requiredInstanceExtensions.end()};

    if (ValidationLayersEnabled)
    {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    auto extensionProperties = context.enumerateInstanceExtensionProperties();

    for (auto extensionName : extensions)
    {
        bool supported = std::ranges::any_of(
            extensionProperties, [&extensionName](vk::ExtensionProperties const &ep) {
                return strcmp(ep.extensionName, extensionName) == 0;
            });
        if (!supported)
        {
            std::string msg{"extension "};
            throw std::runtime_error{msg + extensionName + " is requested, but not supported"};
        }
    }

    return extensions;
}

} // namespace vkstart