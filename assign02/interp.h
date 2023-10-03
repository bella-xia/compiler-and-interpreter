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
  std::map<std::string, IntrinsicFn> intrinsic_funcs;
  Environment *global_env;

public:
  Interpreter(Node *ast_to_adopt);
  ~Interpreter();

  void analyze();
  Value execute();

private:
  void preinstall_intrinsic_funcs(Environment *global_env);
  // hierachical analyze helper functions
  void unit_level_analyze(Node *unit_ast);
  void function_level_analyze(Node *func_ast, Environment *parent_env);
  void statement_level_analyze(Node *state_ast, Environment *env);
  void conditional_flow_analyze(Node *condition_ast, Environment *parent_env);
  void low_level_analyze(Node *current_ast, Environment *env);

  // three respective level of execution
  Value unit_level_execute(Node *unit_ast);
  Value function_level_execute(Node *func_ast, Environment *parent_env);
  Value statement_level_execute(Node *state_ast, Environment *env);
  Value if_flow_execute(Node *condition_ast, Environment *parent_env);
  Value while_flow_execute(Node *condition_ast, Environment *parent_env);
  Value low_level_execute(Node *current_ast, Environment *env);

  // helper function for all possible operations in low level execution
  Value low_level_execute_assign(Node *current_ast, Environment *env);
  Value low_level_execute_vardef(Node *current_ast, Environment *env);
  Value low_level_execute_varref(Node *current_ast, Environment *env);
  Value low_level_execute_funcall(Node *funcall_ast, Environment *parent_env);
  Value low_level_execute_int_literal(Node *current_ast, Environment *env);
  Value low_level_execute_str_literal(Node *current_ast, Environment *env);
  Value low_level_execute_operation(Node *current_ast, Environment *env);
  // TODO: private member functions

  // intrinsic functions
  static Value intrinsic_print(Value args[], unsigned num_args,
                               const Location &loc, Interpreter *interp);
  static Value intrinsic_println(Value args[], unsigned num_args,
                                 const Location &loc, Interpreter *interp);
  static Value intrinsic_readint(Value args[], unsigned num_args,
                                 const Location &loc, Interpreter *interp);

  // instrinsic functions for ValRep Arr
  static Value intrinsic_arr_mkarr(Value args[], unsigned num_args,
                                   const Location &loc, Interpreter *interp);

  static Value intrinsic_arr_len(Value args[], unsigned num_args,
                                 const Location &loc, Interpreter *interp);
  static Value intrinsic_arr_get(Value args[], unsigned num_args,
                                 const Location &loc, Interpreter *interp);

  static Value intrinsic_arr_set(Value args[], unsigned num_args,
                                 const Location &loc, Interpreter *interp);

  static Value intrinsic_arr_push(Value args[], unsigned num_args,
                                  const Location &loc, Interpreter *interp);

  static Value intrinsic_arr_pop(Value args[], unsigned num_args,
                                 const Location &loc, Interpreter *interp);

  static Value intrinsic_str_strlen(Value args[], unsigned num_args,
                                    const Location &loc, Interpreter *interp);

  static Value intrinsic_str_substr(Value args[], unsigned num_args,
                                    const Location &loc, Interpreter *interp);

  static Value intrinsic_str_strcat(Value args[], unsigned num_args,
                                    const Location &loc, Interpreter *interp);
};

#endif // INTERP_H
