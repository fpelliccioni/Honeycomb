// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Core/Meta.h"
#include "Honey/Misc/platform/Debug.h"

namespace honey
{

class String;

/// Forwards to assert_\#args. See assert_1(), assert_2().
#define assert(...)                                     EVAL(TOKENIZE_EVAL(assert_, NUMARGS(__VA_ARGS__))(__VA_ARGS__))
/// Forwards to verify_\#args. See verify_1(), verify_2().
#define verify(...)                                     EVAL(TOKENIZE_EVAL(verify_, NUMARGS(__VA_ARGS__))(__VA_ARGS__))

#ifndef FINAL
    /// Assert that an expression is true. Does not evaluate expression in final mode. On failure expression is displayed and the program is halted.
    #define assert_1(Expr)                              if (!(Expr)) { honey::Debug::assertPrint(#Expr, __FILE__, __LINE__, String()); }
    /// Assert with extra message to be displayed on failure
    #define assert_2(Expr, Msg)                         if (!(Expr)) { honey::Debug::assertPrint(#Expr, __FILE__, __LINE__, (Msg)); }
    /// Similar to assert() but the expression is evaluated even in final mode
    #define verify_1(Expr)                              assert_1(Expr)
    /// Verify with extra message to be displayed on failure
    #define verify_2(Expr, Msg)                         assert_2(Expr, Msg)
    /// Display an error message
    #define error(Msg)                                  honey::Debug::assertPrint("Error", __FILE__, __LINE__, (Msg))
#else
    #define assert_1(Expr)                              {}
    #define assert_2(Expr, Msg)                         {}
    #define verify_1(Expr)                              if (!(Expr)) {}
    #define verify_2(Expr, Msg)                         if (!(Expr)) {}
    #define error(Msg)                                  {}
#endif

/// Debug mode functions.
/**
  * \see defines assert(), error()
  * \see class Release
  */
class Debug : private platform::Debug
{
    typedef platform::Debug Super;
public:
    /// Used by Assert macros
    static void assertPrint(const String& expr, const String& file, int line, const String& msg)    { Super::assertPrint(expr, file, line, msg); }

    #ifndef FINAL
        /// Print string to debug output window
        static void print(const String& str)            { Super::print(str); }
    #else
        static void print(const String&)                {}
    #endif
};


}
