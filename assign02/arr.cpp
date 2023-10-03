#include "arr.h"

Arr::Arr() : ValRep(VALREP_VECTOR)
{
    m_vec = std::vector<Value>();
}

Arr::Arr(std::vector<Value> vec) : ValRep(VALREP_VECTOR), m_vec(vec)
{
}

Value Arr::pop()
{
    if (m_vec.empty())
    {
        return Value(VALUE_INVALID);
    }
    Value last_ele = m_vec.back();
    m_vec.pop_back();
    return last_ele;
}

Value Arr::set(int idx, Value val)
{
    m_vec[idx] = val;
    return val;
}

Arr::~Arr()
{
}