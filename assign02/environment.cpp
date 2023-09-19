#include "environment.h"

Environment::Environment(Environment *parent)
    : m_parent(parent),
      environment_map({})
{
  assert(m_parent != this);
}

Environment::~Environment()
{
}

Value Environment::lookup(std::string var)
{
  if (environment_map.find(var) != environment_map.end())
  {
    return environment_map[var];
  }
  else if (m_parent == nullptr)
  {
    return Value(VALUE_INVALID);
  }
  return m_parent->lookup(var);
}

bool Environment::define(std::string var, Value v)
{
  if (environment_map.find(var) != environment_map.end())
  {
    return false;
  }
  environment_map.insert({var, v});
  return true;
}

bool Environment::modify(std::string var, Value v)
{
  if (environment_map.find(var) != environment_map.end())
  {
    environment_map[var] = v;
    return true;
  }
  else if (m_parent == nullptr)
  {
    return false;
  }
  return m_parent->modify(var, v);
}

// TODO: implement member functions
