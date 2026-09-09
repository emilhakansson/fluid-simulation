cmake_minimum_required (VERSION 4.1)
set(CMAKE_CXX_STANDARD 17)

project("fluidsim")
add_executable(fluidsim "src/fluid-sim.cpp")

#--- GLFW ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

#--- glad ---
add_library(glad STATIC vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include/)

#--- glm ---
add_subdirectory(vendor/glm)

#--- imgui ---
add_library(imgui STATIC
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp

    vendor/imgui/backends/imgui_impl_glfw.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui
	PUBLIC
	vendor/imgui
	vendor/imgui/backends
)

target_link_libraries(imgui
	PUBLIC
	glfw
)

target_include_directories(fluidsim
	PRIVATE
	glfw
	glad
	glm
)

target_link_libraries(imgui
	PUBLIC
	opengl32
)

target_link_libraries(fluidsim
	PRIVATE
	imgui
	glfw
	glad
	glm::glm
	opengl32
)
