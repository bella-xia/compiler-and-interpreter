#include <cassert>
#include <algorithm>
#include <utility>
#include <map>
#include "grammar_symbols.h"
#include "parse.tab.h"
#include "node.h"
#include "ast.h"
#include "exceptions.h"
#include "semantic_analysis.h"

SemanticAnalysis::SemanticAnalysis()
    : m_global_symtab(new SymbolTable(nullptr))
{
  m_cur_symtab = m_global_symtab;
}

SemanticAnalysis::~SemanticAnalysis()
{
  delete m_global_symtab;
}

void SemanticAnalysis::visit_struct_type(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 1);
  Node *struct_identifier = n->get_kid(0);
  assert(struct_identifier->get_tag() == TOK_IDENT);
  std::string struct_name = struct_identifier->get_str();
  Symbol *struct_symbol = m_cur_symtab->lookup_recursive("struct " + struct_name);
  if (struct_symbol == nullptr)
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "unable to find struct instance '%s'", ("struct " + struct_name).c_str());
  }
  else
  {
    n->set_symbol(struct_symbol);
  }
}

void SemanticAnalysis::visit_union_type(Node *n)
{
  RuntimeError::raise("union types aren't supported");
}

void SemanticAnalysis::visit_variable_declaration(Node *n)
{
  assert(n->get_num_kids() == 3);
  // first node : storage
  Node *storage = n->get_kid(0);
  int storage_tag = storage->get_tag();
  assert(storage_tag == TOK_STATIC || storage_tag == TOK_EXTERN ||
         storage_tag == TOK_AUTO || storage_tag == TOK_UNSPECIFIED_STORAGE);
  // second node : basic type
  Node *type_specifier = n->get_kid(1);
  int tag = type_specifier->get_tag();
  switch (tag)
  {
  case AST_BASIC_TYPE:
    visit_basic_type(type_specifier);
    break;
  case AST_STRUCT_TYPE:
    visit_struct_type(type_specifier);
    break;
  default:
    break;
  }
  std::shared_ptr<Type> type_ptr(std::shared_ptr<Type>(type_specifier->get_type()));
  // third node : variation declarator list
  Node *declare_list = n->get_kid(2);
  for (auto i = declare_list->cbegin(); i != declare_list->cend(); ++i)
  {
    visit_declarator_item(*i, type_ptr);
    std::string var_name = (*i)->get_str();
    std::shared_ptr<Type> var_type((*i)->get_type());
    Symbol *defined_symbol = m_cur_symtab->lookup_local(var_name);
    if (defined_symbol != nullptr)
    {
      std::shared_ptr<Type> defined_type(defined_symbol->get_type());
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Double declaration of variable '%s': previously as type '%s' and now as '%s' ",
                           var_name.c_str(), defined_type->as_str().c_str(), var_type->as_str().c_str());
    }
    m_cur_symtab->define(SymbolKind::VARIABLE, var_name, var_type);
  }
}

