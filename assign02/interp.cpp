#include <cassert>
#include <algorithm>
#include <memory>
#include <iostream>
#include <set>
#include <string>
#include "ast.h"
#include "node.h"
#include "exceptions.h"
#include "function.h"
#include "arr.h"
#include "str.h"
#include "interp.h"
#include "environment.h"

Interpreter::Interpreter(Node *ast_to_adopt)
    : m_ast(ast_to_adopt), defined_var({}), intrinsic_funcs({{"print", &intrinsic_print},
                                                             {"println", &intrinsic_println},
                                                             {"readint", &intrinsic_readint},
                                                             {"mkarr", &intrinsic_arr_mkarr},
                                                             {"len", &intrinsic_arr_len},
                                                             {"get", &intrinsic_arr_get},
                                                             {"set", &intrinsic_arr_set},
                                                             {"push", &intrinsic_arr_push},
                                                             {"pop", &intrinsic_arr_pop},
                                                             {"strlen", &intrinsic_str_strlen},
                                                             {"strcat", &intrinsic_str_strcat},
                                                             {"substr", &intrinsic_str_substr}})
{
}

Interpreter::~Interpreter()
{
  delete m_ast;
}

void Interpreter::preinstall_intrinsic_funcs(Environment *global_env)
{
  bool result;
  // iterate over the intrinsic function map
  for (std::map<std::string, IntrinsicFn>::const_iterator it = intrinsic_funcs.cbegin();
       it != intrinsic_funcs.cend(); it++)
  {
    result = global_env->define(it->first, Value(it->second));
    // ensure that all intrinsic function are installed into the global environment properly
    assert(result);
  }
}

void Interpreter::analyze()
{
  // pass into hierachical analysis
  unit_level_analyze(m_ast);
}

void Interpreter::unit_level_analyze(Node *unit_ast)
{
  // initiate global environment
  Environment env = Environment();
  // preinstall intrinsic functions
  preinstall_intrinsic_funcs(&env);

  // iterate over all kids of the global env, determine whether it is a statement
  // or a function and pass into the corresponding analysis
  int nkids = unit_ast->get_num_kids();
  for (int i = 0; i < nkids; i++)
  {
    Node *child = unit_ast->get_kid(i);
    if (child->get_tag() == AST_STATEMENT)
    {
      statement_level_analyze(child, &env);
    }
    else
    {
      function_level_analyze(child, &env);
    }
  }
}

void Interpreter::function_level_analyze(Node *func_ast, Environment *parent_env)
{
  // ensure that there is func_name (func_params -> optional) {func_stmt_list}
  int nkids = func_ast->get_num_kids();
  bool result;
  assert(nkids == 2 || nkids == 3);

  // get func name and stmt list and ensure their AST type
  int stmt_idx = (nkids == 2) ? 1 : 2;
  Node *func_name = func_ast->get_kid(0);
  Node *stmt_list = func_ast->get_kid(stmt_idx);

  assert(func_name->get_tag() == AST_VARREF);
  assert(stmt_list->get_tag() == AST_STATEMENT_LIST);

  std::vector<std::string> param_names;

  // if there are 3 kids, meaning there will be param lists
  if (nkids == 3)
  {

    Node *func_parameters = func_ast->get_kid(1);
    assert(func_parameters->get_tag() == AST_PARAMETER_LIST);
    int nparamkids = func_parameters->get_num_kids();
    for (int k = 0; k < nparamkids; k++)
    {
      param_names.push_back(func_parameters->get_kid(k)->get_str());
    }
  }
  // define function and put it into the parent environment
  Function *func = new Function(func_name->get_str(), param_names, parent_env, stmt_list);
  result = parent_env->define(func->get_name(), Value(func));
  if (!result)
  {
    SemanticError::raise(func_name->get_loc(), "Unable to define variable '%s' because the variable already exists", func_name->get_str().c_str());
  }
  // create new environment for function arguments
  Environment arg_env = Environment(parent_env);

  // define all the parameter variables in the new function environment
  for (std::vector<std::string>::const_iterator cit = param_names.cbegin();
       cit < param_names.cend(); cit++)
  {
    result = arg_env.define(*cit, Value());
    if (!result)
    {
      SemanticError::raise(func_name->get_loc(), "Unable to define variable '%s' because the variable already exists", (*cit).c_str());
    }
  }
  // create new environment for function statements
  Environment stmt_env = Environment(&arg_env);
  // get all statements
  int nkidkids = stmt_list->get_num_kids();
  for (int j = 0; j < nkidkids; j++)
  {
    statement_level_analyze(stmt_list->get_kid(j), &stmt_env);
  }
}

