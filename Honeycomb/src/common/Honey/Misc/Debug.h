// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Core/Meta.h"
#include "Honey/Misc/platform/Debug.h"

namespace honey
{

class String;

/// Debug mode functions
/**
  * \see defines assert(), verify(), error()
  */
namespace debug
{
    /// Forwards to assert_\#args. See assert_1(), assert_2().
    #define assert(...)                                 EVAL(TOKCAT(assert_, NUMARGS(__VA_ARGS__))(__VA_ARGS__))
    /// Forwards to verify_\#args. See verify_1(), verify_2().
    #define verify(...)                                 EVAL(TOKCAT(verify_, NUMARGS(__VA_ARGS__))(__VA_ARGS__))

    #ifndef FINAL
        /// Assert that an expression is true, otherwise throws AssertionFailure with the expression. Does nothing in final mode.
        #define assert_1(Expr)                          assert_2(Expr, "")
        /// Assert with extra message to be displayed on failure
        #define assert_2(Expr, Msg)                     if (!(Expr)) { honey::debug::priv::assertFail(#Expr, __FUNC__, __FILE__, __LINE__, (Msg)); }
        /// Similar to assert() but evaluates the expression and throws an error even in final mode
        #define verify_1(Expr)                          assert_1(Expr)
        /// Verify with extra message to be displayed on failure.  Message ignored in final mode.
        #define verify_2(Expr, Msg)                     assert_2(Expr, Msg)
        /// Throw AssertionFailure with a message.  Message ignored in final mode.
        #define error(Msg)                              assert_2(false, Msg)
    #else
        #define assert_1(Expr) {}
        #define assert_2(Expr, Msg) {}
        #define verify_1(Expr)                          verify_2(Expr, "")
        #define verify_2(Expr, Msg)                     if (!(Expr)) { honey::debug::priv::assertFail("", "", "", 0, ""); }
        #define error(Msg)                              verify_2(false, Msg)
    #endif

    #ifndef FINAL
        /// Print string to debug output window.  Does nothing in final mode.
        inline void print(const String& str)            { platform::print(str); }
    #else
        inline void print(const String&) {}
    #endif
    
    /** \cond */
    namespace priv
    {
        inline void assertFail(const char* expr, const char* func, const char* file, int line, const String& msg)
                                                        { platform::assertFail(expr, func, file, line, msg); }
    }
    /** \endcond */
}

}