void SemanticAnalysis::visit_basic_type(Node *n)
{
  std::vector<int> types{-1, -1, -1, -1, -1};

  for (auto i = n->cbegin(); i != n->cend(); ++i)
  {
    int m_tag = (*i)->get_tag();
    switch (m_tag)
    {
    // the first index stores basic type
    case NODE_TOK_VOID:
    case NODE_TOK_INT:
    case NODE_TOK_CHAR:
      // if the current first idx is empty, stores the corresponding node
      if (types[0] == -1)
      {
        types[0] = m_tag;
      }
      else
      {
        leave_non_global_scopes();
        SemanticError::raise(n->get_loc(), "Double / conflicting declaration of basic type void / int / char");
        // throw error bc more than one type is given
      }
      break;
      // the second index stores short / long
    case NODE_TOK_SHORT:
    case NODE_TOK_LONG:
      // if the current first idx is empty, stores the corresponding node
      if (types[1] == -1)
      {
        types[1] = m_tag;
      }
      else
      {
        if (types[1] == m_tag)
        {
          std::string keyword = m_tag == NODE_TOK_SHORT ? "short" : "long";
          leave_non_global_scopes();
          SemanticError::raise(n->get_loc(), "Double declaration of keyword '%s'", keyword.c_str());
        }
        else
        {
          leave_non_global_scopes();
          SemanticError::raise(n->get_loc(), "Conflicting declaration of keyword 'short' and 'long'");
        }
        // throw error bc more than one type is given
      }
      break;
      // the third index stores signed / unsigned
    case NODE_TOK_UNSIGNED:
    case NODE_TOK_SIGNED:
      if (types[2] == -1)
      {
        types[2] = m_tag;
      }
      else
      {
        if (types[1] == m_tag)
        {
          std::string keyword = m_tag == NODE_TOK_SIGNED ? "signed" : "unsigned";
          leave_non_global_scopes();
          SemanticError::raise(n->get_loc(), "Double declaration of keyword '%s'", keyword.c_str());
        }
        else
        {
          leave_non_global_scopes();
          SemanticError::raise(n->get_loc(), "Conflicting declaration of keyword 'signed' and 'unsigned'");
        }
        // throw error bc more than one type is given
      }
      break;
    case NODE_TOK_CONST:
      if (types[3] == NODE_TOK_CONST || types[4] == NODE_TOK_CONST)
      {
        leave_non_global_scopes();
        SemanticError::raise(n->get_loc(), "Double declaration of qualifier 'const'");
        // throw error bc double declaration of const
      }
      else if (types[3] == NODE_TOK_VOLATILE)
      {
        types[4] = NODE_TOK_CONST;
      }
      else
      {
        types[3] = NODE_TOK_CONST;
      }
      break;
    case NODE_TOK_VOLATILE:
      if (types[3] == NODE_TOK_VOLATILE || types[4] == NODE_TOK_VOLATILE)
      {
        leave_non_global_scopes();
        SemanticError::raise(n->get_loc(), "Double declaration of qualifier 'volatile'");
        // throw error bc double declaration of const
      }
      else if (types[3] == NODE_TOK_CONST)
      {
        types[4] = NODE_TOK_VOLATILE;
      }
      else
      {
        types[3] = NODE_TOK_VOLATILE;
      }
      break;
    default:
      break;
    }
  }
  BasicTypeKind kind;
  // start checking type
  if (types[0] == NODE_TOK_VOID)
  {
    if (types[1] != -1 || types[2] != -1)
    {
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Invalid keyword for basic type void");
      // throw error: undefined specifier for void
    }
    else if (types[3] != -1)
    {
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Invalid qualifier for basic type void");
    }
    // assign kind = VOID
    kind = BasicTypeKind::VOID;
  }
  else if (types[0] == -1 || types[0] == NODE_TOK_INT)
  {
    if (types[1] != -1)
    {
      kind = types[1] == NODE_TOK_LONG ? BasicTypeKind::LONG : BasicTypeKind::SHORT;
    }
    else if (types[0] == -1 && types[2] == -1)
    {
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Empty basic type");
      // throw error : empty base type
    }
    else
    {
      kind = BasicTypeKind::INT;
    }
  }
  else
  {
    if (types[1] != -1)
    {
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Invalid keyword 'long' / 'short' for basic type char");
      // throw error: char does not have long / short
    }
    else
    {
      kind = BasicTypeKind::CHAR;
    }
  }
  bool sign = (types[2] == NODE_TOK_UNSIGNED) ? 0 : 1;
  // then wrap it inside qualifier if necessary
  if (types[3] == -1 && types[4] == -1)
  {
    n->set_type(std::shared_ptr<Type>(new BasicType(kind, sign)));
  }
  else if (types[4] != -1)
  {
    TypeQualifier q1 = (types[4] == NODE_TOK_CONST) ? TypeQualifier::CONST : TypeQualifier::VOLATILE;
    TypeQualifier q2 = (types[3] == NODE_TOK_CONST) ? TypeQualifier::CONST : TypeQualifier::VOLATILE;
    std::shared_ptr<Type> qualifier_lev_1(new QualifiedType(std::shared_ptr<Type>(new BasicType(kind, sign)), q1));
    std::shared_ptr<Type> qualifier_lev_2(new QualifiedType(qualifier_lev_1, q2));
    n->set_type(qualifier_lev_2);
  }
  else
  {
    TypeQualifier q = (types[3] == NODE_TOK_CONST) ? TypeQualifier::CONST : TypeQualifier::VOLATILE;
    std::shared_ptr<Type> qualifier(new QualifiedType(std::shared_ptr<Type>(new BasicType(kind, sign)), q));
    n->set_type(qualifier);
  }
}

