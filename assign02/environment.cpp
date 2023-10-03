#include "environment.h"

Environment::Environment(Value parent)
    : ValRep(VALREP_ENVIRONMENT), m_parent(parent),
      environment_map({})
{
  assert(m_parent.is_invalid() || m_parent.get_env() != this);
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
  else if (m_parent.is_invalid())
  {
    return Value(VALUE_INVALID);
  }
  return m_parent.get_env()->lookup(var);
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
  else if (m_parent.is_invalid())
  {
    return false;
  }
  return m_parent.get_env()->modify(var, v);
}

// TODO: implement member functions
