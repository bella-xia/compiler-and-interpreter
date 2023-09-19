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

bool Environment::define(std::string var)
{
  if (environment_map.find(var) != environment_map.end())
  {
    return false;
  }
  environment_map.insert({var, Value()});
  return true;
}

bool Environment::assign_int(std::string var, int value)
{
  if (environment_map.find(var) != environment_map.end())
  {
    environment_map[var].set_ival(value);
    return true;
  }
  else if (m_parent == nullptr)
  {
    return false;
  }
  return m_parent->assign_int(var, value);
}

// TODO: implement member functions