void Interpreter::statement_level_analyze(Node *state_ast, Environment *env)
{
  // iterate over all kids, if there is a control flow go to the corresponding function,
  // otherwise pass into low level general analysis
  int nkids = state_ast->get_num_kids();
  for (int i = 0; i < nkids; i++)
  {
    Node *child = state_ast->get_kid(i);
    if (child->get_tag() == AST_IF || child->get_tag() == AST_WHILE)
    {
      // enter conditional flow
      conditional_flow_analyze(child, env);
    }
    else
    {
      low_level_analyze(child, env);
    }
  }
}

void Interpreter::conditional_flow_analyze(Node *condition_ast, Environment *parent_env)
{
  // create a new environment for statements inside control flow
  Environment env = Environment(parent_env);
  bool is_if = condition_ast->get_tag() == AST_IF;
  int nkids = condition_ast->get_num_kids();
  for (int i = 0; i < nkids; i++)
  {
    Node *child = condition_ast->get_kid(i);
    if (is_if && i == 2)
    {
      // enter the else loop
      conditional_flow_analyze(child, parent_env);
    }
    else
    {
      low_level_analyze(child, &env);
    }
  }
}

void Interpreter::low_level_analyze(Node *current_ast, Environment *env)
{
  int tag = current_ast->get_tag();
  int nkids = current_ast->get_num_kids();
  std::string var_name;
  bool result;
  Value result_value;
  // only look at VARREF and VARDEF ast types as those are the only ones
  // which contributes to environment variable changes
  switch (tag)
  {
  case AST_VARDEF:
    assert(nkids == 1);
    assert(current_ast->get_kid(0)->get_tag() == AST_VARREF);
    var_name = current_ast->get_kid(0)->get_str();
    result = env->define(var_name, Value());
    if (!result)
    {
      SemanticError::raise(current_ast->get_loc(), "Unable to define variable '%s' because the variable already exists", var_name.c_str());
    }
    break;
  case AST_VARREF:
    assert(nkids == 0);
    var_name = current_ast->get_str();
    result_value = env->lookup(var_name);
    if (result_value.is_invalid())
    {
      SemanticError::raise(current_ast->get_loc(), "Unable to access variable '%s' because it is never defined", var_name.c_str());
    }
    break;
  default:
    for (int i = 0; i < nkids; ++i)
    {
      low_level_analyze(current_ast->get_kid(i), env);
    }
    break;
  }
}

Value Interpreter::execute()
{
  Value result;
  // pass into the hierachical execution
  result = unit_level_execute(m_ast);
  return result;
}

Value Interpreter::unit_level_execute(Node *unit_ast)
{
  // create global environment and preinstall intrinsic functions
  Environment env = Environment();
  preinstall_intrinsic_funcs(&env);

  // iterate over all kids and either go to statement or function execution
  // based on AST node type
  int nkids = unit_ast->get_num_kids();
  Value outcome;
  for (int i = 0; i < nkids; i++)
  {
    Node *child = unit_ast->get_kid(i);
    outcome = (child->get_tag() == AST_STATEMENT) ? statement_level_execute(child, &env) : function_level_execute(child, &env);
  }
  return outcome;
}

