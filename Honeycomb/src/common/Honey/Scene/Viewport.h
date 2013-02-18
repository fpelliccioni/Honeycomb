// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Alge/Alge.h"
#include "Honey/Math/Geom/Rect.h"

namespace honey
{

/// Viewport rectangle for rendering. Viewport space coordinates are (0,0) at top-left, (1,1) at bottom-right.
class Viewport
{
public:
    Viewport(const Rect& rect = Rect(0,0,1,1))                  : _rect(rect) {}

    /// Set viewport rendering area
    void setRect(const Rect& rect)                              { _rect = rect; }
    const Rect& getRect() const                                 { return _rect; }

    Real left() const                                           { return _rect.left(); }
    Real top() const                                            { return _rect.top(); }
    Real width() const                                          { return _rect.width(); }
    Real height() const                                         { return _rect.height(); }

    /// Convert point from screen space to viewport space.  Screen space coords are (0,0) at top-left, (1,1) at bottom-right.
    Vec2 screenToView(const Vec2& screenPoint) const            { return Vec2((screenPoint.x - left()) / width(), (screenPoint.y - top()) / height()); }
    /// Convert point from viewport space to screen space.
    Vec2 viewToScreen(const Vec2& viewPoint) const              { return Vec2(left() + viewPoint.x*width(), top() + viewPoint.y*height()); }

    /// Convert point from clip space to viewport space.  Clip space coords are (-1,-1) at bottom-left, to (1,1) at top-right.
    Vec2 clipToView(const Vec2& clipPoint) const                { return Vec2((clipPoint.x+1)/2, (-clipPoint.y+1)/2); }
    /// Convert point from viewport space to clip space.
    Vec2 viewToClip(const Vec2& viewPoint) const                { return Vec2(viewPoint.x*2 - 1, (1-viewPoint.y)*2 - 1); }
    
private:
    Rect _rect;
};

}
