#pragma once

/**
 * Model.h
 *
 * Lifecycle contract every renderable shape (Triangle, Square, ...) must
 * follow. Renderer talks to shapes only through this interface, so it
 * never needs to know which concrete shape it's holding.
 *
 * InitModel() / Render() are pure virtual - any subclass that doesn't
 * implement both will fail to compile. Resize() has a default empty body
 * since not every shape cares about screen size.
 */

class Model {
public:
    Model() {}
    virtual ~Model() {}

    virtual void InitModel() = 0;          // called once, before the first frame
    virtual void Render() = 0;             // called once per frame
    virtual void Resize(int w, int h) {}   // called whenever the surface/window resizes
};