void SemanticAnalysis::visit_function_definition(Node *n)
{
  int nkid = n->get_num_kids();
  // return type, identifier, function parameter list, statement list
  assert(nkid == 4);
  // function identifier
  assert(n->get_kid(1)->get_tag() == TOK_IDENT);
  const std::string func_name = n->get_kid(1)->get_str();
  Symbol *func_symbol = m_cur_symtab->lookup_local(func_name);
  bool prev_declared = false;
  if (func_symbol != nullptr)
  {
    if (!func_symbol->get_type()->is_function())
    {
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Double declaration of variable '%s': previously as type '%s' and now as function ",
                           func_name.c_str(), func_symbol->get_type()->as_str().c_str());
    }
    if (func_symbol->is_defined())
    {
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Double definition of function '%s'", func_name.c_str());
    }
    visit(n->get_kid(0));
    std::shared_ptr<Type> return_type(n->get_kid(0)->get_type());
    if (!std::shared_ptr<Type>(func_symbol->get_type()->get_base_type())->is_same(return_type.get()))
    {
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Definition of function '%s' with different return type than declared function. Expected '%s' but defined as '%s'",
                           func_name.c_str(),
                           func_symbol->get_type()->get_base_type()->as_str().c_str(),
                           return_type->as_str().c_str());
    }
    prev_declared = true;
  }
  else
  {
    visit_function_declaration(n);
  }
  func_symbol = m_cur_symtab->lookup_local(func_name);
  func_symbol->set_is_defined(true);
  std::shared_ptr<Type> func_type_ptr(func_symbol->get_type());
  // members are the parameters
  Node *func_members = n->get_kid(2);
  assert(func_members->get_tag() == AST_FUNCTION_PARAMETER_LIST);
  // function parameter level scope
  enter_scope();
  int idx = 0;
  int declare_param = func_type_ptr->get_num_members();
  int define_param = func_members->get_num_kids();
  if (declare_param != define_param)
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Definition of function '%s' with different parameter number than declared function, expected %d but defined as %d",
                         func_name.c_str(),
                         declare_param,
                         define_param);
  }
  for (auto i = func_members->cbegin(); i != func_members->cend(); i++)
  {
    // function parameter level scope
    assert((*i)->get_tag() == AST_FUNCTION_PARAMETER);
    if (prev_declared)
    {
      visit(*i);
    }
    SymbolKind param_kind;
    std::shared_ptr<Type> param_type((*i)->get_type());
    if (!param_type->is_same(func_type_ptr->get_member(idx).get_type().get()))
    {
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Definition of function '%s' with different parameter than declared function, expected '%s' but defined as '%s'",
                           func_name.c_str(),
                           func_type_ptr->get_member(idx).get_type()->as_str().c_str(),
                           param_type->as_str().c_str());
    }
    idx++;
    if (param_type->is_struct())
    {
      param_kind = SymbolKind::TYPE;
    }
    else if (param_type->is_function())
    {
      param_kind = SymbolKind::FUNCTION;
    }
    else
    {
      param_kind = SymbolKind::VARIABLE;
    }
    if (m_cur_symtab->lookup_local((*i)->get_str()) != nullptr)
    {
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Double declaration of variable %s in function '%s'",
                           (*i)->get_str().c_str(),
                           func_name.c_str());
    }

    m_cur_symtab->define(param_kind, (*i)->get_str(), param_type);
  }

  Node *stmt_list = n->get_kid(3);
  assert(stmt_list->get_tag() == AST_STATEMENT_LIST);
  enter_scope();
  m_cur_symtab->set_fn_type(std::shared_ptr<Type>(
      m_cur_symtab->lookup_recursive(func_name)->get_type()));
  visit_statement_list(stmt_list);
  leave_scope();
  leave_scope();
  // TODO: implement
}

void SemanticAnalysis::visit_function_declaration(Node *n)
{
  int nkid = n->get_num_kids();
  // return type, identifier, function parameter list
  assert((nkid == 3 && n->get_tag() == AST_FUNCTION_DECLARATION) ||
         (nkid == 4 && n->get_tag() == AST_FUNCTION_DEFINITION));
  // base type is the return type
  Node *base_type = n->get_kid(0);
  visit(base_type);
  std::shared_ptr<Type> return_type(base_type->get_type());
  std::shared_ptr<Type> func_type(new FunctionType(return_type));

  // function identifier
  assert(n->get_kid(1)->get_tag() == TOK_IDENT);
  const std::string func_name = n->get_kid(1)->get_str();
  Symbol *func_symbol = m_cur_symtab->lookup_local(func_name);
  if (func_symbol != nullptr)
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Double declaration of function '%s'", func_name.c_str());
  }
  // members are the parameters
  Node *func_members = n->get_kid(2);
  assert(func_members->get_tag() == AST_FUNCTION_PARAMETER_LIST);
  enter_scope();
  for (auto i = func_members->cbegin(); i != func_members->cend(); i++)
  {
    // function parameter level scope
    assert((*i)->get_tag() == AST_FUNCTION_PARAMETER);
    visit_function_parameter(*i);
    func_type->add_member(Member((*i)->get_str(), (*i)->get_type()));
  }
  leave_scope();
  func_symbol = m_cur_symtab->declare(SymbolKind::FUNCTION, func_name, func_type);
  n->set_symbol(func_symbol);
  // TODO: implement
}

void SemanticAnalysis::visit_function_parameter(Node *n)
{
  assert(n->get_num_kids() == 2);
  // first node : basic type
  Node *declare_type = n->get_kid(0);
  assert(declare_type->get_tag() == AST_BASIC_TYPE || declare_type->get_tag() == AST_STRUCT_TYPE);
  if (declare_type->get_tag() == AST_BASIC_TYPE)
  {
    visit_basic_type(declare_type);
  }
  else
  {
    visit_struct_type(declare_type);
  }
  std::shared_ptr<Type> declare_type_ptr = declare_type->get_type();

  // second node : variation declarator list
  Node *declarator = n->get_kid(1);
  visit_declarator_item(declarator, declare_type_ptr);
  std::string var_name = declarator->get_str();
  std::shared_ptr<Type> var_type(declarator->get_type());
  n->set_str(var_name);
  n->set_type(var_type);
}

// TODO: solution

void SemanticAnalysis::visit_statement_list(Node *n)
{
  visit_children(n);
  // TODO: implement
}

