// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

/** \cond */
namespace honey { namespace platform
{

#pragma warning(push)
#pragma warning(disable:4250)
template<class Subclass>
class StringStream : public std::basic_ostringstream<Char>
{
    typedef std::basic_ostringstream<Char> Super;

public:
    friend Subclass& operator<<(Subclass& os, std::ios_base& (*manip)(std::ios_base&))              { os.StringStream::Super::operator<<(manip); return os; }
    friend Subclass& operator<<(Subclass& os, basic_ostream::_Myt& (*manip)(basic_ostream::_Myt&))  { os.StringStream::Super::operator<<(manip); return os; }
    //TODO: MSVC crash fix, uncomment
    //template<class T>
    //friend Subclass& operator<<(Subclass& os, const std::_Smanip<T>& manip)                         { std::operator<<(os, manip); return os; }
    //template<class T>
    //friend Subclass& operator<<(Subclass& os, const std::_Fillobj<T>& manip)                        { std::operator<<(os, manip); return os; }
};
#pragma warning(pop)

} }
/** \endcond */

