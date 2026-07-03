#pragma once

/**
 * Triangle.h
 *
 * Concrete Model: a rotating triangle (shader, VAO, VBOs). Owned by
 * Renderer through the Model interface - no other file needs to know
 * this shape is specifically a triangle.
 *
 * Shared by all three targets (Desktop/GLFW, Web/Emscripten, Android).
 * On Android, the constructor takes the AAssetManager* needed to read
 * the .glsl shader source out of the APK; Desktop/Web read the same
 * files straight off disk, so they don't need it.
 */

#include "Model.h"
#include "Platform.h"   // GLuint/GLint/GLfloat types used by the members below

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Triangle : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Triangle(AAssetManager* assetMgr);
#else
    Triangle();
#endif
    ~Triangle() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint programID    = 0;
    GLuint vao          = 0;
    GLuint vboPos       = 0;
    GLuint vboColor     = 0;
    GLint  uRadianAngle = -1;
    float  degree       = 0.0f;
};