void SemanticAnalysis::visit_return_expression_statement(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 1);
  Node *return_node = n->get_kid(0);
  visit(return_node);
  std::shared_ptr<Type> actual_return_type(return_node->get_type());
  std::shared_ptr<Type> expected_return_type(m_cur_symtab->get_fn_type()->get_base_type());
  if ((actual_return_type->is_int_variations() && expected_return_type->is_int_variations()) ||
      actual_return_type->is_same(expected_return_type.get()))
  {
    return;
  }
  leave_non_global_scopes();
  SemanticError::raise(n->get_loc(), "Incompatible return type. Expected '%s' but '%s' is returned",
                       expected_return_type->as_str().c_str(), actual_return_type->as_str().c_str());
  // TODO: implement
}

void SemanticAnalysis::visit_struct_type_definition(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 2);
  const std::string struct_name = n->get_kid(0)->get_str();
  Node *field_define_list = n->get_kid(1);
  assert(field_define_list->get_tag() == AST_FIELD_DEFINITION_LIST);
  std::shared_ptr<Type> type(new StructType(struct_name));
  Symbol *defined_symbol = m_cur_symtab->lookup_local("struct " + struct_name);
  if (defined_symbol != nullptr)
  {
    std::shared_ptr<Type> defined_type(defined_symbol->get_type());
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Double declaration of variable '%s': previously as type '%s' and now as '%s' ",
                         struct_name.c_str(), defined_type->as_str().c_str(), type->as_str().c_str());
  }
  Symbol *cur_symbol = m_cur_symtab->define(SymbolKind::TYPE, "struct " + struct_name, type);
  n->set_symbol(cur_symbol);
  enter_scope();
  for (auto i = field_define_list->cbegin(); i != field_define_list->cend(); ++i)
  {
    assert((*i)->get_tag() == AST_VARIABLE_DECLARATION);
    visit_variable_declaration(*i);
  }
  for (auto i = m_cur_symtab->cbegin(); i != m_cur_symtab->cend(); ++i)
  {
    Member cur_mem = Member((*i)->get_name(), (*i)->get_type());
    type->add_member(cur_mem);
  }
  leave_scope();
}

void SemanticAnalysis::visit_binary_expression(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 3);
  Node *operation = n->get_kid(0);
  int operator_tag = operation->get_tag();
  switch (operator_tag)
  {
  case TOK_ASSIGN:
    visit_assign_operation(n);
    break;
  case TOK_LT:
  case TOK_GT:
  case TOK_LTE:
  case TOK_GTE:
  case TOK_EQUALITY:
  case TOK_INEQUALITY:
    visit_relational_operation(n);
    break;
  case TOK_PLUS:
  case TOK_MINUS:
    visit_plus_minus_operation(n);
    break;
  case TOK_MOD:
  case TOK_DIVIDE:
  case TOK_ASTERISK:
    visit_other_arithmetic_operation(n);
    break;
  case TOK_LOGICAL_AND:
  case TOK_LOGICAL_OR:
    visit_logical_operation(n);
    break;
  default:
    break;
  }
  // TODO: implement
}

void SemanticAnalysis::visit_unary_expression(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 2);
  int unary_tag = n->get_kid(0)->get_tag();
  Node *var = n->get_kid(1);
  visit(var);
  std::shared_ptr<Type> var_type(var->get_type());
  // before * and & cannot be literal values
  if ((unary_tag == TOK_ASTERISK || unary_tag == TOK_AMPERSAND) && !var->check_is_lvalue())
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Invalid use of unary symbol '%c' before a literal value",
                         (unary_tag == TOK_ASTERISK) ? '*' : '&');
  }
  switch (unary_tag)
  {
  case TOK_ASTERISK:
    if (!var_type->is_pointer())
    {
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Invalid use of unary symbol '*' before a non-pointer type");
    }
    n->set_type(var_type->get_base_type());
    break;
  case TOK_AMPERSAND:
    n->set_type(std::shared_ptr<Type>(new PointerType(var_type)));
    break;
  case TOK_MINUS:
    if (!var_type->is_int_variations())
    {
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Invalid use of unary operation '-' before type '%s'",
                           var_type->as_str().c_str());
    }
    else
    {
      n->set_type(std::shared_ptr<Type>(new BasicType(
          (var_type->get_basic_type_kind() < BasicTypeKind::INT) ? BasicTypeKind::INT : var_type->get_basic_type_kind(),
          var_type->is_signed())));
    }
    break;
  case TOK_NOT:
    if (!var_type->is_int_variations())
    {
      leave_non_global_scopes();
      SemanticError::raise(n->get_loc(), "Invalid use of unary operation '!' before type '%s'",
                           var_type->as_str().c_str());
    }
    else
    {
      n->set_type(std::shared_ptr<Type>(new BasicType(BasicTypeKind::INT, false)));
    }
    break;
  default:
    break;
  }
  // TODO: implement
}

void SemanticAnalysis::visit_postfix_expression(Node *n)
{
  RuntimeError::raise("postfix expressions aren't supported");
}