Value Interpreter::function_level_execute(Node *func_ast, Environment *parent_env)
{
  int nkids = func_ast->get_num_kids();
  // based on whether there are parameter lists, nkids can either be 2 or 3
  assert(nkids == 2 || nkids == 3);

  int stmt_idx = (nkids == 2) ? 1 : 2;

  // get function name and function body
  Node *func_name = func_ast->get_kid(0);
  Node *stmt_list = func_ast->get_kid(stmt_idx);

  // initiate empty parameter vector, if there are parameter list AST,
  // push all parameter names into the vector
  std::vector<std::string> param_names;

  if (nkids == 3)
  {
    Node *func_parameters = func_ast->get_kid(1);
    int nparamkids = func_parameters->get_num_kids();
    for (int k = 0; k < nparamkids; k++)
    {
      param_names.push_back(func_parameters->get_kid(k)->get_str());
    }
  }
  // define function and put it into the parent environment
  Function *func = new Function(func_name->get_str(), param_names, parent_env, stmt_list);
  parent_env->define(func->get_name(), Value(func));
  // return the value function
  return Value(func);
}

Value Interpreter::statement_level_execute(Node *state_ast, Environment *env)
{
  int nkids = state_ast->get_num_kids();
  Value result;
  // iterate over the kids of the statement,
  // either go to control flow execution or general low_level execution
  for (int i = 0; i < nkids; i++)
  {
    Node *child = state_ast->get_kid(i);
    if (child->get_tag() == AST_IF)
    {
      result = if_flow_execute(child, env);
    }
    else if (child->get_tag() == AST_WHILE)
    {
      result = while_flow_execute(child, env);
    }
    else
    {
      result = low_level_execute(child, env);
    }
  }
  return result;
}

Value Interpreter::if_flow_execute(Node *condition_ast, Environment *parent_env)
{
  int nkids = condition_ast->get_num_kids();
  assert(nkids == 2 || nkids == 3);
  Node *condition = condition_ast->get_kid(0);
  Node *if_stmt_list = condition_ast->get_kid(1);
  assert(if_stmt_list->get_tag() == AST_STATEMENT_LIST);
  Value ident = low_level_execute(condition, parent_env);
  // make sure ident is evaluable
  if (ident.get_kind() != VALUE_INT)
  {
    EvaluationError::raise(condition->get_loc(), "the evaluation of the condition is not a numeric value");
  }
  if (ident.get_ival())
  {
    // if statement execute
    // create new if environment and execute all following
    // statement list under this new environment
    Environment if_env = Environment(parent_env);
    int nifkids = if_stmt_list->get_num_kids();
    Value result;

    for (int i = 0; i < nifkids; ++i)
    {
      result = statement_level_execute(if_stmt_list->get_kid(i), &if_env);
    }
  }
  else if (nkids == 3)
  {
    // else statement execute
    // create new else environment and execute all following
    // statement list under this new environment
    Node *else_stmt_list = condition_ast->get_kid(2);
    assert(else_stmt_list->get_tag() == AST_STATEMENT_LIST);
    Environment else_env = Environment(parent_env);
    int nelsekids = else_stmt_list->get_num_kids();
    Value result;
    for (int i = 0; i < nelsekids; ++i)
    {
      result = statement_level_execute(else_stmt_list->get_kid(i), &else_env);
    }
  }
  // conditional statement evaluates to 0
  return Value();
}

