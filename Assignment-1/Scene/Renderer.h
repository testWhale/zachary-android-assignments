#pragma once

/**
 * Renderer.h
 *
 * Singleton controller and the only thing main.cpp / NativeTemplate.cpp
 * talk to. It owns the scene's Model(s) through the Model interface and
 * drives their lifecycle (init once, resize/render every frame).
 *
 * Deliberately does NOT #include "Triangle.h" or "Square.h" - this header
 * only knows about Model*. Concrete shapes are named only inside
 * Renderer.cpp's createModels(), which is the one place allowed to know
 * which shapes the scene is built from.
 */

#include "Model.h"

#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Renderer {
public:
    static Renderer& Instance();

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();

private:
    Renderer() = default;
    ~Renderer();

    void createModels();       // constructs every Model the scene needs
    void initializeModels();   // calls InitModel() on each
    void clearModels();        // deletes and empties models

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    std::vector<Model*> models;   // currently: one Triangle + one Square
};