void SemanticAnalysis::visit_conditional_expression(Node *n)
{
  RuntimeError::raise("conditional expressions aren't supported");
}

void SemanticAnalysis::visit_cast_expression(Node *n)
{
  int nkids = n->get_num_kids();
  assert(nkids == 2);
  Node *basic_type = n->get_kid(0);
  assert(basic_type->get_tag() == AST_BASIC_TYPE);
  visit(basic_type);
  std::shared_ptr<Type> cast_type(basic_type->get_type());
  Node *var = n->get_kid(1);
  assert(var->get_tag() == AST_VARIABLE_REF);
  visit(var);
  std::shared_ptr<Type> var_type(var->get_type());
  bool compatible = false;
  if ((cast_type->is_int_variations() && var_type->is_int_variations()) || cast_type->is_same(var_type.get()))
  {
    compatible = true;
  }
  else if ((cast_type->is_const() || cast_type->is_volatile()) && (var_type->is_int_variations() || var_type->is_const() || var_type->is_volatile()))
  {
    compatible = true;
  }
  if (compatible)
  {
    n->set_type(std::shared_ptr<Type>(cast_type));
  }
  else
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Incompatible types: attempting to cast type '%s' to '%s' ",
                         var_type->as_str().c_str(), cast_type->as_str().c_str());
  }
  // TODO: implement
}

void SemanticAnalysis::visit_function_call_expression(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 2);
  Node *func_ref = n->get_kid(0);
  assert(func_ref->get_tag() == AST_VARIABLE_REF);
  visit(func_ref);
  Symbol *func_symbol = func_ref->get_symbol();
  std::shared_ptr<Type> func_type(func_symbol->get_type());
  if (!func_type->is_function())
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "variable '%s' is not a function ",
                         func_symbol->get_name().c_str());
  }
  // if (!func_symbol->is_defined())
  // {
  //   SemanticError::raise(n->get_loc(), "function '%s' is not defined ",
  //                        func_symbol->get_name().c_str());
  // }
  Node *arg_list = n->get_kid(1);
  int arg_num = arg_list->get_num_kids();
  int param_num = func_type->get_num_members();
  if (arg_num != param_num)
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "function call to '%s' has incorrect number of parameters: expected %d, but get %d",
                         func_symbol->get_name().c_str(),
                         param_num, arg_num);
  }
  visit(arg_list);
  int idx = 0;
  for (auto i = arg_list->cbegin(); i != arg_list->cend(); ++i)
  {
    std::shared_ptr<Type> arg((*i)->get_type());
    std::shared_ptr<Type> param(func_type->get_member(idx).get_type());
    if (param->is_array())
    {
      apply_assignment_rule(n, std::shared_ptr<Type>(new PointerType(param->get_base_type())), arg);
    }
    else
    {
      apply_assignment_rule(n, param, arg);
    }
    idx++;
  }
  n->set_type(std::shared_ptr<Type>(func_type->get_base_type()));
  // TODO: implement
}

void SemanticAnalysis::visit_field_ref_expression(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 2);
  Node *struct_ref = n->get_kid(0);
  Node *component_ident = n->get_kid(1);
  assert(component_ident->get_tag() == TOK_IDENT);
  visit_children(n);
  std::shared_ptr<Type> struct_type(struct_ref->get_type());
  if (!struct_type->is_struct())
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Expect a struct for field reference but instead got '%s' ",
                         struct_type->as_str().c_str());
  }
  const Member *component_member = struct_type->find_member(component_ident->get_str());
  n->set_type(std::shared_ptr<Type>(component_member->get_type()));
  // TODO: implement
}

void SemanticAnalysis::visit_indirect_field_ref_expression(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 2);
  assert(n->get_kid(1)->get_tag() == TOK_IDENT);
  visit_children(n);
  std::shared_ptr<Type> struct_ref_type(n->get_kid(0)->get_type());
  std::string struct_ident = n->get_kid(1)->get_str();
  if (!struct_ref_type->is_pointer() || !struct_ref_type->get_base_type()->is_struct())
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Expect a struct pointer for indirect field reference but instead got '%s' ",
                         struct_ref_type->as_str().c_str());
  }
  const Member *component_member = struct_ref_type->get_base_type()->find_member(struct_ident);
  n->set_type(std::shared_ptr<Type>(component_member->get_type()));
}

void SemanticAnalysis::visit_array_element_ref_expression(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 2);
  // kid 1: needs to be an array (can be a direct variable reference or an unary expression)
  Node *array = n->get_kid(0);
  visit(array);
  std::shared_ptr<Type> array_type(array->get_type());
  if (!array_type->is_array() && !array_type->is_pointer())
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "type %s is given for indexing whereas type array is required", array_type->as_str().c_str());
  }
  // kid 2: int literal or variable reference: need to be integer
  Node *idx = n->get_kid(1);
  visit(idx);
  std::shared_ptr<Type> idx_type(idx->get_type());
  if (!idx_type->is_int_variations())
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "type %s cannot be used as index for array", idx_type->as_str().c_str());
  }
  n->set_type(std::shared_ptr<Type>(array_type->get_base_type()));
  // TODO: implement
}