Value Interpreter::while_flow_execute(Node *condition_ast, Environment *parent_env)
{
  int nkids = condition_ast->get_num_kids();
  assert(nkids == 2);
  Node *condition = condition_ast->get_kid(0);
  Node *while_stmt_list = condition_ast->get_kid(1);
  assert(while_stmt_list->get_tag() == AST_STATEMENT_LIST);
  Value ident = low_level_execute(condition, parent_env);
  Value result;
  if (ident.get_kind() != VALUE_INT)
  {
    EvaluationError::raise(condition->get_loc(), "the evaluation of the condition is not a numeric value");
  }
  // recursively run over condition as long as it is true
  while (ident.get_ival())
  {
    // create new while environment and execute all
    // statement in statement list in this new environment
    Environment while_env = Environment(parent_env);
    int nwhilekids = while_stmt_list->get_num_kids();
    for (int i = 0; i < nwhilekids; i++)
    {
      result = statement_level_execute(while_stmt_list->get_kid(i), &while_env);
    }
    // re-evaluate ident and ensure it is still numeric
    ident = low_level_execute(condition, parent_env);
    if (ident.get_kind() != VALUE_INT)
    {
      EvaluationError::raise(condition->get_loc(), "the evaluation of the condition is not a numeric value");
    }
  }
  // conditional statement evaluates to 0
  return Value();
}

Value Interpreter::low_level_execute(Node *current_ast, Environment *env)
{
  int current_tag = current_ast->get_tag();
  switch (current_tag)
  {
  case AST_ASSIGN:
    return low_level_execute_assign(current_ast, env);
  case AST_VARDEF:
    return low_level_execute_vardef(current_ast, env);
  case AST_VARREF:
    return low_level_execute_varref(current_ast, env);
  case AST_INT_LITERAL:
    return low_level_execute_int_literal(current_ast, env);
  case AST_STRING_LITERAL:
    return low_level_execute_str_literal(current_ast, env);
  case AST_FNCALL:
    return low_level_execute_funcall(current_ast, env);
  default:
    return low_level_execute_operation(current_ast, env);
  }
}

Value Interpreter::low_level_execute_funcall(Node *funcall_ast, Environment *parent_env)
{
  // get function Value from the parent environment
  int nkids = funcall_ast->get_num_kids();
  assert(nkids == 1 || nkids == 2);

  // get respective kids
  Node *func_name = funcall_ast->get_kid(0);
  assert(func_name->get_tag() == AST_VARREF);

  // find function from parent_env
  Value func_val = parent_env->lookup(func_name->get_str());
  enum ValueKind val_kind = func_val.get_kind();
  // if is it an intrinsic function
  if (val_kind == VALUE_INTRINSIC_FN)
  {
    // initiate argument lists with passed-in parameters if there are any
    Environment arg_env = Environment(parent_env);
    int num_args = (nkids == 1) ? 0 : funcall_ast->get_kid(1)->get_num_kids();
    Value args[num_args];
    if (nkids == 2)
    {
      Node *func_arglist = funcall_ast->get_kid(1);
      assert(func_arglist->get_tag() == AST_ARGUMENT_LIST);
      int num_args = func_arglist->get_num_kids();
      for (int i = 0; i < num_args; i++)
      {
        args[i] = low_level_execute(func_arglist->get_kid(i), parent_env);
      }
    }
    // create and evaluate intrinsic function
    IntrinsicFn intrinsic_func = func_val.get_intrinsic_fn();
    Value outcome = intrinsic_func(args, num_args, func_name->get_loc(), this);
    return outcome;
  }
  else if (val_kind == VALUE_FUNCTION)
  {
    Environment *curr_env = func_val.get_function()->get_parent_env();
    // create new env for function call argument list
    Environment arg_env = Environment(curr_env);
    Function *func = func_val.get_function();
    // find all the variables in the arglist and in the paramlist and match their values
    if (nkids == 2)
    {
      Node *func_arglist = funcall_ast->get_kid(1);
      // if the number of parameters in the arglist does not fit the number of parameters,
      // throw evaluation error
      if (func_arglist->get_num_kids() != func->get_num_params())
      {
        EvaluationError::raise(func_arglist->get_loc(), "Function %s expects %d parameters, but %d are given by the argument list", func->get_name().c_str(), func->get_num_params(), func_arglist->get_num_kids());
      }
      int args = func_arglist->get_num_kids();
      const std::vector<std::string> params = func->get_params();
      for (int i = 0; i < args; i++)
      {
        arg_env.define(params[i], low_level_execute(func_arglist->get_kid(i), parent_env));
      }
    }
    else if (func->get_num_params() != 0)
    {
      EvaluationError::raise(func_name->get_loc(), "Function %s expects %d parameters, but %d are given by the argument list", func->get_name().c_str(), func->get_num_params(), 0);
    }
    Value outcome;
    // create new env for function call statement list
    Environment stmt_env = Environment(&arg_env);
    Node *funcall_stmt_list = func->get_body();
    int nstmtkids = funcall_stmt_list->get_num_kids();
    for (int i = 0; i < nstmtkids; ++i)
    {
      outcome = statement_level_execute(funcall_stmt_list->get_kid(i), &stmt_env);
    }
    return outcome;
  }
  // if value kind is neither function nor intrinsic function, throw evaluation error
  EvaluationError::raise(func_name->get_loc(), "the funcall variable %s type is not function or intrinsic function", func_name->get_str().c_str());
}

