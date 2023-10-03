#ifndef FUNCTION_H
#define FUNCTION_H

#include <vector>
#include <string>
#include "value.h"
#include "valrep.h"
class Environment;
class Node;

class Function : public ValRep
{
private:
  std::string m_name;
  std::vector<std::string> m_params;
  Value m_parent_env;
  Node *m_body;

  // value semantics prohibited
  Function(const Function &);
  Function &operator=(const Function &);

public:
  Function(const std::string &name, const std::vector<std::string> &params, Value parent_env, Node *body);
  virtual ~Function();

  std::string get_name() const { return m_name; }
  const std::vector<std::string> &get_params() const { return m_params; }
  unsigned get_num_params() const { return unsigned(m_params.size()); }
  Value get_parent_env() const { return m_parent_env; }
  Node *get_body() const { return m_body; }
};

#endif // FUNCTION_H
