// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Core/Meta.h"

namespace honey
{

/// Run a function at scope exit. See ScopeGuard() to create.
template<class F>
class ScopeGuard_ : mt::NoCopy
{
public:
    ScopeGuard_(F&& func)               : _func(forward<F>(func)), _engaged(true) {}
    ~ScopeGuard_()                      { if (_engaged) _func(); }
    /// Disengage the guard so the function isn't run at scope exit
    void release()                      { _engaged = false; }
private:
    F _func;
    bool _engaged;
};

/// Create a scope guard using type deduction. Call with lambda: `auto guard = ScopeGuard([] {...});`
/** \relates ScopeGuard_ */
template<class F>
ScopeGuard_<F> ScopeGuard(F&& func)     { return ScopeGuard_<F>(forward<F>(func)); }

}