Value Interpreter::low_level_execute_assign(Node *current_ast, Environment *env)
{
  int nkids = current_ast->get_num_kids();
  assert(nkids == 2);
  assert(current_ast->get_kid(0)->get_tag() == AST_VARREF);
  std::string LHS = current_ast->get_kid(0)->get_str();
  Value RHS = low_level_execute(current_ast->get_kid(1), env);
  bool result = env->modify(LHS, RHS);
  if (!result)
  {
    EvaluationError::raise(current_ast->get_kid(0)->get_loc(), "Unable to modify the value of variable %s", LHS.c_str());
  }
  return RHS;
}

Value Interpreter::low_level_execute_vardef(Node *current_ast, Environment *env)
{
  int nkids = current_ast->get_num_kids();
  assert(nkids == 1);
  assert(current_ast->get_kid(0)->get_tag() == AST_VARREF);
  std::string LHS = current_ast->get_kid(0)->get_str();
  env->define(LHS, Value());
  return Value();
}

Value Interpreter::low_level_execute_int_literal(Node *current_ast, Environment *env)
{
  int nkids = current_ast->get_num_kids();
  assert(nkids == 0);
  return Value(std::stoi(current_ast->get_str()));
}

Value Interpreter::low_level_execute_str_literal(Node *current_ast, Environment *env)
{
  int nkids = current_ast->get_num_kids();
  assert(nkids == 0);
  std::string *str_actual = new std::string(current_ast->get_str());
  return Value(new Str(str_actual));
}

Value Interpreter::low_level_execute_varref(Node *current_ast, Environment *env)
{
  int nkids = current_ast->get_num_kids();
  assert(nkids == 0);
  Value result = env->lookup(current_ast->get_str());
  return result;
}

Value Interpreter::low_level_execute_operation(Node *current_ast, Environment *env)
{
  int nkids = current_ast->get_num_kids();
  int current_tag = current_ast->get_tag();
  assert(nkids == 2);
  Value LHS = low_level_execute(current_ast->get_kid(0), env);
  if (LHS.get_kind() != VALUE_INT)
  {
    EvaluationError::raise(current_ast->get_kid(0)->get_loc(), "Left-hand side value for the current operation cannot be evaluated as numeric values");
  }
  int LHS_value = LHS.get_ival();
  // short circuit
  if ((current_tag == AST_LOGICAL_AND && !LHS_value) || (current_tag == AST_LOGICAL_OR && LHS_value))
  {
    return Value(current_tag == AST_LOGICAL_OR);
  }
  Value RHS = low_level_execute(current_ast->get_kid(1), env);
  if (RHS.get_kind() != VALUE_INT)
  {
    EvaluationError::raise(current_ast->get_kid(1)->get_loc(), "Right-hand side value for the current operation cannot be evaluated as numeric values");
  }
  int RHS_value = RHS.get_ival();
  switch (current_tag)
  {
  case AST_ADD:
    return Value(LHS_value + RHS_value);
  case AST_SUB:
    return Value(LHS_value - RHS_value);
  case AST_MULTIPLY:
    return Value(LHS_value * RHS_value);
  case AST_DIVIDE:
    if (RHS_value == 0)
    {
      EvaluationError::raise(current_ast->get_loc(), "Unable to complete division operation because the denominator is 0");
    }
    return Value(LHS_value / RHS_value);
  case AST_LOGICAL_AND:
    return Value(LHS_value && RHS_value);
  case AST_LOGICAL_OR:
    return Value(LHS_value || RHS_value);
  case AST_GT:
    return Value(LHS_value > RHS_value);
  case AST_GTE:
    return Value(LHS_value >= RHS_value);
  case AST_LT:
    return Value(LHS_value < RHS_value);
  case AST_LTE:
    return Value(LHS_value <= RHS_value);
  case AST_EQ:
    return Value(LHS_value == RHS_value);
  case AST_NEQ:
    return Value(LHS_value != RHS_value);
  default:
    return Value(VALUE_INVALID);
  }
}

