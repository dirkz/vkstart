#include "Engine.h"

#include "ValidationLayers.h"

namespace vkstart
{

void Engine::SetupDebugMessenger()
{
    if (!ValidationLayers::Enabled())
    {
        return;
    }

    m_debugMessenger = std::make_unique<DebugMessenger>(m_instance);
}

} // namespace vkstart