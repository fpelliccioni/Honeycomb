// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include <codecvt>

/** \cond */
namespace honey { namespace platform
{

template<class Subclass>
class StringStream : public std::wstringstream
{
    typedef std::wstringstream Super;

public:
    /// Does nothing if `str` is null
    friend Subclass& operator<<(Subclass& os, const Char* str)
    {
        if (!str) return os;
        std::wstring_convert<std::codecvt_utf8_utf16<Char>, Char> convert;
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convertw;
        std::operator<<(os, convertw.from_bytes(convert.to_bytes(str)));
        return os;
    }
    
    /// Converts C-string to String and writes. Does nothing if `str` is null.
    friend Subclass& operator<<(Subclass& os, const char* str)
    {
        if (!str) return os;
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convertw;
        std::operator<<(os, convertw.from_bytes(str));
        return os;
    }
    
    friend Subclass& operator<<(Subclass& os, std::ios_base& (*manip)(std::ios_base&))
                                                                    { os.std::wstringstream::operator<<(manip); return os; }
    friend Subclass& operator<<(Subclass& os, std::basic_ios<wchar_t>& (*manip)(std::basic_ios<wchar_t>&))
                                                                    { os.std::wstringstream::operator<<(manip); return os; }
    friend Subclass& operator<<(Subclass& os, std::basic_ostream<wchar_t>& (*manip)(std::basic_ostream<wchar_t>&))
                                                                    { os.std::wstringstream::operator<<(manip); return os; }
    friend Subclass& operator<<(Subclass& os, Super& (*manip)(Super&))
                                                                    { os.std::wstringstream::operator<<(manip); return os; }
    /// setw
    friend Subclass& operator<<(Subclass& os, const std::__1::__iom_t6& manip)
                                                                    { static_cast<std::wstringstream&>(os) << manip; return os; }
    /// setFill
    friend Subclass& operator<<(Subclass& os, const std::__1::__iom_t4<wchar_t>& manip)
                                                                    { static_cast<std::wstringstream&>(os) << manip; return os; }
    
    friend Subclass& operator<<(Subclass& os, long val)             { os.std::wstringstream::operator<<(static_cast<int32>(val)); return os; }
    friend Subclass& operator<<(Subclass& os, unsigned long val)    { os.std::wstringstream::operator<<(static_cast<uint32>(val)); return os; }
    
    String str() const
    {
        std::wstring_convert<std::codecvt_utf8_utf16<Char>, Char> convert;
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convertw;
        return convert.from_bytes(convertw.to_bytes(Super::str()));
    }
};

} }
/** \endcond */