void SemanticAnalysis::visit_variable_ref(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 1);
  std::string var_name = n->get_kid(0)->get_str();
  Symbol *var_symbol = m_cur_symtab->lookup_recursive(var_name);
  if (var_symbol == nullptr)
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "the variable reference '%s' is never declared or defined ", var_name.c_str());
  }
  n->set_symbol(var_symbol);
  n->set_str(var_name);
  // TODO: implement
}

void SemanticAnalysis::visit_literal_value(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 1);
  n->set_is_lvalue(false);
  Node *literal_val = n->get_kid(0);
  if (literal_val->get_tag() == TOK_INT_LIT)
  {
    // case it is a int literal
    LiteralValue int_lit_val = LiteralValue::from_int_literal(literal_val->get_str(), literal_val->get_loc());
    std::shared_ptr<Type> lit_val_type(new BasicType((int_lit_val.is_long()) ? BasicTypeKind::LONG : BasicTypeKind::INT, !int_lit_val.is_unsigned()));
    n->set_type(lit_val_type);
  }
  else if (literal_val->get_tag() == TOK_CHAR_LIT)
  {
    LiteralValue::from_char_literal(literal_val->get_str(), literal_val->get_loc());
    std::shared_ptr<Type> lit_val_type(std::shared_ptr<Type>(new BasicType(BasicTypeKind::INT, false)));
    n->set_type(lit_val_type);
  }
  else
  {
    // TOK_STRING_LIT
    LiteralValue::from_str_literal(literal_val->get_str(), literal_val->get_loc());
    std::shared_ptr<Type> lit_val_type(std::shared_ptr<Type>(
        new PointerType(std::shared_ptr<Type>(
            new QualifiedType(std::shared_ptr<Type>(
                                  new BasicType(BasicTypeKind::CHAR, true)),
                              TypeQualifier::CONST)))));
    n->set_type(lit_val_type);
  }
  // TODO: implement
}

void SemanticAnalysis::visit_declarator_item(Node *n, std::shared_ptr<Type> type)
{
  int tag = n->get_tag();
  assert(tag == AST_NAMED_DECLARATOR || tag == AST_ARRAY_DECLARATOR || tag == AST_POINTER_DECLARATOR);
  switch (tag)
  {
  case AST_ARRAY_DECLARATOR:
  {
    // two kids: named declarator / array declarator; int size
    assert(n->get_num_kids() == 2);
    visit_declarator_item(n->get_kid(0), std::shared_ptr<Type>(new ArrayType(type, std::stoi(n->get_kid(1)->get_str()))));
    n->set_str(n->get_kid(0)->get_str());
    n->set_type(n->get_kid(0)->get_type());
    break;
  }
  case AST_POINTER_DECLARATOR:
  {
    assert(n->get_num_kids() == 1);
    visit_declarator_item(n->get_kid(0), std::shared_ptr<Type>(new PointerType(type)));
    n->set_str(n->get_kid(0)->get_str());
    n->set_type(n->get_kid(0)->get_type());
    break;
  }
  default:
  {
    // AST_NAMED_DECLARATOR
    assert(n->get_num_kids() == 1);
    std::shared_ptr<Type> cur_type(type);
    n->set_str(n->get_kid(0)->get_str());
    n->set_type(cur_type);
  }
  }
}

void SemanticAnalysis::visit_assign_operation(Node *n)
{
  visit_children(n);
  Node *LHS = n->get_kid(1);
  Node *RHS = n->get_kid(2);
  // check 1: LHS must be lvalue
  if (!LHS->check_is_lvalue())
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "the left hand side of assigment is not a lvalue");
  }
  std::shared_ptr<Type> lsymbol_type(LHS->get_type());
  std::shared_ptr<Type> rsymbol_type(RHS->get_type());
  n->set_type(apply_assignment_rule(n, lsymbol_type, rsymbol_type));
}

void SemanticAnalysis::visit_relational_operation(Node *n)
{
  Node *LHS = n->get_kid(1);
  Node *RHS = n->get_kid(2);
  visit(LHS);
  visit(RHS);
  bool compatible;
  std::shared_ptr<Type> lsymbol_type(LHS->get_type());
  std::shared_ptr<Type> rsymbol_type(RHS->get_type());
  // check 3: if LHS is int
  if (lsymbol_type->is_int_variations() && rsymbol_type->is_int_variations())
  {
    compatible = true;
  }
  else if (lsymbol_type->is_int_variations() || rsymbol_type->is_int_variations())
  {
    compatible = false;
  }
  // check 4: if LHS is pointer
  else if (lsymbol_type->is_pointer() || lsymbol_type->is_struct())
  {
    if (lsymbol_type->is_same(rsymbol_type.get()))
    {
      compatible = true;
    }
    else
    {
      compatible = false;
    }
  }
  else
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "%s type symbol cannot be compared", lsymbol_type->as_str().c_str());
  }
  if (!compatible)
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Incompatible types (%s, %s) in comparison", lsymbol_type->as_str().c_str(),
                         rsymbol_type.get()->as_str().c_str());
  }
  else
  {
    n->set_type(std::shared_ptr<Type>(new BasicType(BasicTypeKind::INT, false)));
  }
}

