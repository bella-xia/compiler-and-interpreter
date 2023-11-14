#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

#include <cstdint>
#include <memory>
#include <utility>
#include "type.h"
#include "symtab.h"
#include "ast_visitor.h"

class SemanticAnalysis : public ASTVisitor
{
private:
  SymbolTable *m_global_symtab, *m_cur_symtab;
  std::vector<SymbolTable *> &m_symtabs;
  std::map<std::string, Node *> *m_string_literal_map;

public:
  SemanticAnalysis(std::vector<SymbolTable *> &symtabs);
  virtual ~SemanticAnalysis();

  SymbolTable *get_global_symtab() { return m_global_symtab; }
  std::map<std::string, Node *> *get_string_literal_map() { return m_string_literal_map; }

  virtual void visit_struct_type(Node *n);
  virtual void visit_union_type(Node *n);
  virtual void visit_variable_declaration(Node *n);
  virtual void visit_basic_type(Node *n);
  virtual void visit_function_definition(Node *n);
  virtual void visit_function_declaration(Node *n);
  virtual void visit_function_parameter(Node *n);
  virtual void visit_statement_list(Node *n);
  virtual void visit_return_expression_statement(Node *n);
  virtual void visit_struct_type_definition(Node *n);
  virtual void visit_binary_expression(Node *n);
  virtual void visit_unary_expression(Node *n);
  virtual void visit_postfix_expression(Node *n);
  virtual void visit_conditional_expression(Node *n);
  virtual void visit_cast_expression(Node *n);
  virtual void visit_function_call_expression(Node *n);
  virtual void visit_field_ref_expression(Node *n);
  virtual void visit_indirect_field_ref_expression(Node *n);
  virtual void visit_array_element_ref_expression(Node *n);
  virtual void visit_variable_ref(Node *n);
  virtual void visit_literal_value(Node *n);
  virtual void visit_while_statement(Node *n);
  virtual void visit_do_while_statement(Node *n);
  virtual void visit_for_statement(Node *n);
  virtual void visit_if_statement(Node *n);
  virtual void visit_if_else_statement(Node *n);

private:
  void visit_declarator_item(Node *n, std::shared_ptr<Type> type);
  void enter_scope();
  void leave_scope();
  void leave_all_scopes();

  void visit_assign_operation(Node *n);
  void visit_relational_operation(Node *n);
  void visit_plus_minus_operation(Node *n);
  void visit_other_arithmetic_operation(Node *n);
  void visit_logical_operation(Node *n);
  std::shared_ptr<Type> apply_assignment_rule(Node *n, std::shared_ptr<Type> lsymbol_type, std::shared_ptr<Type> rsymbol_type);
  std::shared_ptr<Type> get_priority_type(const std::shared_ptr<Type> &type1, const std::shared_ptr<Type> &type2);
  // TODO: add helper functions
};

#endif // SEMANTIC_ANALYSIS_H
