# ===========================================================================
# ImportDependencies.cmake
#
# Downloads and configures third-party libraries needed for the desktop build.
# GLM is header-only and needed on every platform, so it is fetched here too;
# the Android build fetches its own copy inside
# android/app/src/main/cpp/CMakeLists.txt (this file isn't included there).
#
# Dependencies:
#   GLFW  - window creation and input   (desktop only)
#   GLEW  - OpenGL extension loader     (desktop only, static build)
#   GLM   - header-only math library used by Scene/Transform.h (all platforms)
# ===========================================================================

include(FetchContent)

function(importDependencies)

    # -----------------------------------------------------------------------
    # GLM - header-only math library
    # -----------------------------------------------------------------------
    message(STATUS "[ImportDependencies] Fetching GLM ...")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(glm)

    if(ANDROID)
        # Android has GLESv3 built-in; nothing else to fetch here.
        return()
    endif()

    # -----------------------------------------------------------------------
    # GLFW - window and input management
    # -----------------------------------------------------------------------
    message(STATUS "[ImportDependencies] Fetching GLFW ...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)

    # -----------------------------------------------------------------------
    # GLEW � OpenGL extension loader (static)
    # -----------------------------------------------------------------------
    message(STATUS "[ImportDependencies] Fetching GLEW ...")
    FetchContent_Declare(
        glew
        GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
    set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(glew-cmake_BUILD_STATIC ON  CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glew)

    message(STATUS "[ImportDependencies] All desktop dependencies ready.")

endfunction()
