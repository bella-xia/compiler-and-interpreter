#ifndef STR_H
#define STR_H

#include <vector>
#include <string>
#include "value.h"
#include "valrep.h"
class Environment;
class Node;

class Str : public ValRep
{
private:
    std::string m_str;

    // value semantics prohibited
    Str(const Str &);
    Str &operator=(const Str &);

public:
    // constructor
    Str();
    Str(std::string str);
    virtual ~Str();

    int get_len() { return m_str.size(); }
    std::string get_str() { return m_str; }
    Str *get_substr(int start, int len);
    Str *get_strcat(Str *other) { return new Str(std::string(m_str + other->get_str())); }
};

#endif // FUNCTION_H
