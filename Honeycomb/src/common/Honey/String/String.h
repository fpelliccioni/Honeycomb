// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/Debug.h"

namespace honey
{
    
/// Represents a single code unit (not code point) for class String
typedef char16_t Char;

/// Unicode UTF-16 string class, wrapper around std::u16string
/**
  * length() returns the number of UTF-16 code units, not code points (characters or visual symbols).
  * Some code points are 32-bit and thus are composed of 2 code units, called a surrogate pair or high / low surrogates.
  *
  * Modeling the string as an array of fixed-length UTF-16 code units rather than variable-length code points
  * is a trade-off for efficiency, as a large range of common characters can be represented by a single code unit.
  */
class String : public std::basic_string<Char>
{
    typedef std::basic_string<Char> Super;
    friend class StringStream;

public:
    /// List of strings
    typedef vector<String> List;

    String() {}
    String(const std::basic_string<Char>& str)                              : Super(str) {}
    /// Does nothing if `str` is null
    String(const Char* str)                                                 { operator=(str); }
    String(Char c)                                                          { operator=(c); }
    /// Does nothing if `str` is null
    String(const char* str)                                                 { operator=(str); }
    String(char c)                                                          { operator=(c); }

    /// Forwards to assign()
    template<class T>
    String& operator=(T&& rhs)                                              { return assign(forward<T>(rhs)); }
    String& operator+=(const String& rhs)                                   { return append(rhs); }

    int size() const                                                        { return static_cast<int>(Super::size()); }
    int length() const                                                      { return static_cast<int>(Super::length()); }
    int max_size() const                                                    { return static_cast<int>(Super::max_size()); }
    int capacity() const                                                    { return static_cast<int>(Super::capacity()); }
    String& clear()                                                         { Super::clear(); return *this; }

    /// Forwards to insert() at back
    template<class T>
    String& append(const T& str, int pos = 0, int count = npos)             { return insert(length(), str, pos, count); }
    String& append(Char c, int count = 1)                                   { return insert(length(), c, count); }
    //String& append(wchar_t c, int count = 1)                                { return insert(length(), c, count); }
    String& append(char c, int count = 1)                                   { return insert(length(), c, count); }
    template<class InputIterator>
    String& append(InputIterator first, InputIterator last)                 { return insert(begin() + length(), first, last); }

    /// Clears and forwards to append()
    template<class T>
    String& assign(const T& str, int pos = 0, int count = npos)             { clear(); return append(str, pos, count); }
    String& assign(Char c, int count = 1)                                   { clear(); return append(c, count); }
    //String& assign(wchar_t c, int count = 1)                                { clear(); return append(c, count); }
    String& assign(char c, int count = 1)                                   { clear(); return append(c, count); }
    template<class InputIterator>
    String& assign(InputIterator first, InputIterator last)                 { clear(); return append<InputIterator>(first, last); }
    String& assign(String&& str)                                            { clear(); return append(forward<String>(str)); }

    String& insert(int pos1, const String& str, int pos2 = 0, int count = npos);
    /// Does nothing if `str` is null
    String& insert(int pos1, const Char* str, int pos2 = 0, int count = npos);
    String& insert(int pos1, Char c, int count = 1)                                 { Super::insert(pos1, count, c); return *this; }
    /// Does nothing if `str` is null
    String& insert(int pos1, const char* str, int pos2 = 0, int count = npos);
    /// Converts char to Char and inserts
    String& insert(int pos1, char c, int count = 1)                                 { return insert(pos1, static_cast<Char>(c), count); }
    iterator insert(const_iterator p, Char c)                                       { return Super::insert(p, c); }
    iterator insert(iterator p, Char c, int count)                                  { return Super::insert(p, c, count); }
    template<class InputIterator>
    iterator insert(const_iterator p, InputIterator first, InputIterator last)      { return Super::insert(p, first, last); }

    String& erase(int pos = 0, int n = npos)                                { Super::erase(pos, n); return *this; }
    iterator erase(const_iterator position)                                 { return Super::erase(position); }
    iterator erase(const_iterator first, const_iterator last)               { return Super::erase(first, last); }

    String& replace(int pos1, int n1, const String& str, int pos2 = 0, int n2 = npos);
    String& replace(const_iterator i1, const_iterator i2, const String& str)                    { Super::replace(i1, i2, str); return *this; }
    template<class InputIterator>
    String& replace(const_iterator i1, const_iterator i2, InputIterator j1, InputIterator j2)   { Super::replace(i1, i2, j1, j2); return *this; }

    int copy(Char* s, int n, int pos = 0) const                             { return Super::copy(s, n, pos); }
    //int copy(wchar_t* s, int n, int pos = 0) const                          { std::wstring str(begin() + pos, begin() + pos + n); return str.copy(s, str.length()); }
    int copy(char* s, int n, int pos = 0) const                             { std::string str(begin() + pos, begin() + pos + n); return str.copy(s, str.length()); }

    String substr(int pos = 0, int n = npos) const                          { return Super::substr(pos, n); }

    int compareIgnoreCase(const String& str, int pos1 = 0, int n1 = npos, int pos2 = 0, int n2 = npos) const;

    /// Split a string into a list of separate substrings delimited by delim
    List split(const String& delim = ' ', int pos = 0, int count = npos) const;

    /// Join list into one string, separated by delim
    static String join(const List& strings, const String& delim = ' ', int start = npos, int end = npos);

    /// Convert string to lower case
    String toLower() const;
    /// Convert string to upper case
    String toUpper() const;

    std::string toStdString() const                                         { return std::string(begin(), end()); }
};

/// \name String operators
/// @{

/// Concatenate strings
/** \relates String */
inline String operator+(const String& lhs, const String& rhs)               { return std::operator+(lhs, rhs); }
/// \relates String
inline bool operator==(const String& lhs, const String& rhs)                { return std::operator==(lhs, rhs); }
/// \relates String
inline bool operator!=(const String& lhs, const String& rhs)                { return std::operator!=(lhs, rhs); }
/// \relates String
inline bool operator<=(const String& lhs, const String& rhs)                { return std::operator<=(lhs, rhs); }
/// \relates String
inline bool operator>=(const String& lhs, const String& rhs)                { return std::operator>=(lhs, rhs); }
/// \relates String
inline bool operator< (const String& lhs, const String& rhs)                { return std::operator<(lhs, rhs); }
/// \relates String
inline bool operator> (const String& lhs, const String& rhs)                { return std::operator>(lhs, rhs); }
/// @}


/// \name String::List operators
/// @{

/// Append to string list
/** \relates String */
inline String::List& operator<<(String::List& list, const String& str)      { list.push_back(str); return list; }
/** \relates String */
inline String::List&& operator<<(String::List&& list, const String& str)    { return move(operator<<(list, str)); }
/// @}

}

