#!/usr/bin/env sh
# =============================================================================
# build_web.sh – Build Fan3D (Assignment 2) for WebGL using Emscripten
#
# Prerequisites:
#   1. Install the Emscripten SDK:  https://emscripten.org/docs/getting_started/
#   2. Activate it:   source /path/to/emsdk/emsdk_env.sh
#   3. Run from the project root:  ./build_web.sh
#
# Output: web/index.html  (+ index.js, index.wasm, index.data)
# Serve:  cd web && python3 -m http.server 8080
#
# GLM (header-only) is fetched into external/glm on first run since this is a
# raw emcc command line, not a CMake build that could FetchContent it.
# external/ is git-ignored -- never commit it.
#
# Windowing backend (choose one):
#   GLFW  – default, recommended  (-DUSE_GLFW  -s USE_GLFW=3)
#   SDL2  – legacy fallback        (no -DUSE_GLFW, -s USE_SDL=2)
# =============================================================================

set -e

mkdir -p web

if [ ! -d external/glm ]; then
    echo "Fetching GLM ..."
    mkdir -p external
    git clone --depth 1 --branch 1.0.1 https://github.com/g-truc/glm.git external/glm
fi

# --- GLFW backend (default) -------------------------------------------------
emcc -std=c++17 \
    Scene/main.cpp \
    Scene/Renderer.cpp \
    Scene/Transform.cpp \
    Scene/Fan.cpp \
    -IScene \
    -Iexternal/glm \
    -DUSE_GLFW \
    -s USE_GLFW=3 \
    -s USE_WEBGL2=1 \
    -s FULL_ES3=1 \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    --preload-file android/app/src/main/assets/shader@assets/shader \
    -o web/index.html

# --- SDL2 backend (opt-in) --------------------------------------------------
# To use SDL2 instead, comment out the GLFW block above and uncomment below:
# emcc -std=c++17 \
#     Scene/main.cpp \
#     Scene/Renderer.cpp \
#     Scene/Transform.cpp \
#     Scene/Fan.cpp \
#     -IScene \
#     -Iexternal/glm \
#     -s USE_SDL=2 \
#     -s USE_WEBGL2=1 -s FULL_ES3=1 \
#     -s WASM=1 \
#     -s ALLOW_MEMORY_GROWTH=1 \
#     --preload-file android/app/src/main/assets/shader@assets/shader \
#     -o web/index.html

echo ""
echo "Build succeeded."
echo "Run:  cd web && python3 -m http.server 8080"
echo "Open: http://localhost:8080/"
