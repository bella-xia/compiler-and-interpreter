#include "str.h"

Str::Str() : ValRep(VALREP_STRING), m_str(new std::string)
{
}

Str::Str(std::string *str) : ValRep(VALREP_STRING), m_str(str)
{
}

Str::~Str()
{
}

Str *Str::get_substr(int start, int len)
{
    if (start + len > (int)m_str->size())
    {
        return new Str(new std::string(""));
    }
    return new Str(new std::string(m_str->substr(start, len)));
}
