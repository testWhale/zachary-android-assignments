#define LOG_TAG "Fan3D"
#include "Fan.h"
#include "ShaderHelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>

// ---------------------------------------------------------------------------
// Geometry - one 8-vertex / 36-index cube, shared by every part.
// Position is layout(location=0), Shade is layout(location=1): a per-vertex
// grayscale factor (front corners brighter than back corners) that the
// fragment shader multiplies against PARTCOLOR. This is what gives every
// part its 3D "shaded" look without needing per-part vertex data.
// ---------------------------------------------------------------------------
static const GLfloat kPositions[8][3] = {
    {-1.0f, -1.0f,  1.0f},  // V0 front bottom-left
    {-1.0f,  1.0f,  1.0f},  // V1 front top-left
    { 1.0f,  1.0f,  1.0f},  // V2 front top-right
    { 1.0f, -1.0f,  1.0f},  // V3 front bottom-right
    {-1.0f, -1.0f, -1.0f},  // V4 back bottom-left
    {-1.0f,  1.0f, -1.0f},  // V5 back top-left
    { 1.0f,  1.0f, -1.0f},  // V6 back top-right
    { 1.0f, -1.0f, -1.0f},  // V7 back bottom-right
};

static const GLfloat kShades[8] = {
    1.0f, 1.0f, 1.0f, 1.0f,   // front corners
    0.6f, 0.6f, 0.6f, 0.6f,   // back corners
};

static const GLushort kIndices[36] = {
    0,3,1, 3,2,1,
    7,4,6, 4,5,6,
    4,0,5, 0,1,5,
    3,7,2, 7,6,2,
    1,2,5, 2,6,5,
    3,0,7, 0,4,7
};

static constexpr GLuint ATTRIB_POSITION = 0;
static constexpr GLuint ATTRIB_SHADE    = 1;

static constexpr size_t kPosSize   = sizeof(kPositions);
static constexpr size_t kShadeSize = sizeof(kShades);

// ---- Per-part colours (see assignment brief, Part 1 step 4) ---------------
static const glm::vec3 kColorBase  (0.45f, 0.28f, 0.12f);  // brown
static const glm::vec3 kColorPole  (0.55f, 0.55f, 0.58f);  // light gray
static const glm::vec3 kColorHub   (0.20f, 0.20f, 0.22f);  // dark gray
// Sized for the max runtime blade count (stretch goal: 3/4/5 blades).
static const glm::vec3 kColorBlade[5] = {
    glm::vec3(0.85f, 0.12f, 0.12f),  // blade 0: red
    glm::vec3(0.15f, 0.35f, 0.85f),  // blade 1: blue
    glm::vec3(0.90f, 0.55f, 0.08f),  // blade 2: orange
    glm::vec3(0.18f, 0.70f, 0.22f),  // blade 3: green
    glm::vec3(0.55f, 0.20f, 0.75f),  // blade 4: purple
};

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
#ifdef PLATFORM_ANDROID
Fan::Fan(AAssetManager* assetMgr) : mgr(assetMgr) {
#else
Fan::Fan() {
#endif
    modelType = FanType;
}

Fan::~Fan()
{
    if (vao)     { glDeleteVertexArrays(1, &vao); vao     = 0; }
    if (vbo)     { glDeleteBuffers(1, &vbo);       vbo     = 0; }
    if (ibo)     { glDeleteBuffers(1, &ibo);       ibo     = 0; }
    if (program) { glDeleteProgram(program);       program = 0; }
}

// ---------------------------------------------------------------------------
// InitModel - compile shaders, upload the shared cube once, build the VAO,
// and reset the matrix stack / GL state via Transform::TransformInit().
// ---------------------------------------------------------------------------
void Fan::InitModel()
{
    LOGI("Fan::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        mgr,
        "shader/FanVertex.glsl",
        "shader/FanFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "FanVertex.glsl",
        "FanFragment.glsl");
#endif

    if (!program) { LOGE("Fan: failed to build shader program"); return; }

    uMVP       = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uPartColor = glGetUniformLocation(program, "PARTCOLOR");

    // VBO: positions sub-region then per-corner shade sub-region.
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, kPosSize + kShadeSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,        kPosSize,   kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, kPosSize, kShadeSize, kShades);

    // IBO - the same 36 indices are reused for every one of the seven draws.
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    // VAO captures the attribute layout + IBO binding.
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glEnableVertexAttribArray(ATTRIB_SHADE);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(ATTRIB_SHADE,    1, GL_FLOAT, GL_FALSE, 0, (void*)kPosSize);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Resets all four Transform stacks to identity and sets the base GL
    // state (GL_DEPTH_TEST, culling, etc.) - see Transform.cpp.
    transform.TransformInit();

    // Stretch goal - oscillation: t=0 for the sway starts now, so it eases
    // in from centre instead of jumping to wherever sin() would be at an
    // arbitrary clock time.
    startTime = std::chrono::steady_clock::now();

    LOGI("Fan::InitModel done (VAO=%u, VBO=%u, IBO=%u)", vao, vbo, ibo);
}