// intrinsic functions

// intrinsic function print
Value Interpreter::intrinsic_print(Value args[], unsigned num_args,
                                   const Location &loc, Interpreter *interp)
{
  if (num_args != 1)
    EvaluationError::raise(loc, "Wrong number of arguments passed to print function");
  printf("%s", args[0].as_str().c_str());
  return Value();
}

// intrinsic function println
Value Interpreter::intrinsic_println(Value args[], unsigned num_args,
                                     const Location &loc, Interpreter *interp)
{
  if (num_args != 1)
    EvaluationError::raise(loc, "Wrong number of arguments passed to println function");
  printf("%s\n", args[0].as_str().c_str());
  return Value();
}

// intrinsic function readint
Value Interpreter::intrinsic_readint(Value args[], unsigned num_args,
                                     const Location &loc, Interpreter *interp)
{
  if (num_args != 0)
    EvaluationError::raise(loc, "Wrong number of arguments passed to readint function");
  int a;
  std::scanf("%d", &a);
  return Value(a);
}

Value Interpreter::intrinsic_arr_mkarr(Value args[], unsigned num_args,
                                       const Location &loc, Interpreter *interp)
{
  std::vector<Value> *arr = new std::vector<Value>();
  for (unsigned i = 0; i < num_args; ++i)
  {
    if (args[i].get_kind() != VALUE_INT && args[i].get_kind() != VALUE_ARRAY)
    {
      EvaluationError::raise(loc, "Not an integer or an array, cannot be used in array");
    }
    arr->push_back(args[i]);
  }
  Arr *array = new Arr(arr);
  return Value(array);
}

Value Interpreter::intrinsic_arr_len(Value args[], unsigned num_args,
                                     const Location &loc, Interpreter *interp)
{
  if (num_args != 1)
    EvaluationError::raise(loc, "Wrong number of arguments passed to len function");
  Value arr_val = args[0];
  if (arr_val.get_kind() != VALUE_ARRAY)
    EvaluationError::raise(loc, "len function encounters a non-array parameter");
  return arr_val.get_array()->get_len();
}

Value Interpreter::intrinsic_arr_get(Value args[], unsigned num_args,
                                     const Location &loc, Interpreter *interp)
{
  if (num_args != 2)
    EvaluationError::raise(loc, "Wrong number of arguments passed to get function");
  Value arr_val = args[0];
  if (arr_val.get_kind() != VALUE_ARRAY)
    EvaluationError::raise(loc, "getter function encounters a non-array parameter");
  Value idx = args[1];
  if (idx.get_kind() != VALUE_INT)
    EvaluationError::raise(loc, "arrays only have interger index");
  return arr_val.get_array()->get_idx(idx.get_ival());
}

