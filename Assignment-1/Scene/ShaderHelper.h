#pragma once

/**
 * ShaderHelper.h
 *
 * One place for every Model's shader compile/link logic to live, so no
 * shape has to reimplement compileShader/linkProgram itself.
 *
 * Two ways to get shader source into the same compile/link core:
 *   1. "stringify"  - buildProgram(vertSrc, fragSrc) compiles raw source
 *      strings directly. This is what Triangle.cpp used to do inline;
 *      it's kept here for any future Model that prefers embedding its
 *      shader text in code instead of shipping .glsl files.
 *   2. file-based    - buildProgramFromAssets() (Android, reads out of the
 *      APK via AAssetManager) / buildProgramFromFile() (Desktop/Web, reads
 *      off disk) load the .glsl source into a std::string and then funnel
 *      into the exact same buildProgram() underneath - no duplicated
 *      compile/link logic between the two loading paths.
 */

#include "Platform.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class ShaderHelper {
public:
    // --- Core compile/link, shared by both loading methods below ---
    static GLuint compileShader(GLenum type, const char* src);
    static GLuint linkProgram(GLuint vert, GLuint frag);

    // --- Method 1: "stringify" - build directly from inline source strings ---
    static GLuint buildProgram(const char* vertSrc, const char* fragSrc);

    // --- Method 2: file-based - load shader source from disk / APK assets ---
#ifdef PLATFORM_ANDROID
    static std::string loadAsset(AAssetManager* mgr, const char* path);
    static GLuint buildProgramFromAssets(AAssetManager* mgr, const char* vertPath, const char* fragPath);
#else
    static std::string loadFile(const char* filename);
    static GLuint buildProgramFromFile(const char* vertFile, const char* fragFile);
#endif
};

// ---------------------------------------------------------------------------
// Core compile/link
// ---------------------------------------------------------------------------

inline GLuint ShaderHelper::compileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    if (!shader) { LOGE("glCreateShader failed (type=0x%x)", type); return 0; }
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::string log(static_cast<size_t>(len), '\0');
        glGetShaderInfoLog(shader, len, nullptr, &log[0]);
        LOGE("Shader compile error:\n%s", log.c_str());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

inline GLuint ShaderHelper::linkProgram(GLuint vert, GLuint frag)
{
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    GLint ok = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        std::string log(static_cast<size_t>(len), '\0');
        glGetProgramInfoLog(prog, len, nullptr, &log[0]);
        LOGE("Program link error:\n%s", log.c_str());
        glDeleteProgram(prog);
        prog = 0;
    }
    glDetachShader(prog, vert);
    glDetachShader(prog, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);
    return prog;
}

// ---------------------------------------------------------------------------
// Method 1: stringify
// ---------------------------------------------------------------------------

inline GLuint ShaderHelper::buildProgram(const char* vertSrc, const char* fragSrc)
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc);
    if (!vs) return 0;
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!fs) { glDeleteShader(vs); return 0; }
    return linkProgram(vs, fs);
}

// ---------------------------------------------------------------------------
// Method 2: file-based
// ---------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID

inline std::string ShaderHelper::loadAsset(AAssetManager* mgr, const char* path)
{
    if (!mgr) { LOGE("loadAsset: null AAssetManager (path=%s)", path); return std::string(); }

    AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);
    if (!asset) { LOGE("loadAsset: could not open asset '%s'", path); return std::string(); }

    off_t size = AAsset_getLength(asset);
    std::string contents(static_cast<size_t>(size), '\0');
    AAsset_read(asset, &contents[0], static_cast<size_t>(size));
    AAsset_close(asset);
    return contents;
}

inline GLuint ShaderHelper::buildProgramFromAssets(AAssetManager* mgr, const char* vertPath, const char* fragPath)
{
    std::string vertSrc = loadAsset(mgr, vertPath);
    std::string fragSrc = loadAsset(mgr, fragPath);
    if (vertSrc.empty() || fragSrc.empty()) {
        LOGE("buildProgramFromAssets: failed to load '%s' / '%s'", vertPath, fragPath);
        return 0;
    }
    return buildProgram(vertSrc.c_str(), fragSrc.c_str());
}

#else // Desktop / Web

inline std::string ShaderHelper::loadFile(const char* filename)
{
    // Try these relative paths, in this order, returning the first that opens.
    // assets/shader/<filename> is checked first because that's where the
    // post-build copy step (Desktop) / --preload-file mapping (Web) actually
    // places the files, so the common case resolves on the first attempt.
    std::vector<std::string> candidates = {
        std::string("assets/shader/") + filename,
        std::string(filename),
        std::string("shader/") + filename,
        std::string("assets/") + filename
    };

    for (const auto& path : candidates) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (file.is_open()) {
            std::ostringstream ss;
            ss << file.rdbuf();
            return ss.str();
        }
    }

    LOGE("loadFile: could not open '%s' via any candidate path", filename);
    return std::string();
}

inline GLuint ShaderHelper::buildProgramFromFile(const char* vertFile, const char* fragFile)
{
    std::string vertSrc = loadFile(vertFile);
    std::string fragSrc = loadFile(fragFile);
    if (vertSrc.empty() || fragSrc.empty()) {
        LOGE("buildProgramFromFile: failed to load '%s' / '%s'", vertFile, fragFile);
        return 0;
    }
    return buildProgram(vertSrc.c_str(), fragSrc.c_str());
}

#endif
