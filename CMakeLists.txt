cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

project(Final)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# Add custom modules path if any
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Add config
configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)


# Define output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Link directories (if you have custom libraries)
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Find required packages
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

if(APPLE)
    find_package(Freetype REQUIRED)
    message(STATUS "Found Freetype in ${FREETYPE_INCLUDE_DIRS}")
endif()

# Define libraries based on the operating system
if(WIN32)
    set(LIBS glfw3 opengl32 assimp freetype)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
    find_package(OpenGL REQUIRED)
    add_definitions(${OPENGL_DEFINITIONS})
    find_package(X11 REQUIRED)
    # Order matters for linking
    set(LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread freetype ${ASSIMP_LIBRARY})
    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(IOKit_LIBRARY IOKit)
    find_library(CoreVideo_LIBRARY CoreVideo)
    set(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
    set(APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY} ${FREETYPE_LIBRARIES})
    set(LIBS ${APPLE_LIBS})
else()
    set(LIBS )
endif()

# Add static libraries
add_library(STB_IMAGE src/stb_image.cpp)
add_library(GLAD src/glad.c)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/includes
    ${GLM_INCLUDE_DIR}
    ${GLFW3_INCLUDE_DIR}
    ${ASSIMP_INCLUDE_DIR}
    src/includes/*.h
    src/*.h
)

if(APPLE)
    include_directories(${FREETYPE_INCLUDE_DIRS})
endif()

# Define the executable
add_executable(
    Final
    src/main.cpp
    src/scenes.cpp
    src/includes/Utils.cpp
    src/includes/Object.cpp
    src/includes/AnimatedObject.cpp
    src/includes/Cube.cpp
    src/includes/Sun.cpp
    src/includes/Input.cpp
    src/includes/BloomFBO.cpp
    src/includes/BloomRenderer.cpp
    )

# Link libraries to the executable
target_link_libraries(Final ${LIBS} STB_IMAGE GLAD)

# Platform-specific compile options
if(MSVC)
    target_compile_options(Final PRIVATE /std:c++17 /MP)
    target_link_options(Final PUBLIC /ignore:4099)
endif()

# Function to copy shaders after building
add_custom_command(TARGET Final POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/shaders ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
)

# Optional: If you have DLLs to copy (for Windows)
if(WIN32)
    file(GLOB DLLS "dlls/*.dll")
    foreach(DLL ${DLLS})
        add_custom_command(TARGET Final POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
    endforeach()
endif()

if(WIN32)
    file(GLOB DLLS "dlls/*.dll")
    foreach(DLL ${DLLS})
        add_custom_command(TARGET Final POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
    endforeach()
endif()


# Optionally, configure Visual Studio debugger working directory
if(MSVC)
    set_target_properties(Final PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
endif()
