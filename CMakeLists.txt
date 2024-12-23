cmake_minimum_required(VERSION 3.0)
project(todhchai)

# If you want C++11 or later:
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# If you want to keep builds in separate directories
# put all .a, .so, .dll, .exe in the same place:
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Build external dependencies (glfw, glad, etc.)
add_subdirectory(external)

# Include directories for your project + externals
include_directories(
    external/glfw-3.1.2/include/
    external/glm-0.9.7.1/
    external/glad-opengl-3.3/include/
    external/
    todhchai/             # If your .h files are in `todhchai/`
)

add_executable(todhchai
    todhchai/todhchai.cpp
    todhchai/helpers/camera.cpp
    todhchai/objects/skybox.cpp
    todhchai/objects/building.cpp
    todhchai/helpers/texture.cpp
    todhchai/render/shader.cpp
)

# Link in OpenGL, glfw, glad, etc.
target_link_libraries(todhchai
    ${OPENGL_LIBRARY}
    glfw
    glad
)