Value Interpreter::intrinsic_arr_set(Value args[], unsigned num_args,
                                     const Location &loc, Interpreter *interp)
{
  if (num_args != 3)
    EvaluationError::raise(loc, "Wrong number of arguments passed to set function");
  Value arr_val = args[0];
  if (arr_val.get_kind() != VALUE_ARRAY)
    EvaluationError::raise(loc, "setter function encounters a non-array parameter");
  Value idx = args[1];
  if (idx.get_kind() != VALUE_INT)
    EvaluationError::raise(loc, "arrays only have interger index");
  Value setter_val = args[2];
  if (setter_val.get_kind() != VALUE_INT && setter_val.get_kind() != VALUE_ARRAY)
    EvaluationError::raise(loc, "arrays can only store integers");
  return arr_val.get_array()->set(idx.get_ival(), setter_val);
}

Value Interpreter::intrinsic_arr_push(Value args[], unsigned num_args,
                                      const Location &loc, Interpreter *interp)
{
  if (num_args != 2)
    EvaluationError::raise(loc, "Wrong number of arguments passed to push function");
  Value arr_val = args[0];
  if (arr_val.get_kind() != VALUE_ARRAY)
    EvaluationError::raise(loc, "push function encounters a non-array parameter");
  Value push_val = args[1];
  if (push_val.get_kind() != VALUE_INT && push_val.get_kind() != VALUE_ARRAY)
    EvaluationError::raise(loc, "arrays can only store integers");
  arr_val.get_array()->push(push_val);
  return Value();
}

Value Interpreter::intrinsic_arr_pop(Value args[], unsigned num_args,
                                     const Location &loc, Interpreter *interp)
{
  if (num_args != 1)
    EvaluationError::raise(loc, "Wrong number of arguments passed to pop function");
  Value arr_val = args[0];
  if (arr_val.get_kind() != VALUE_ARRAY)
    EvaluationError::raise(loc, "pop function encounters a non-array parameter");
  Value result = arr_val.get_array()->pop();
  if (result.is_invalid())
  {
    EvaluationError::raise(loc, "there is no more element in the array to be poped");
  }
  return result;
}

Value Interpreter::intrinsic_str_strlen(Value args[], unsigned num_args,
                                        const Location &loc, Interpreter *interp)
{
  if (num_args != 1)
  {
    EvaluationError::raise(loc, "Wrong number of arguments passed to strlen function");
  }
  Value str_val = args[0];
  if (str_val.get_kind() != VALUE_STRING)
  {
    EvaluationError::raise(loc, "strlen function encounters a non-string parameter");
  }
  return Value(str_val.get_str()->get_len());
}

Value Interpreter::intrinsic_str_substr(Value args[], unsigned num_args,
                                        const Location &loc, Interpreter *interp)
{
  if (num_args != 3)
  {
    EvaluationError::raise(loc, "Wrong number of arguments passed to substr function");
  }
  Value str_val = args[0];
  Value start_idx = args[1];
  Value str_len = args[2];
  if (str_val.get_kind() != VALUE_STRING || start_idx.get_kind() != VALUE_INT || str_len.get_kind() != VALUE_INT)
  {
    EvaluationError::raise(loc, "parameter type does not fit the standard of substr function");
  }
  return Value(str_val.get_str()->get_substr(start_idx.get_ival(), str_len.get_ival()));
}

Value Interpreter::intrinsic_str_strcat(Value args[], unsigned num_args,
                                        const Location &loc, Interpreter *interp)
{
  if (num_args != 2)
  {
    EvaluationError::raise(loc, "Wrong number of arguments passed to strcat function");
  }
  Value str_val1 = args[0];
  Value str_val2 = args[1];
  if (str_val1.get_kind() != VALUE_STRING || str_val2.get_kind() != VALUE_STRING)
  {
    EvaluationError::raise(loc, "strcat function encounters a non-string parameter");
  }
  return Value(str_val1.get_str()->get_strcat(str_val2.get_str()));
}
