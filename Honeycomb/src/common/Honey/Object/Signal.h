// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/String/Id.h"
#include "Honey/Misc/StdUtil.h"
#include "Honey/Memory/SmallAllocator.h"

namespace honey
{

/// Methods to create and operate on signals and listeners
/**
  * \defgroup Signal Signals and Listeners
  *
  * Signals and listeners provide a way for one function call to be received by multiple callees.
  */
/// @{

/// Call once inside a class that has signals
#define SIGNAL_DECL(BaseClass) \
    IdStatic(_signalBase, #BaseClass)

/// Call inside a class to declare a signal
#define SIGNAL(Name, Param) \
    struct Name                                 : Signal<void Param> { IdStatic(id, StringStream() << _signalBase() << "::"#Name) };

/// Multicast sender
template<class Sig> struct Signal               : mt::funcTraits<Sig> { typedef Sig Sig; };

/// Multicast receiver
class SlotBase : public SmallAllocatorObject
{
public:
    SlotBase(const Id& id, const Id& signalId)  : _id(id), _signalId(signalId) {}
    virtual ~SlotBase()                         {}

    const Id& id() const                        { return _id; }
    const Id& signalId() const                  { return _signalId; }
private:
    Id _id;
    Id _signalId;
};

/** \cond */
namespace priv
{
    #define SLOT_ARG_MAX 5

    template<class Signal, int Arity> struct SlotSignal;
  
    #define SLOT_SIGNAL_PARAM(It)               COMMA_IFNOT(It,1) typename Signal::template param<It-1>::Type const& a##It

    #define CLASS(It)                                                                                   \
        template<class Signal> struct SlotSignal<Signal,It> : SlotBase                                  \
        {                                                                                               \
            SlotSignal(const Id& id)            : SlotBase(id, Signal::id()) {}                         \
            virtual void operator()(ITERATE_(1,It,SLOT_SIGNAL_PARAM)) = 0;                              \
        };                                                                                              \

    ITERATE(0, SLOT_ARG_MAX, CLASS);
    #undef CLASS


    template<class Signal, int Arity, class F> struct Slot;

    #define SLOT_ARG(It)                        COMMA_IFNOT(It,1) a##It

    #define CLASS(It)                                                                                   \
        template<class Signal, class F> class Slot<Signal,It,F> : public SlotSignal<Signal,It>          \
        {                                                                                               \
        public:                                                                                         \
            Slot(const Id& id, F&& f)           : SlotSignal(id), _f(forward<F>(f)) {}                  \
                                                                                                        \
            virtual void operator()(ITERATE_(1,It,SLOT_SIGNAL_PARAM))                                   \
            {                                                                                           \
                _f(ITERATE_(1,It,SLOT_ARG));                                                            \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            typename mt::removeConstRef<F>::Type _f; /* F is forwarded, remove any c/r for storage */   \
        };                                                                                              \

    ITERATE(0, SLOT_ARG_MAX, CLASS);
    #undef CLASS
}
/** \endcond */

/// @}

}

