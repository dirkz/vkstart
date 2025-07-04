#include "Engine.h"

namespace vkstart
{

Engine::Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr) : m_context{vkGetInstanceProcAddr}
{
}

} // namespace vkstart