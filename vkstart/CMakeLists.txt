add_library(vkstart OBJECT Engine.h Engine.cpp)
target_link_libraries(vkstart PRIVATE SDL-Hpp Vulkan::Vulkan)