#include "str.h"

Str::Str() : ValRep(VALREP_STRING), m_str(std::string(""))
{
}

Str::Str(std::string str) : ValRep(VALREP_STRING), m_str(str)
{
}
Str *Str::get_substr(int start, int len)
{
    if (start + len > (int)m_str.size())
    {
        return new Str(std::string(""));
    }
    return new Str(std::string(m_str.substr(start, len)));
}

Str::~Str()
{
}
