#pragma once

/**
 * Fan.h
 *
 * Assignment 2 - 3D Fan.
 *
 * The whole fan (base, pole, hub, four blades - seven parts total) is drawn
 * from a single shared cube VAO/VBO/IBO. Each part gets its own model matrix
 * composed on the provided Transform push/pop stack (Transform.h/.cpp,
 * unmodified) and its own colour uploaded through the PARTCOLOR uniform, one
 * glUniform3fv call per draw. There is no per-part vertex data - only the
 * matrix and the colour change between the seven glDrawElements calls.
 *
 * Part hierarchy (Render()):
 *
 *   world  : shared pull-back + tilt (Translate, then Rotate 20 deg on Y)
 *   base   : push -> translate -> scale -> draw(brown)      -> pop
 *   pole   : push -> translate -> scale -> draw(light gray) -> pop
 *   hub    : push -> translate -> scale -> draw(dark gray)  -> pop
 *   blade0..N-1 : push -> translate to hub -> rotate(spinAngle + i*360/N) on Z
 *                 -> translate outward -> scale -> draw(colour[i]) -> pop
 *                 (N = bladeCount, default 4 - see stretch goal below)
 *
 * Gesture behaviour:
 *   TAP (down+release, barely moved) -> toggles fanOn, logs "Fan ON"/"Fan OFF"
 *   DRAG (down, move, release)       -> drag velocity (px/ms, std::chrono)
 *       is converted to dragBoost, clamped to kMaxBoost, and added on top of
 *       kBaseSpeed for as long as the drag continues.
 *
 * Stretch goal - fling with decay: release no longer snaps dragBoost to 0.
 * Instead Render() multiplies it by kDecay (0.98) every frame, so the fan
 * winds down gradually after a flick instead of instantly resuming base
 * speed - the harder the flick, the longer the coast-down.
 *
 * Stretch goal - runtime blade count: SetBladeCount(3/4/5) changes
 * bladeCount and the blades re-space themselves at 360/bladeCount degrees
 * apart. Wired to the 3/4/5 keys on Desktop and Web (see main.cpp).
 *
 * Stretch goal (proposed) - oscillation: the hub + blades sway left-right
 * around a vertical axis through the hub, like a real oscillating table
 * fan. oscAngle = kOscAmplitudeDeg * sin(elapsedSeconds * kOscSpeed), applied
 * as one extra push/rotate/pop wrapped around the existing hub + blade
 * draws - base and pole stay fixed, only the head sways. Runs independently
 * of fanOn (a real fan's oscillation motor is separate from the blade motor).
 *
 * Platform-agnostic - shared by Android, Desktop, and WebGL builds.
 */

#include "Model.h"
#include "Platform.h"
#include "Transform.h"

#include <chrono>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* assetMgr);
#else
    Fan();
#endif
    ~Fan() override;

    void InitModel()        override;
    void Render()            override;
    void Resize(int w, int h) override;

    void TouchEventDown(float x, float y)    override;
    void TouchEventMove(float x, float y)    override;
    void TouchEventRelease(float x, float y) override;

    // Stretch goal: runtime-configurable blade count, clamped to [3, 5].
    void SetBladeCount(int count) override;

private:
    // ---- Draw helper: uploads MVP + PARTCOLOR, then issues the shared draw
    void drawPart(const glm::vec3& color);

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    // ---- GL objects (one shared cube) --------------------------------------
    GLuint program = 0;
    GLuint vao     = 0;
    GLuint vbo     = 0;
    GLuint ibo     = 0;

    GLint uMVP        = -1;
    GLint uPartColor  = -1;

    // ---- Core animation / matrix-stack state (per assignment spec) --------
    Transform transform;                 // provided matrix stack (model/view/projection)
    float spinAngle = 0.0f;              // current blade angle (degrees)
    bool  fanOn     = true;              // toggled by a tap
    float dragBoost = 0.0f;              // extra speed while dragging, 0 otherwise

    static constexpr float kBaseSpeed    = 1.5f;  // deg/frame when ON
    static constexpr float kMaxBoost     = 20.0f; // clamp for the drag boost
    static constexpr float kBoostScale   = 8.0f;  // px/ms -> deg/frame
    static constexpr float kTapThreshold = 12.0f; // px: below this it's a tap
    static constexpr float kDecay        = 0.98f; // stretch: per-frame dragBoost decay after release

    // ---- Touch / gesture bookkeeping ---------------------------------------
    float lastX = 0.0f, lastY = 0.0f;           // previous touch position
    float movedDistance = 0.0f;                 // total movement since touch-down
    std::chrono::steady_clock::time_point lastMoveTime; // for velocity (Part 4)

    // ---- Stretch goal: runtime blade count ---------------------------------
    static constexpr int kMinBlades = 3;
    static constexpr int kMaxBlades = 5;
    int bladeCount = 4;

    // ---- Stretch goal: oscillation ------------------------------------------
    std::chrono::steady_clock::time_point startTime; // t=0 for the sway, set in InitModel()
    static constexpr float kOscAmplitudeDeg = 25.0f;  // max sway either side of centre
    static constexpr float kOscSpeed        = 0.7f;   // radians of sin() phase per second
};
