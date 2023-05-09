# CMakeList.txt : CMake project for vulkan_tutorial, include source and define
# project specific logic here.
#

# Add source to this project's executable.
find_package(glm REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(Vulkan REQUIRED)
add_executable (vulkan_tutorial "vulkan_tutorial.cpp" "vulkan_tutorial.h")
target_link_libraries(vulkan_tutorial glfw ${GLFW_LIBRARIES} Vulkan::Vulkan )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET vulkan_tutorial PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
