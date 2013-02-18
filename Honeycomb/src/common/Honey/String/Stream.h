// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/String/String.h"
#include "Honey/String/platform/Stream.h"

namespace honey
{
/** \cond */
namespace stringstream { namespace priv
{
    template<class T>
    StringStream& globalIn(StringStream& os, T&& val)                           { return operator<<(os, forward<T>(val)); }
} }
/** \endcond */

#pragma warning(push)
#pragma warning(disable:4250)
/// Class to format a string for output or use as input
class StringStream : public platform::StringStream<StringStream>
{
    typedef platform::StringStream<StringStream> Super;

public:
    /// Stream manipulator
    template<class T>
    struct Manip
    {
        typedef void (*Func)(StringStream&, T);
        
        Manip()                                                                 {}
        Manip(const Func& f, const T& arg)                                      : f(f), arg(arg) {}

        void call(StringStream& stream) const                                   { f(stream, arg); }

        Func f;
        T arg;
    };

    StringStream()                                                              : _indent(0), _indentSize(4) {}
    explicit StringStream(const String& str)                                    : _indent(0), _indentSize(4) { operator<<(str); }

    /// Forward member operators to global
    template<class T>
    StringStream& operator<<(T&& val)                                           { return stringstream::priv::globalIn(*this, forward<T>(val)); }

    #ifdef WIN32
        //TODO: MSVC crash fix, remove block
        template<class T>
        StringStream& operator<<(const std::_Smanip<T>& manip)                  { std::operator<<(*this, manip); return *this; }
        template<class T>
        StringStream& operator<<(std::_Smanip<T>&& manip)                       { std::operator<<(*this, manip); return *this; }
        template<class T>
        StringStream& operator<<(const std::_Fillobj<T>& manip)                 { std::operator<<(*this, manip); return *this; }
        template<class T>
        StringStream& operator<<(std::_Fillobj<T>&& manip)                      { std::operator<<(*this, manip); return *this; }
    #endif

    /// \name Input operators
    /// @{
    friend StringStream& operator<<(StringStream& os, const String& str)        { std::operator<<(os, str); return os; }
    /// Does nothing if `str` is null
    friend StringStream& operator<<(StringStream& os, const Char* str)          { if (!str) return os; std::operator<<(os, str); return os; }
    // MSVC treats Char as integral. Portable method is to append as string.
    friend StringStream& operator<<(StringStream& os, Char val)                 { Char str[] = { val, 0 }; return os << str; }
    /// Converts wide string to String and writes. Does nothing if `str` is null.
    //friend StringStream& operator<<(StringStream& os, const wchar_t* str)       { if (!str) return os; return honey::operator<<(os, String(str)); }
    /// Converts wchar_t to String and writes
    //friend StringStream& operator<<(StringStream& os, wchar_t val)              { wchar_t str[] = { val, 0 }; return honey::operator<<(os, str); }
    /// Converts C-string to String and writes. Does nothing if `str` is null.
    friend StringStream& operator<<(StringStream& os, const char* str)          { if (!str) return os; return os << String(str); }
    /// Converts char to Char and writes
    friend StringStream& operator<<(StringStream& os, char val)                 { return os << static_cast<Char>(val); }
    /// Writes integral value
    friend StringStream& operator<<(StringStream& os, uint8 val)                { os.Super::operator<<(static_cast<int16>(val)); return os; }
    friend StringStream& operator<<(StringStream& os, bool val)                 { os.Super::operator<<(val); return os; }
    friend StringStream& operator<<(StringStream& os, int16 val)                { os.Super::operator<<(val); return os; }
    // MSVC treats Char as non-built-in. Overrides conflict.
    //friend StringStream& operator<<(StringStream& os, uint16 val)               { os.Super::operator<<(val); return os; }
    friend StringStream& operator<<(StringStream& os, int32 val)                { os.Super::operator<<(val); return os; }
    friend StringStream& operator<<(StringStream& os, uint32 val)               { os.Super::operator<<(val); return os; }
    friend StringStream& operator<<(StringStream& os, int64 val)                { os.Super::operator<<(val); return os; }
    friend StringStream& operator<<(StringStream& os, uint64 val)               { os.Super::operator<<(val); return os; }
    friend StringStream& operator<<(StringStream& os, float val)                { os.Super::operator<<(val); return os; }
    friend StringStream& operator<<(StringStream& os, double val)               { os.Super::operator<<(val); return os; }
    friend StringStream& operator<<(StringStream& os, float128 val)             { os.Super::operator<<(val); return os; }

    friend StringStream& operator<<(StringStream& os, StringStream& (*manip)(StringStream&))    { return manip(os); }
    template<class T>
    friend StringStream& operator<<(StringStream& os, const Manip<T>& manip)    { manip.call(os); return os; }
    /// @}

    /// Set indent level
    void setIndent(int level)                                                   { assert(level >= 0); _indent = level; }
    int getIndent() const                                                       { return _indent; }

    /// Set number of spaces per indent level
    void setindentSize(int spaces)                                              { _indentSize = spaces; }
    int getindentSize() const                                                   { return _indentSize; }
    
    /// Get copy of buffer as string
    String str() const                                                          { return Super::str(); } 
    operator String() const                                                     { return str(); }

private:
    int _indent;
    int _indentSize;
};
#pragma warning(pop)

/** \cond */
namespace priv
{
    inline void setindentSize(StringStream& stream, int spaces)                 { stream.setindentSize(spaces); }
}
/** \endcond */

/// Increase stream indent level by 1
/** \relates StringStream */
inline StringStream& indentInc(StringStream& stream)                            { stream.setIndent(stream.getIndent()+1); return stream; }
/// Decrease stream indent level by 1
/** \relates StringStream */
inline StringStream& indentDec(StringStream& stream)                            { stream.setIndent(stream.getIndent()-1); return stream; }
/// Set number of spaces per indent level
/** \relates StringStream */
inline StringStream::Manip<int> indentSize(int spaces)                          { return StringStream::Manip<int>(&priv::setindentSize, spaces); }

/// End line and indent the next line
/** \relates StringStream */
inline StringStream& endl(StringStream& stream)
{
    stream << std::endl;
    for (int i = 0, end = stream.getIndent()*stream.getindentSize(); i < end; ++i)
        stream << ' ';
    return stream;
}

}



