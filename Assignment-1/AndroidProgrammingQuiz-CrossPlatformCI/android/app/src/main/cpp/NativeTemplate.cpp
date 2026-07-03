/**
 * NativeTemplate.cpp
 *
 * Thin JNI bridge between the Java/Android layer and Renderer. All GL
 * logic lives behind the Model interface in Scene/ (Renderer owns and
 * drives whichever Model(s) it constructs) - this file knows nothing
 * about Triangle or any other concrete shape.
 *
 * Native method signatures match MainActivity.java:
 *   package com.example.helloworldandroid
 *   class   MainActivity
 */

#define LOG_TAG "GLPIFrameworkIntroNative"

#include <jni.h>
#include <android/asset_manager_jni.h>
//#include "Platform.h"   // GLES3/gl3.h + LOGI/LOGE - Renderer.h/Model.h don't pull this in
#include "Renderer.h"

// ---------------------------------------------------------------------------
// JNI entry points
// ---------------------------------------------------------------------------

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeInit(JNIEnv* env, jobject /*thiz*/, jobject assetManager)
{
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    Renderer::Instance().setAssetManager(mgr);
    return Renderer::Instance().initializeRenderer() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeResize(
        JNIEnv* /*env*/, jobject /*thiz*/, jint width, jint height)
{
    Renderer::Instance().resize(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeRender(
        JNIEnv* /*env*/, jobject /*thiz*/)
{
    Renderer::Instance().render();
}
