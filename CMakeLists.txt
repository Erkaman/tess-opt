# CMake entry point
cmake_minimum_required (VERSION 3.1)
project (tess_opt)

find_package(OpenGL REQUIRED)

# get rid of annoying MSVC warnings.
add_definitions(-D_CRT_SECURE_NO_WARNINGS)


# stop it from whining about LOCATION.
cmake_policy(SET CMP0026 OLD)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/deps/cmake")
include(CreateLaunchers)


# Compile external dependencies
add_subdirectory (deps)

set (CMAKE_CXX_STANDARD 11)


include_directories(
	deps/glfw-3.2/include/GLFW/
	deps/glm-0.9.7.5/
	deps/glad/include
	deps/tiny_obj_loader
	deps/imgui/include

)


if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(OPENGL_LIBRARY
	${OPENGL_LIBRARY}
	-lGL -lGLU -lXrandr -lXext -lX11 -lrt
	${CMAKE_DL_LIBS}
	${GLFW_LIBRARIES}
)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(OPENGL_LIBRARY
	${OPENGL_LIBRARY}
	${CMAKE_DL_LIBS}
	${GLFW_LIBRARIES}
)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
)

add_executable(tess_opt
  src/main.cpp
  src/imgui_impl_glfw_gl3.cpp

  deps/glad/src/glad.c

  deps/imgui/src/imgui.cpp
  deps/imgui/src/imgui_draw.cpp




	)

target_link_libraries(tess_opt
	${ALL_LIBS}
)

create_target_launcher(tess_opt WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/")
