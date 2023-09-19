#ifndef INTERP_H
#define INTERP_H

#include "value.h"
#include "environment.h"
class Node;
class Location;

class Interpreter
{
private:
  Node *m_ast;
  std::set<std::string> defined_var;

public:
  Interpreter(Node *ast_to_adopt);
  ~Interpreter();

  void analyze();
  Value execute();

  void unit_level_analyze(Node *unit_ast);
  void statement_level_analyze(Node *state_ast, Environment *env);
  void low_level_analyze(Node *current_ast, Environment *env);

  // three respective level of execution
  Value unit_level_execute(Node *unit_ast);
  Value statement_level_execute(Node *state_ast, Environment *env);
  Value low_level_execute(Node *current_ast, Environment *env);

  // helper function for all possible operations in low level execution
  Value low_level_execute_assign(Node *current_ast, Environment *env);
  Value low_level_execute_vardef(Node *current_ast, Environment *env);
  Value low_level_execute_varref(Node *current_ast, Environment *env);
  Value low_level_execute_int_literal(Node *current_ast, Environment *env);
  Value low_level_execute_operation(Node *current_ast, Environment *env);

private:
  // TODO: private member functions
};

#endif // INTERP_H
