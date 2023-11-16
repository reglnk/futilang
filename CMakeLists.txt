cmake_minimum_required(VERSION 3.17)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

project(sampl VERSION 0.1.0 LANGUAGES C CXX)
add_executable(sampl src/main.cpp glad/src/gl.c)

# GLAD configuration ==================
include_directories(glad/include)

# GLFW configuration ==================
set(EXT_GLFW_INCLUDE_DIR "" CACHE PATH "")
set(EXT_GLFW_LIBRARY_PATH "" CACHE FILEPATH "Import library file")

if (EXT_GLFW_INCLUDE_DIR AND EXT_GLFW_LIBRARY_PATH)
	message(STATUS "Using GLFW with provided paths")
	include_directories("${EXT_GLFW_INCLUDE_DIR}")
	target_link_libraries(sampl "${EXT_GLFW_LIBRARY_PATH}")
	unset(glfw3_DIR CACHE)
else()
	find_package(glfw3 3.3 REQUIRED)
	message(STATUS "Found glfw3 package")
	# GLFW includes system directories itself
	target_link_libraries(sampl glfw)
endif()
