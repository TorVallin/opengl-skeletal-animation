cmake_minimum_required(VERSION 3.0)
SET(TARGET_NAME opengl-skeletal-animation)
project(${TARGET_NAME})

SET(OpenGL_GL_PREFERENCE "GLVND")
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "-g -Wall -Wextra -pedantic -Werror=return-type")

# Path to other .cmake files
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

# Define the executable
add_executable(${PROJECT_NAME} src/main.cpp src/Window.cpp src/Window.h src/shader/Shader.cpp src/shader/Shader.h src/shapes/Grid.h src/models/Model.h src/models/AnimatedModelLoader.h src/models/Mesh.h src/renderer/Renderer.cpp src/renderer/Renderer.h src/models/Conversions.cpp src/models/Conversions.h src/models/Model.cpp src/models/animation/Bone.cpp src/models/animation/Bone.h src/models/AnimatedModelLoader.cpp src/models/TextureLoader.h)

find_package(OpenGL REQUIRED)

# glm
find_package(glm REQUIRED)

# glfw3
find_package(GLFW3 REQUIRED)

# glad
add_library(glad "dependencies/glad/src/glad.c")

# stb_image
add_library(stb_image "dependencies/stb_image/stb_image.cpp" "dependencies/stb_image/stb_image.hpp")

# assimp
find_package(assimp REQUIRED)
message(STATUS "Found assimp")

# Stores all variables in the LIBS variable
SET(LIBS glfw glad OpenGL assimp stb_image)

# Define the include DIRs
include_directories(
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/dependencies/glad/include"
        "${CMAKE_SOURCE_DIR}/dependencies/stb_image"
)

link_directories(${CMAKE_SOURCE_DIR}/${TARGET_NAME})
# Define the link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
