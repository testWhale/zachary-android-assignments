cmake_minimum_required(VERSION 3.22.1)

project("fan3d")

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT ANDROID)
    message(FATAL_ERROR "This CMakeLists.txt is for Android builds only.")
endif()

add_definitions(-DANDROID -DPLATFORM_ANDROID)

# ---------------------------------------------------------------------------
# GLM - header-only math library used by Scene/Transform.h.
# Fetched independently here because this CMakeLists.txt is a separate NDK
# module build and does not include the top-level cmake/ImportDependencies.cmake.
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glm)

set(SCENE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Scene)

add_library(fan3d SHARED
    NativeTemplate.cpp
    ${SCENE_DIR}/Renderer.cpp
    ${SCENE_DIR}/Transform.cpp
    ${SCENE_DIR}/Fan.cpp
)

target_include_directories(fan3d PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SCENE_DIR}
)

find_library(log-lib     log)
find_library(android-lib android)
find_library(glesv3-lib  GLESv3)
find_library(egl-lib     EGL)

target_link_libraries(fan3d
    ${log-lib}
    ${android-lib}
    ${glesv3-lib}
    ${egl-lib}
    glm::glm
)
