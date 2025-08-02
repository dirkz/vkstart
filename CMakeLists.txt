# CMakeList.txt : CMake project for vulkan-hpp-sdl, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("vkstart-main" LANGUAGES CXX)

# Add source to this project's executable.
if (WIN32)
add_executable (${PROJECT_NAME} WIN32 "main.cpp" )
else()
add_executable (${PROJECT_NAME} "main.cpp" )
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

find_package(Vulkan)

option(SDL_STATIC "Build SDL as a static library" ON)

add_subdirectory(glm)
add_subdirectory(SDL-Hpp)
add_subdirectory(vkstart)
add_subdirectory(shaders)
add_subdirectory(textures)
add_subdirectory(models)

add_dependencies(${PROJECT_NAME} shaders)

target_precompile_headers(${PROJECT_NAME} REUSE_FROM vkstart)

target_link_libraries(${PROJECT_NAME} PRIVATE vkstart SDL-Hpp Vulkan::Vulkan)
