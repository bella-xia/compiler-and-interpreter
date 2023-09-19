#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <cassert>
#include <map>
#include <string>
#include "value.h"

class Environment
{
private:
  Environment *m_parent;
  std::map<std::string, Value> environment_map;
  // copy constructor and assignment operator prohibited
  Environment(const Environment &);
  Environment &operator=(const Environment &);

public:
  Environment(Environment *parent = nullptr);
  ~Environment();
  Value lookup(std::string var);
  bool define(std::string var, Value v);
  bool modify(std::string var, Value v);

  // TODO: add member functions allowing lookup, definition, and assignment
};

#endif // ENVIRONMENT_H
