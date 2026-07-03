#pragma once

/**
 * Square.h
 *
 * Second concrete Model, added alongside Triangle to prove the
 * Model + Renderer architecture scales: Renderer doesn't need to change
 * to add this shape, and Square owns its own program/VAO/VBOs - it never
 * touches Triangle's GPU resources.
 *
 * Same construction pattern as Triangle: on Android the constructor takes
 * the AAssetManager* needed to read .glsl shader source out of the APK;
 * Desktop/Web read the same files straight off disk.
 */

#include "Model.h"
#include "Platform.h"   // GLuint/GLint/GLfloat types used by the members below

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Square : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Square(AAssetManager* assetMgr);
#else
    Square();
#endif
    ~Square() override;

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
};
