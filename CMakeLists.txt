cmake_minimum_required(VERSION 3.20)

set(RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Editor/resources)
set(EDITOR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Editor/src)
set(RUNTIME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/src)
set(UTILS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utils/src)
set(SANDBOX_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Sandbox/src)
set(WALNUT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Walnut/src)
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
set(ICON_FONT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/IconFontAwesome)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)

project(MiniEngine VERSION 0.2.0)

find_package(spdlog REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(Assimp REQUIRED)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/glfw)

# GLAD: OpenGL Loader
set(GLAD_DIR ${THIRD_PARTY_DIR}/glad)
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)
target_link_libraries(glad PRIVATE glfw)

# Dear ImGui
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)
file(GLOB imgui_src ${IMGUI_DIR}/*.cpp)
file(GLOB imgui_backend ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
add_library(imgui STATIC ${imgui_src} ${imgui_backend})
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui glfw glad)

# Imguizmo
set(IMGUIZMO_DIR ${THIRD_PARTY_DIR}/ImGuizmo)
file(GLOB imguizmo_src ${IMGUIZMO_DIR}/Im*.cpp)
add_library(imguizmo OBJECT ${imguizmo_src})
target_include_directories(imgui PUBLIC ${IMGUIZMO_DIR})
target_link_libraries(imguizmo PUBLIC imgui glfw glad)

# Source files
file(GLOB_RECURSE walnut_src ${WALNUT_SOURCE_DIR}/*.cpp )
file(GLOB_RECURSE runtime_src ${RUNTIME_SOURCE_DIR}/*.cpp ${RUNTIME_SOURCE_DIR}/*.embed)
file(GLOB_RECURSE editor_src ${EDITOR_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE utils_src ${UTILS_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE sandbox_src ${SANDBOX_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE resources ${RESOURCE_DIR}/*.rc)

# Utils
add_library(Utils ${utils_src})
target_include_directories(Utils PUBLIC ${UTILS_SOURCE_DIR})

# Engine Runtime for Editor
add_library(Runtime ${runtime_src})
target_include_directories(Runtime PUBLIC ${RUNTIME_SOURCE_DIR} ${THIRD_PARTY_DIR}/tinyobjloader ${THIRD_PARTY_DIR}/stb)
target_include_directories(Runtime PRIVATE ${ICON_FONT_DIR})
target_link_libraries(Runtime PUBLIC spdlog::spdlog glm::glm assimp::assimp glfw glad)
target_link_libraries(Runtime PUBLIC imgui imguizmo)
target_link_libraries(Runtime PUBLIC EnTT::EnTT yaml-cpp)

# Editor
add_executable(Editor ${editor_src} ${resources})
target_include_directories(Editor PRIVATE ${EDITOR_SOURCE_DIR})
target_include_directories(Editor PRIVATE ${THIRD_PARTY_DIR}/glfw/include ${THIRD_PARTY_DIR}/stb)
target_include_directories(Editor PRIVATE ${ICON_FONT_DIR})
target_link_libraries(Editor PRIVATE Runtime)
target_link_libraries(Editor PRIVATE imgui imguizmo)
target_link_libraries(Editor PRIVATE spdlog::spdlog)
target_link_libraries(Editor PRIVATE glad glfw glm::glm)
target_link_libraries(Editor PRIVATE EnTT::EnTT yaml-cpp)

# Sandbox
add_executable(Sandbox ${sandbox_src})
target_include_directories(Sandbox PRIVATE ${EDITOR_SOURCE_DIR})
target_include_directories(Sandbox PRIVATE ${THIRD_PARTY_DIR}/glfw/include ${THIRD_PARTY_DIR}/stb)
target_include_directories(Sandbox PRIVATE ${ICON_FONT_DIR})
target_link_libraries(Sandbox PRIVATE Runtime)
target_link_libraries(Sandbox PRIVATE imgui imguizmo)
target_link_libraries(Sandbox PRIVATE spdlog::spdlog)
target_link_libraries(Sandbox PRIVATE glad glfw glm::glm)
target_link_libraries(Sandbox PRIVATE EnTT::EnTT yaml-cpp)
