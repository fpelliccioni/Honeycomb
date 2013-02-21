// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/String/String.h"
#include "Honey/String/Stream.h"

namespace honey
{

String& String::insert(int pos1, const String& str, int pos2, int count)
{
    if (count == npos) count = str.length() - pos2;
    Super::insert(pos1, str, pos2, count);
    return *this;
}

String& String::insert(int pos1, const Char* str, int pos2, int count)
{
    if (!str) return *this;
    if (count == npos)  Super::insert(pos1, str + pos2);
    else                Super::insert(pos1, str + pos2, count);
    return *this;
}

String& String::insert(int pos1, const char* str, int pos2, int count)
{
    if (!str) return *this;
    if (count == npos) count = std::char_traits<char>::length(str) - pos2;
    Super::insert(begin() + pos1, str + pos2, str + pos2 + count);
    return *this;
}

String& String::replace(int pos1, int n1, const String& str, int pos2, int n2)
{
    if (n2 == npos) n2 = str.length() - pos2;
    Super::replace(pos1, n1, str, pos2, n2);
    return *this;
}

int String::compareIgnoreCase(const String& str, int pos1, int n1, int pos2, int n2) const
{
    if (n1 == npos) n1 = length() - pos1;
    if (n2 == npos) n2 = str.length() - pos2;

    if (pos1 + n1 > length()) n1 = length() - pos1;
    if (pos2 + n2 > str.length()) n2 = str.length() - pos2;

    int length = (n1 < n2) ? n1 : n2;

    for (int i = 0; i < length; ++i)
    {
        Char val = std::tolower((*this)[pos1+i]);
        Char val2 = std::tolower(str[pos2+i]);
        if (val != val2) return val < val2 ? -1 : 1;
    }

    if (n1 != n2) return n1 < n2 ? -1 : 1;
    return 0;
}

String String::toLower() const
{
    String str;
    std::transform(begin(), end(), str.begin(), (int (*)(int))std::tolower);
    return str;
}

String String::toUpper() const
{
    String str;
    std::transform(begin(), end(), str.begin(), (int (*)(int))std::toupper);
    return str;
}


/// Iterator class for tokenizing strings by a delimiter
class Tokenizer
{
public:
    typedef std::input_iterator_tag iterator_category;
    typedef String                  value_type;
    typedef ptrdiff_t               difference_type;
    typedef String*                 pointer;
    typedef String&                 reference;
    
    Tokenizer(const String& str, const String& delim, String::const_iterator it) :
        _str(&str),
        _delim(&delim),
        _pos(it - _str->begin()),
        _tokenCount(0)
    {
        assert(!_delim->empty());
        operator++();
    }

    Tokenizer& operator++()
    {
        if (_pos < 0 || _pos >= _str->length() || _pos == String::npos)
        {
            _pos = String::npos;
            _token.clear();
            return *this;
        }

        if (_tokenCount > 0)
            _pos += _delim->length();

        int lastPos = _pos;
    
        _pos = _str->find(*_delim, _pos);
        if (_pos == String::npos)
            _pos = _str->length();
    
        _token = _str->substr(lastPos, _pos - lastPos);
        ++_tokenCount;

        return *this;
    }

    bool operator==(const Tokenizer& rhs) const     { return _pos == rhs._pos; }
    bool operator!=(const Tokenizer& rhs) const     { return !operator==(rhs); }

    const String& operator*() const                 { return _token; }

private:
    const String* _str;
    const String* _delim;
    
    int _pos;
    String _token;
    int _tokenCount;
};

String::List String::split(const String& delim, int pos, int count) const
{
    List ret;
    if (count == npos)
        count = length();
    std::copy(Tokenizer(*this, delim, begin()+pos), Tokenizer(*this, delim, begin()+pos+count), std::back_inserter(ret));
    return ret;
}


class Linker
{
public:
    typedef std::output_iterator_tag    iterator_category;
    typedef void                        value_type;
    typedef ptrdiff_t                   difference_type;
    typedef void*                       pointer;
    typedef void                        reference;

    Linker(String& str, const String& delim = ' ')      : _str(&str), _delim(&delim), _token(0) {}

    Linker& operator=(const String& str)
    {
        if (_token > 0) _str->append(*_delim);
        _str->append(str);
        return *this;
    }

    Linker& operator++()                                { ++_token; return *this; }
    Linker& operator*()                                 { return *this; }

private:
    String* _str;
    const String* _delim;
    int _token;
};

String String::join(const List& strings, const String& delim, int start, int end)
{
    auto itStart = start != npos ? strings.begin() + start : strings.begin();
    auto itEnd = end != npos ? strings.begin() + end : strings.end();

    String ret;
    std::copy(itStart, itEnd, Linker(ret, delim));
    return ret;
}

}