// ---------------------------------------------------------------------------
// Resize - projection only; the view stack stays identity.
// ---------------------------------------------------------------------------
void Fan::Resize(int w, int h)
{
    float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f, 0.0f);
}

// ---------------------------------------------------------------------------
// drawPart - upload this part's MVP + colour, draw the shared cube.
// ---------------------------------------------------------------------------
void Fan::drawPart(const glm::vec3& color)
{
    glm::mat4* mvp = transform.TransformGetModelViewProjectionMatrix();
    if (uMVP >= 0)
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(*mvp));
    if (uPartColor >= 0)
        glUniform3fv(uPartColor, 1, glm::value_ptr(color));

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// Render - Part 2 (spin) folded in: advance spinAngle, then draw all seven
// parts from the shared world matrix using push/pop.
// ---------------------------------------------------------------------------
void Fan::Render()
{
    if (!program || !vao) return;

    // Part 2: advance the blade angle only while the fan is on.
    if (fanOn) {
        spinAngle += kBaseSpeed + dragBoost;
        if (spinAngle >= 360.0f) spinAngle -= 360.0f;
    }

    // Stretch goal - fling with decay: dragBoost is no longer zeroed on
    // release. It winds down a little every frame instead, so a hard flick
    // coasts down gradually rather than snapping back to base speed.
    if (dragBoost > 0.0f) {
        dragBoost *= kDecay;
        if (dragBoost < 0.01f) dragBoost = 0.0f;
    }

    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // World: shared pull-back + 3D tilt, every part starts from here.
    transform.TransformTranslate(0.0f, 0.8f, -8.0f);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    // ---- base ----
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -2.6f, 0.0f);
        transform.TransformScale(1.6f, 0.25f, 0.8f);
        drawPart(kColorBase);
    transform.TransformPopMatrix();

    // ---- pole ----
    // Spans from just inside the base top (-2.35) up to just inside the
    // hub bottom (0.95) - embeds ~0.05 into each so there's no visible gap
    // and, critically, no stub poking out below the base or above the hub.
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -0.7f, 0.0f);
        transform.TransformScale(0.15f, 1.7f, 0.15f);
        drawPart(kColorPole);
    transform.TransformPopMatrix();

    // ---- oscillation (stretch goal) -----------------------------------
    // Sway the whole head (hub + blades) left-right around a vertical axis
    // through the hub. Translate to the pivot, rotate, translate back -
    // base and pole are outside this push/pop so they never move, only the
    // head does. Independent of fanOn, same as a real fan's separate
    // oscillation motor.
    float elapsedSec = std::chrono::duration<float>(
        std::chrono::steady_clock::now() - startTime).count();
    float oscAngle = kOscAmplitudeDeg * std::sin(elapsedSec * kOscSpeed);

    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 1.25f, 0.0f);
        transform.TransformRotate(glm::radians(oscAngle), 0.0f, 1.0f, 0.0f);
        transform.TransformTranslate(0.0f, -1.25f, 0.0f);

        // ---- hub ----
        // Sits at the top of the pole (pole top ~1.0, hub half-height 0.3).
        transform.TransformPushMatrix();
            transform.TransformTranslate(0.0f, 1.25f, 0.0f);
            transform.TransformScale(0.3f, 0.3f, 0.3f);
            drawPart(kColorHub);
        transform.TransformPopMatrix();

        // ---- blades, evenly spaced around Z (stretch goal: bladeCount is
        // runtime-configurable, 3/4/5 - see SetBladeCount()) ----
        // Pivot y matches the hub's y so blades attach at the fan head, not
        // partway down the pole. Pivot z=0.4 clears the hub's front face
        // (hub is scaled 0.3, so its face sits at z=0.3), so blades hover
        // just in front of it instead of sitting inside the hub and
        // getting depth-tested away. The radial offset (1.1) minus the
        // blade half length (0.8) leaves the near edge sitting exactly at
        // the hub's edge (0.3) - the hub's own size is the padding, so
        // blades don't overlap each other or bury themselves inside the hub.
        const float bladeSpacing = 360.0f / (float)bladeCount;
        for (int i = 0; i < bladeCount; ++i) {
            transform.TransformPushMatrix();
                transform.TransformTranslate(0.0f, 1.25f, 0.4f);
                transform.TransformRotate(glm::radians(spinAngle + i * bladeSpacing), 0.0f, 0.0f, 1.0f);
                transform.TransformTranslate(0.0f, 1.1f, 0.0f);
                transform.TransformScale(0.22f, 0.8f, 0.05f);
                drawPart(kColorBlade[i]);
            transform.TransformPopMatrix();
        }
    transform.TransformPopMatrix();
}

