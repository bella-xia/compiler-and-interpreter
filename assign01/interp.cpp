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
#include "interp.h"
#include "environment.h"

Interpreter::Interpreter(Node *ast_to_adopt)
    : m_ast(ast_to_adopt), defined_var({})
{
}

Interpreter::~Interpreter()
{
  delete m_ast;
}

void Interpreter::analyze()
{
  unit_level_analyze(m_ast);

  // TODO: implement
}

void Interpreter::unit_level_analyze(Node *unit_ast)
{
  Environment env = Environment();
  int nkids = unit_ast->get_num_kids();
  for (int i = 0; i < nkids; i++)
  {
    statement_level_analyze(unit_ast->get_kid(i), &env);
  }
}

void Interpreter::statement_level_analyze(Node *state_ast, Environment *env)
{
  int nkids = state_ast->get_num_kids();
  for (int i = 0; i < nkids; i++)
  {
    low_level_analyze(state_ast->get_kid(i), env);
  }
}

void Interpreter::low_level_analyze(Node *current_ast, Environment *env)
{
  int tag = current_ast->get_tag();
  int nkids = current_ast->get_num_kids();
  std::string var_name;
  bool result;
  Value result_value;
  switch (tag)
  {
  case AST_VARDEF:
    assert(nkids == 1);
    assert(current_ast->get_kid(0)->get_tag() == AST_VARREF);
    var_name = current_ast->get_kid(0)->get_str();
    result = env->define(var_name);
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
  // TODO: implement
  Value result;
  result = unit_level_execute(m_ast);
  return result;
}

Value Interpreter::unit_level_execute(Node *unit_ast)
{
  Environment env = Environment();
  int nkids = unit_ast->get_num_kids();
  Value result;
  for (int i = 0; i < nkids; i++)
  {
    if (i == nkids - 1)
    {
      result = statement_level_execute(unit_ast->get_kid(i), &env);
    }
    else
    {
      statement_level_execute(unit_ast->get_kid(i), &env);
    }
  }
  return result;
}

Value Interpreter::statement_level_execute(Node *state_ast, Environment *env)
{
  int nkids = state_ast->get_num_kids();
  Value result;
  for (int i = 0; i < nkids; i++)
  {
    if (i == nkids - 1)
    {
      result = low_level_execute(state_ast->get_kid(i), env);
    }
    else
    {
      low_level_execute(state_ast->get_kid(i), env);
    }
  }
  return result;
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
  default:
    return low_level_execute_operation(current_ast, env);
  }
}

Value Interpreter::low_level_execute_assign(Node *current_ast, Environment *env)
{
  int nkids = current_ast->get_num_kids();
  assert(nkids == 2);
  assert(current_ast->get_kid(0)->get_tag() == AST_VARREF);
  std::string LHS = current_ast->get_kid(0)->get_str();
  Value RHS = low_level_execute(current_ast->get_kid(1), env);
  bool result = env->assign_int(LHS, RHS.get_ival());
  if (!result)
  {
    SemanticError::raise(current_ast->get_loc(), "Unable to assign integer value to variable '%s'", LHS.c_str());
  }
  return RHS;
}

Value Interpreter::low_level_execute_vardef(Node *current_ast, Environment *env)
{
  int nkids = current_ast->get_num_kids();
  assert(nkids == 1);
  assert(current_ast->get_kid(0)->get_tag() == AST_VARREF);
  std::string LHS = current_ast->get_kid(0)->get_str();
  bool result = env->define(LHS);
  if (!result)
  {
    SemanticError::raise(current_ast->get_loc(), "Unable to define variable '%s' because the variable already exists", LHS.c_str());
  }
  return Value();
}

Value Interpreter::low_level_execute_int_literal(Node *current_ast, Environment *env)
{
  int nkids = current_ast->get_num_kids();
  assert(nkids == 0);
  return Value(std::stoi(current_ast->get_str()));
}

Value Interpreter::low_level_execute_varref(Node *current_ast, Environment *env)
{
  int nkids = current_ast->get_num_kids();
  assert(nkids == 0);
  Value result = env->lookup(current_ast->get_str());
  if (result.is_invalid())
  {
    SemanticError::raise(current_ast->get_loc(), "Unable to access variable '%s'", current_ast->get_str().c_str());
  }
  return result;
}

Value Interpreter::low_level_execute_operation(Node *current_ast, Environment *env)
{
  int nkids = current_ast->get_num_kids();
  int current_tag = current_ast->get_tag();
  assert(nkids == 2);
  Value LHS = low_level_execute(current_ast->get_kid(0), env);
  Value RHS = low_level_execute(current_ast->get_kid(1), env);
  int LHS_value = LHS.get_ival();
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