void SemanticAnalysis::visit_plus_minus_operation(Node *n)
{
  Node *LHS = n->get_kid(1);
  Node *RHS = n->get_kid(2);
  visit(LHS);
  visit(RHS);
  std::shared_ptr<Type> lsymbol_type(LHS->get_type());
  std::shared_ptr<Type> rsymbol_type(RHS->get_type());
  // check 3: if LHS is int
  if (lsymbol_type->is_int_variations() && rsymbol_type->is_int_variations())
  {
    n->set_type(get_priority_type(lsymbol_type, rsymbol_type));
    n->set_is_lvalue(false);
    return;
  }
  else if (lsymbol_type->is_pointer() && rsymbol_type->is_int_variations())
  {
    n->set_type(std::shared_ptr<Type>(lsymbol_type));
    n->set_is_lvalue(false);
    return;
  }
  // check 4: if LHS is pointer
  else
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "data types '%s' and '%s' cannot perform arithmatic operations + or -",
                         lsymbol_type->as_str().c_str(), rsymbol_type->as_str().c_str());
  }
}

void SemanticAnalysis::visit_other_arithmetic_operation(Node *n)
{
  Node *LHS = n->get_kid(1);
  Node *RHS = n->get_kid(2);
  visit(LHS);
  visit(RHS);
  std::shared_ptr<Type> lsymbol_type(LHS->get_type());
  std::shared_ptr<Type> rsymbol_type(RHS->get_type());
  // check 3: if LHS is int
  if (lsymbol_type->is_int_variations() && rsymbol_type->is_int_variations())
  {
    n->set_type(get_priority_type(lsymbol_type, rsymbol_type));
    n->set_is_lvalue(false);
    return;
  }
  else
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "data types '%s' and '%s' cannot perform arithmatic operations *, / or %%",
                         lsymbol_type->as_str().c_str(), rsymbol_type->as_str().c_str());
  }
}

void SemanticAnalysis::visit_logical_operation(Node *n)
{
  Node *LHS = n->get_kid(1);
  Node *RHS = n->get_kid(2);
  visit(LHS);
  visit(RHS);
  std::shared_ptr<Type> lsymbol_type(LHS->get_type());
  std::shared_ptr<Type> rsymbol_type(RHS->get_type());
  // check 3: if LHS is int
  if (lsymbol_type->is_int_variations() && rsymbol_type->is_int_variations())
  {
    n->set_type(std::shared_ptr<Type>(new BasicType(BasicTypeKind::INT, false)));
    n->set_is_lvalue(false);
    return;
  }
  else
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "data types '%s' and '%s' cannot perform logical operations || or &&",
                         lsymbol_type->as_str().c_str(), rsymbol_type->as_str().c_str());
  }
}

std::shared_ptr<Type> SemanticAnalysis::get_priority_type(const std::shared_ptr<Type> &type1, const std::shared_ptr<Type> &type2)
{
  assert(type1->is_int_variations() && type2->is_int_variations());
  bool sign = (type1->is_signed() && type2->is_signed());
  // char short int long
  std::vector<int> int_types{0, 0, 0, 0};
  int_types[(int)type1->get_basic_type_kind()]++;
  int_types[(int)type2->get_basic_type_kind()]++;
  if (int_types[3] == 0)
  {
    // both types less precise than int
    return std::shared_ptr<Type>(new BasicType(BasicTypeKind::INT, sign));
  }
  else
  {
    // if at least one is long
    return std::shared_ptr<Type>(new BasicType(BasicTypeKind::LONG, sign));
  }
}

void SemanticAnalysis::visit_while_statement(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 2);

  Node *condition = n->get_kid(0);
  enter_scope();
  visit(condition);
  if (!condition->get_type()->is_int_variations() && !condition->get_type()->is_pointer())
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Unable to evaluate the statement inside while conditional because it is of type '%s' ",
                         condition->get_type()->as_str().c_str());
  }
  leave_scope();
  Node *stmt_list = n->get_kid(1);
  assert(stmt_list->get_tag() == AST_STATEMENT_LIST);
  enter_scope();
  visit(stmt_list);

  leave_scope();
}

void SemanticAnalysis::visit_do_while_statement(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 2);

  Node *stmt_list = n->get_kid(0);
  assert(stmt_list->get_tag() == AST_STATEMENT_LIST);
  enter_scope();
  visit(stmt_list);
  leave_scope();
  Node *condition = n->get_kid(1);
  enter_scope();
  visit(condition);
  if (!condition->get_type()->is_int_variations() && !condition->get_type()->is_pointer())
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Unable to evaluate the statement inside while conditional because it is of type '%s' ",
                         condition->get_type()->as_str().c_str());
  }
  leave_scope();
}