// ---------------------------------------------------------------------------
// Part 3 / Part 4 - touch (down / move / release) drives tap-vs-swipe.
// ---------------------------------------------------------------------------
void Fan::TouchEventDown(float x, float y)
{
    lastX = x;
    lastY = y;
    movedDistance = 0.0f;
    lastMoveTime  = std::chrono::steady_clock::now();
}

void Fan::TouchEventMove(float x, float y)
{
    float dx = x - lastX;
    float dy = y - lastY;
    float dist = std::sqrt(dx * dx + dy * dy);
    movedDistance += dist;

    auto now = std::chrono::steady_clock::now();
    float dtMs = std::chrono::duration<float, std::milli>(now - lastMoveTime).count();

    if (dtMs > 0.0f) {
        float velocity = dist / dtMs;  // pixels per millisecond
        dragBoost = std::min(velocity * kBoostScale, kMaxBoost);
        LOGI("Fan: drag velocity=%.3f px/ms -> boost=%.2f deg/frame", velocity, dragBoost);
    }

    lastX = x;
    lastY = y;
    lastMoveTime = now;
}

void Fan::TouchEventRelease(float /*x*/, float /*y*/)
{
    if (movedDistance < kTapThreshold) {
        fanOn = !fanOn;
        LOGI("Fan %s", fanOn ? "ON" : "OFF");
    }
    // Stretch goal - fling with decay: dragBoost is deliberately NOT reset
    // to 0 here anymore. Render() winds it down by kDecay every frame
    // instead, so a hard flick keeps spinning the fan faster for a bit
    // after you let go, like a real fan coasting down.
}

// ---------------------------------------------------------------------------
// Stretch goal - runtime blade count.
// ---------------------------------------------------------------------------
void Fan::SetBladeCount(int count)
{
    int clamped = std::max(kMinBlades, std::min(count, kMaxBlades));
    if (clamped == bladeCount) return;
    bladeCount = clamped;
    LOGI("Fan: bladeCount = %d", bladeCount);
}
