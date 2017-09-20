cmake_minimum_required(VERSION 3.7)
project(raytracing-realtime-approximation)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES src/cpp/App.cpp src/cpp/GLSLProgram.cpp src/cpp/Model.cpp include/Model.h src/cpp/OBJLoader.cpp include/OBJLoader.h include/App.h)
include_directories("include")

find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(glm REQUIRED)
find_package(GLEW REQUIRED)

if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    link_libraries(${GLEW_LIBRARIES})
endif()

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

add_executable(raytracing-realtime-approximation ${SOURCE_FILES})

target_link_libraries(raytracing-realtime-approximation ${OPENGL_LIBRARIES} glfw)