void SemanticAnalysis::visit_for_statement(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 4);
  Node *init = n->get_kid(0);
  Node *condition = n->get_kid(1);
  Node *increment = n->get_kid(2);
  Node *stmt_list = n->get_kid(3);
  assert(stmt_list->get_tag() == AST_STATEMENT_LIST);
  // enter_scope();
  visit(init);
  visit(condition);
  if (!condition->get_type()->is_int_variations() && !condition->get_type()->is_pointer())
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Unable to evaluate the statement inside for conditional because it is of type '%s' ",
                         condition->get_type()->as_str().c_str());
  }
  enter_scope();
  visit(stmt_list);
  // leave_scope();
  leave_scope();
  visit(increment);
}

void SemanticAnalysis::visit_if_statement(Node *n)
{
  int nkid = n->get_num_kids();
  assert(nkid == 2 || nkid == 3);
  Node *condition = n->get_kid(0);
  Node *if_stmt = n->get_kid(1);
  assert(if_stmt->get_tag() == AST_STATEMENT_LIST);

  visit(condition);
  if (!condition->get_type()->is_int_variations() && !condition->get_type()->is_pointer())
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Unable to evaluate the statement inside if conditional because it is of type '%s' ",
                         condition->get_type()->as_str().c_str());
  }
  enter_scope();
  visit(if_stmt);

  leave_scope();
}
void SemanticAnalysis::visit_if_else_statement(Node *n)
{
  visit_if_statement(n);
  Node *else_stmt = n->get_kid(2);
  assert(else_stmt->get_tag() == AST_STATEMENT_LIST);

  enter_scope();
  visit(else_stmt);
  leave_scope();
}

std::shared_ptr<Type> SemanticAnalysis::apply_assignment_rule(Node *n, std::shared_ptr<Type> lsymbol_type, std::shared_ptr<Type> rsymbol_type)
{
  bool compatible;
  if (lsymbol_type->is_const())
  {
    compatible = false;
  }
  // check 3: if LHS is int
  else if (lsymbol_type->is_int_variations())
  {
    if (!rsymbol_type->is_int_variations())
    {
      compatible = false;
    }
    else
    {
      compatible = true;
    }
  }
  else if (lsymbol_type->is_struct())
  {
    if (lsymbol_type->is_same(rsymbol_type.get()))
    {
      compatible = true;
    }
    else
    {
      compatible = false;
    }
  }
  // check 4: if LHS is pointer
  else if (lsymbol_type->is_pointer())
  {
    if (rsymbol_type->is_array())
    {
      return apply_assignment_rule(n, lsymbol_type->get_base_type(), rsymbol_type->get_base_type());
    }
    else if (rsymbol_type->is_pointer())
    {
      std::shared_ptr<Type> lsymbol_base_type(lsymbol_type);
      std::shared_ptr<Type> rsymbol_base_type(rsymbol_type);
      while (lsymbol_base_type->is_pointer() && rsymbol_base_type->is_pointer())
      {
        lsymbol_base_type = lsymbol_base_type->get_base_type();
        rsymbol_base_type = rsymbol_base_type->get_base_type();
      }

      if (lsymbol_base_type->is_same(rsymbol_base_type.get()) &&
          (lsymbol_base_type->is_const() == rsymbol_base_type->is_const()) &&
          (lsymbol_base_type->is_volatile() == rsymbol_base_type->is_volatile()))
      {
        compatible = true;
      }
      else if ((lsymbol_base_type->is_const() && lsymbol_base_type->is_volatile()) &&
               (rsymbol_base_type->is_basic() || rsymbol_base_type->is_const() || rsymbol_base_type->is_volatile()))
      {
        compatible = true;
      }
      else if ((lsymbol_base_type->is_const() || lsymbol_base_type->is_volatile()) &&
               (rsymbol_base_type->is_basic()))
      {
        compatible = true;
      }
      else
      {
        compatible = false;
      }
    }
    else
    {
      compatible = false;
    }
  }
  else
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "%s type symbol cannot be assigned", lsymbol_type->as_str().c_str());
  }
  if (!compatible)
  {
    leave_non_global_scopes();
    SemanticError::raise(n->get_loc(), "Incompatible types (%s, %s) in assignment", lsymbol_type->as_str().c_str(),
                         rsymbol_type.get()->as_str().c_str());
  }
  return std::shared_ptr<Type>(lsymbol_type);
}

void SemanticAnalysis::enter_scope()
{
  SymbolTable *scope = new SymbolTable(m_cur_symtab);
  m_cur_symtab = scope;
}

void SemanticAnalysis::leave_scope()
{
  SymbolTable *old_scope = m_cur_symtab;
  m_cur_symtab = m_cur_symtab->get_parent();
  delete old_scope;
  assert(m_cur_symtab != nullptr);
}

void SemanticAnalysis::leave_non_global_scopes()
{
  while (m_cur_symtab->get_parent() != nullptr)
  {
    SymbolTable *old_scope = m_cur_symtab;
    m_cur_symtab = m_cur_symtab->get_parent();
    delete old_scope;
  }
}

// TODO: implement helper functions
