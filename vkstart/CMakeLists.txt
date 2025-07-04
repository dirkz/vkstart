add_library(vkstart
	OBJECT
	Engine.h
	Engine.cpp
	vkstart.h)

target_include_directories(vkstart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(vkstart PRIVATE SDL-Hpp Vulkan::Vulkan)