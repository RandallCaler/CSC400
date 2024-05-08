cmake_minimum_required(VERSION 3.4)

# Assuming FindGfxLibs.cmake is a module that helps find or configure OpenGL, GLFW3, GLM, etc.
include("${CMAKE_SOURCE_DIR}/FindGfxLibs.cmake")

# Name of the project
project(Penguin)

# Enforce use of C++14
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

# Use glob to get the list of all source files.
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/ext/*/*.cpp" "${CMAKE_SOURCE_DIR}/ext/glad/src/*.c")

# We don't really need to include header and resource files to build, but it's
# nice to have them show up in IDEs.
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/src/*.h" "${CMAKE_SOURCE_DIR}/ext/*/*.h" "${CMAKE_SOURCE_DIR}/ext/glad/*/*.h")
file(GLOB_RECURSE GLSL "${CMAKE_SOURCE_DIR}/resources/*.glsl")

# Include directories
include_directories("${CMAKE_SOURCE_DIR}/ext")
include_directories("${CMAKE_SOURCE_DIR}/ext/glad/include")
# Assuming Assimp headers are within the ext/assimp/include directory
include_directories("${CMAKE_SOURCE_DIR}/ext/assimp/include")
include_directories("${CMAKE_SOURCE_DIR}/ext/imgui/include")

# Set the executable.
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})

# Helper function included from FindGfxLibs.cmake to find and configure GLFW3 and GLM
findGLFW3(${CMAKE_PROJECT_NAME})
findGLM(${CMAKE_PROJECT_NAME})
findASSIMP(${CMAKE_PROJECT_NAME})

# OS specific options and libraries
if(NOT WIN32)
  message(STATUS "Adding GCC style compiler flags")
  add_compile_options("-Wall")
  add_compile_options("-pedantic")
  add_compile_options("-Werror=return-type")
 
  if(APPLE)
    # Add required frameworks for GLFW on MacOS.
    target_link_libraries(${CMAKE_PROJECT_NAME} "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
  else()
    # Link the Linux OpenGL library
    target_link_libraries(${CMAKE_PROJECT_NAME} "GL" "dl")
  endif()
else()
  # Link OpenGL on Windows
  target_link_libraries(${CMAKE_PROJECT_NAME} opengl32.lib)
endif()
