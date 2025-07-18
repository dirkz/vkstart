add_library(vkstart OBJECT
	Engine.h
	Engine.cpp
	vkstart.h
	DebugMessenger.h
	DebugMessenger.cpp
	ValidationLayers.h
	ValidationLayers.cpp
	QueueFamilyIndices.h
	QueueFamilyIndices.cpp
	IWindow.h
	SDL3IWindow.h
	SDL3IWindow.cpp
	Vertex.h
	Vertex.cpp)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET vkstart PROPERTY CXX_STANDARD 20)
endif()

target_precompile_headers(vkstart PUBLIC stdafx.h)

target_include_directories(vkstart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(vkstart PRIVATE SDL-Hpp Vulkan::Vulkan glm::glm-header-only)