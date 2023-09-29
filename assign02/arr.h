#ifndef ARR_H
#define ARR_H

#include <vector>
#include <string>
#include "value.h"
#include "valrep.h"
class Environment;
class Node;

class Arr : public ValRep
{
private:
    std::vector<Value> *m_vec;

    // value semantics prohibited
    Arr(const Arr &);
    Arr &operator=(const Arr &);

public:
    // constructor
    Arr();
    Arr(std::vector<Value> *vec);
    Value get_idx(int idx) { return m_vec->at(idx); }
    Value set(int idx, Value val);
    Value get_len() { return Value(m_vec->size()); }
    void push(Value num) { m_vec->push_back(num); }
    Value pop();
    virtual ~Arr();
};

#endif // FUNCTION_H
