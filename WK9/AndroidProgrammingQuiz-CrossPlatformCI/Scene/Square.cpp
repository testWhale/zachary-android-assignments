#define LOG_TAG "Square"

#include "Square.h"
#include "ShaderHelper.h"

// ---------------------------------------------------------------------------
// Vertex data - a quad offset to the upper-right in clip space so it does
// not overlap the triangle, which is centered at the origin. Drawn as a
// GL_TRIANGLE_FAN (4 vertices, no index buffer needed for a convex quad).
// ---------------------------------------------------------------------------

static const GLfloat kPositions[] = {
    0.35f, 0.60f,   // top-left
    0.85f, 0.60f,   // top-right
    0.85f, 0.10f,   // bottom-right
    0.35f, 0.10f    // bottom-left
};

// Solid yellow. One RGB triplet per vertex - matches the vec3-sized data
// the shared shader pair already expects (see Triangle.cpp).
static const GLfloat kColors[] = {
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f
};

// ---------------------------------------------------------------------------
// Square
// ---------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID
Square::Square(AAssetManager* assetMgr) : mgr(assetMgr) {}
#else
Square::Square() = default;
#endif

Square::~Square()
{
    glDeleteBuffers(1, &vboColor);
    glDeleteBuffers(1, &vboPos);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(programID);
}

void Square::InitModel()
{
    LOGI("Square::InitModel");

    // Reuses the same .glsl source as Triangle, but builds its own program
    // object - Square never shares a program/VAO/VBO with Triangle.
#ifdef PLATFORM_ANDROID
    programID = ShaderHelper::buildProgramFromAssets(mgr, "shader/BlueTriangleVertex.glsl", "shader/BlueTriangleFragment.glsl");
#else
    programID = ShaderHelper::buildProgramFromFile("BlueTriangleVertex.glsl", "BlueTriangleFragment.glsl");
#endif
    if (!programID) { LOGE("Square: could not create program"); return; }

    uRadianAngle = glGetUniformLocation(programID, "RadianAngle");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kColors), kColors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LOGI("Square::InitModel done");
}

void Square::Render()
{
    // Triangle::Render() (drawn first by Renderer) already cleared the
    // frame, so Square only needs to draw on top of it.
    glUseProgram(programID);
    glUniform1f(uRadianAngle, 0.0f);   // static - the square does not rotate

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void Square::Resize(int w, int h)
{
    // Viewport is shared and already set by Triangle::Resize(); nothing
    // shape-specific to do here.
}
