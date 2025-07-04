#include "Engine.h"

namespace vkstart
{

Engine::Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
               std::span<const char *> instanceExtensions)
    : m_context{vkGetInstanceProcAddr}
{
    for (auto extension : instanceExtensions)
    {
        SDL_Log("instance extension: %s", extension);
    }
}

} // namespace vkstart