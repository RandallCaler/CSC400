cmake_minimum_required(VERSION 3.4...3.28.1)
include("${CMAKE_SOURCE_DIR}/FindGfxLibs.cmake")

# Name of the project
project(Penguin)

# Enforce use of C++14
set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_STANDARD 17)

# set(FREETYPE_LIBRARY "C:/Users/thejo/OneDrive/Desktop/freetype-2.13.2/build/Debug/freetyped.lib")  # Adjust to your Freetype library path
# set(FREETYPE_INCLUDE_DIRS "C:/Users/thejo/OneDrive/Desktop/freetype-2.13.2/build/include")

# Use glob to get the list of all source files.
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/ext/*/*.cpp" "${CMAKE_SOURCE_DIR}/ext/glad/src/*.c")

# We don't really need to include header and resource files to build, but it's
# nice to have them show up in IDEs.
file(GLOB_RECURSE HEADERS "src/*.h" "ext/*/*.h" "ext/glad/*/*.h")
file(GLOB_RECURSE GLSL "resources/*.glsl")

# Include directories
include_directories("${CMAKE_SOURCE_DIR}/ext")
include_directories("${CMAKE_SOURCE_DIR}/ext/glad/include")
# Assuming Assimp headers are within the ext/assimp/include directory
include_directories("${CMAKE_SOURCE_DIR}/ext/assimp/include")
include_directories("${CMAKE_SOURCE_DIR}/ext/imgui/include")
include_directories("${CMAKE_SOURCE_DIR}/ext/json/include")
include_directories("${CMAKE_SOURCE_DIR}/ext/includes") # freetype directory


if(EXISTS "${CMAKE_SOURCE_DIR}/config.cmake")
    include("${CMAKE_SOURCE_DIR}/config.cmake")
else()
    message(FATAL_ERROR "Configuration file not found. Please create a config.cmake file in the root directory.")
endif()

# Link FREETYPE libraries
# add_subdirectory(ext/freetype)
include_directories(${FREETYPE_INCLUDE_DIRS})
find_package(Freetype REQUIRED)


# target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${FREETYPE_LIBRARIES})

# Set the executable.
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})

# Helper function included from FindGfxLibs.cmake
findGLFW3(${CMAKE_PROJECT_NAME})
findGLM(${CMAKE_PROJECT_NAME})
findASSIMP(${CMAKE_PROJECT_NAME})



# OS specific options and libraries
if(NOT WIN32)

  message(STATUS "Adding GCC style compiler flags")
  add_compile_options("-Wall")
  add_compile_options("-pedantic")
  add_compile_options("-Werror=return-type")
 
  # TODO: The following links may be uneeded. 
  if(APPLE)
    # Add required frameworks for GLFW.
    target_link_libraries(${CMAKE_PROJECT_NAME} ${FREETYPE_LIBRARIES} "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
  else()
    #Link the Linux OpenGL library
    target_link_libraries(${CMAKE_PROJECT_NAME} "GL" "dl")
  endif()

else()

  # Link OpenGL on Windows
  target_link_libraries(${CMAKE_PROJECT_NAME} opengl32.lib ${FREETYPE_LIBRARIES})

endif()