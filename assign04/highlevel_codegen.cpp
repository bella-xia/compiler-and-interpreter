#include <cassert>
#include "node.h"
#include "instruction.h"
#include "highlevel.h"
#include "ast.h"
#include <iostream>
#include "parse.tab.h"
#include "grammar_symbols.h"
#include "exceptions.h"
#include "local_storage_allocation.h"
#include "highlevel_codegen.h"

// Adjust an opcode for a basic type
HighLevelOpcode get_opcode(HighLevelOpcode base_opcode, const std::shared_ptr<Type> &type)
{
  if (type->is_basic())
    return static_cast<HighLevelOpcode>(int(base_opcode) + int(type->get_basic_type_kind()));
  else if (type->is_pointer())
    return static_cast<HighLevelOpcode>(int(base_opcode) + int(BasicTypeKind::LONG));
  else
    RuntimeError::raise("attempt to use type '%s' as data in opcode selection", type->as_str().c_str());
}

HighLevelOpcode get_opcode(HighLevelOpcode base_opcode, const std::shared_ptr<Type> &type1, const std::shared_ptr<Type> &type2)
{
  if (!(type1->is_basic() || type1->is_pointer()))
  {
    RuntimeError::raise("attempt to use type '%s' as data in opcode selection", type1->as_str().c_str());
  }
  if (!(type2->is_basic() || type2->is_pointer()))
  {
    RuntimeError::raise("attempt to use type '%s' as data in opcode selection", type2->as_str().c_str());
  }
  int first = type1->get_storage_size();
  int second = type2->get_storage_size();
  switch (first)
  {
  case 1:
    switch (second)
    {
    case 2:
      return base_opcode;
    case 4:
      return static_cast<HighLevelOpcode>((int)base_opcode + 1);
    case 8:
      return static_cast<HighLevelOpcode>((int)base_opcode + 2);
    }
  case 2:
    return (second == 4) ? static_cast<HighLevelOpcode>(
                               (int)base_opcode + 3)
                         : static_cast<HighLevelOpcode>(
                               (int)base_opcode + 4);
  case 4:
    return static_cast<HighLevelOpcode>(
        (int)base_opcode + 5);
  default:
    return base_opcode;
  }
}

Operand HighLevelCodegen::check_types(const Operand &original_op,
                                      const std::shared_ptr<Type> &original, const std::shared_ptr<Type> &dest)
{
  int original_size = original->get_storage_size();
  int dest_size = dest->get_storage_size();
  if (original_size < dest_size)
  {
    if (dest->is_pointer() || dest->is_signed())
    {
      HighLevelOpcode sconv_opcode = get_opcode(HINS_sconv_bw, original, dest);
      Operand dest0(Operand::VREG, next_temp_vreg());
      m_hl_iseq->append(new Instruction(sconv_opcode, dest0, original_op));
      return dest0;
    }
    else
    {
      HighLevelOpcode uconv_opcode = get_opcode(HINS_uconv_bw, original, dest);
      Operand dest0(Operand::VREG, next_temp_vreg());
      m_hl_iseq->append(new Instruction(uconv_opcode, dest0, original_op));
      return dest0;
    }
  }
  return original_op;
}

HighLevelCodegen::HighLevelCodegen(int next_label_num, bool optimize)
    : m_next_label_num(next_label_num), m_optimize(optimize), m_hl_iseq(new InstructionSequence()),
      next_vreg_num(10)
{
}

HighLevelCodegen::~HighLevelCodegen()
{
}

void HighLevelCodegen::visit_function_definition(Node *n)
{
  // generate the name of the label that return instructions should target
  std::string fn_name = n->get_kid(1)->get_str();
  m_return_label_name = ".L" + fn_name + "_return";

  unsigned total_local_storage = 0U;

  total_local_storage = n->get_symbol()->get_stack_size();

  m_hl_iseq->append(new Instruction(HINS_enter, Operand(Operand::IMM_IVAL, total_local_storage)));

  int temp_next_vreg = next_vreg_num;
  next_vreg_num = n->get_symbol()->get_next_vreg();
  // visit parameters
  visit(n->get_kid(2));
  // visit body
  visit(n->get_kid(3));

  m_hl_iseq->define_label(m_return_label_name);
  m_hl_iseq->append(new Instruction(HINS_leave, Operand(Operand::IMM_IVAL, total_local_storage)));
  m_hl_iseq->append(new Instruction(HINS_ret));

  // Perform high-level optimizations?
  if (m_optimize)
  {
  }

  next_vreg_num = temp_next_vreg;
}

void HighLevelCodegen::visit_function_parameter_list(Node *n)
{
  for (auto i = n->cbegin(); i != n->cend(); i++)
  {
    visit(*i);
    if ((*i)->has_operand() && (*i)->has_function_operand())
    {
      // std::shared_ptr<Type> original_type((*i)->get_type());
      // std::shared_ptr<Type> dest_type((*i)->get_function_type());
      // Operand orig_operand((*i)->get_operand());
      // Operand modified_op(check_types(orig_operand, original_type, dest_type));
      HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, (*i)->get_type());
      m_hl_iseq->append(new Instruction(mov_opcode, (*i)->get_operand(), (*i)->get_function_operand()));
    }
  }
  // TODO: implement
}

void HighLevelCodegen::visit_statement_list(Node *n)
{
  // visit_children(n);
  int temp_next_vreg = next_vreg_num;
  for (auto i = n->cbegin(); i != n->cend(); ++i)
  {
    visit(*i);
    next_vreg_num = temp_next_vreg;
  }
  // TODO: implement
}

void HighLevelCodegen::visit_expression_statement(Node *n)
{
  visit_children(n);
  // TODO: implement
}

void HighLevelCodegen::visit_return_statement(Node *n)
{
  // TODO: implement
  m_hl_iseq->append(new Instruction(HINS_jmp, Operand(Operand::LABEL, m_return_label_name)));
}

void HighLevelCodegen::visit_return_expression_statement(Node *n)
{
  // A possible implementation:
  Node *expr = n->get_kid(0);

  // generate code to evaluate the expression
  visit(expr);

  std::shared_ptr<Type> orig_type(expr->get_type());
  std::shared_ptr<Type> dest_type(expr->get_function_type());
  Operand orig_op(expr->get_operand());
  Operand modified_op = check_types(orig_op, orig_type, dest_type);

  // move the computed value to the return value vreg
  HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, dest_type);
  m_hl_iseq->append(new Instruction(mov_opcode, Operand(Operand::VREG, LocalStorageAllocation::VREG_RETVAL), modified_op));

  // jump to the return label
  visit_return_statement(n);
}

void HighLevelCodegen::visit_while_statement(Node *n)
{
  // visit_children(n);
  std::string body_label = next_label();
  std::string condition_label = next_label();

  // first jump to the condition
  m_hl_iseq->append(new Instruction(HINS_jmp, Operand(Operand::LABEL, condition_label)));

  // then append the body
  m_hl_iseq->define_label(body_label);
  Node *body = n->get_kid(1);
  visit(body);

  // the append the condition
  m_hl_iseq->define_label(condition_label);
  Node *condition = n->get_kid(0);
  visit(condition);
  m_hl_iseq->append(new Instruction(HINS_cjmp_t, condition->get_operand(), Operand(Operand::LABEL, body_label)));
  // TODO: implement
}

void HighLevelCodegen::visit_do_while_statement(Node *n)
{
  // visit_children(n);
  std::string body_label = next_label();

  // append the body
  m_hl_iseq->define_label(body_label);
  Node *body = n->get_kid(0);
  visit(body);

  // then append the condition
  Node *condition = n->get_kid(1);
  visit(condition);
  m_hl_iseq->append(new Instruction(HINS_cjmp_t, condition->get_operand(), Operand(Operand::LABEL, body_label)));
  // TODO: implement
}

void HighLevelCodegen::visit_for_statement(Node *n)
{
  // visit_children(n);
  std::string body_label = next_label();
  std::string condition_label = next_label();
  Node *init_condition = n->get_kid(0);
  visit(init_condition);

  // first jump to the condition
  m_hl_iseq->append(new Instruction(HINS_jmp, Operand(Operand::LABEL, condition_label)));

  // then append the body
  m_hl_iseq->define_label(body_label);
  Node *body = n->get_kid(3);
  visit(body);
  Node *continue_condition = n->get_kid(2);
  visit(continue_condition);

  // the append the condition
  m_hl_iseq->define_label(condition_label);
  Node *condition = n->get_kid(1);
  visit(condition);

  m_hl_iseq->append(new Instruction(HINS_cjmp_t, condition->get_operand(), Operand(Operand::LABEL, body_label)));
  // TODO: implement
}

void HighLevelCodegen::visit_if_statement(Node *n)
{
  // visit_children(n);
  std::string next_step_label = next_label();

  // evaluate condition
  Node *condition = n->get_kid(0);
  visit(condition);
  // if false, jump to next step
  m_hl_iseq->append(new Instruction(HINS_cjmp_f, condition->get_operand(), Operand(Operand::LABEL, next_step_label)));
  // else stays here
  Node *body = n->get_kid(1);
  visit(body);
  m_hl_iseq->define_label(next_step_label);

  // TODO: implement
}

void HighLevelCodegen::visit_if_else_statement(Node *n)
{
  // visit_children(n);
  std::string next_step_label = next_label();
  std::string else_label = next_label();

  // evaluate condition
  Node *condition = n->get_kid(0);
  visit(condition);
  // if false, jump to next step
  m_hl_iseq->append(new Instruction(HINS_cjmp_f, condition->get_operand(), Operand(Operand::LABEL, else_label)));
  // else stays here
  Node *if_body = n->get_kid(1);
  visit(if_body);
  m_hl_iseq->append(new Instruction(HINS_jmp, Operand(Operand::LABEL, next_step_label)));

  m_hl_iseq->define_label(else_label);
  Node *else_body = n->get_kid(2);
  visit(else_body);

  m_hl_iseq->define_label(next_step_label);
}

void HighLevelCodegen::visit_binary_expression(Node *n)
{
  visit_children(n);
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
    visit_children(n);
    break;
  }
  // TODO: implement
}

void HighLevelCodegen::visit_unary_expression(Node *n)
{
  int unary_tag = n->get_kid(0)->get_tag();
  Node *var = n->get_kid(1);
  visit(var);
  switch (unary_tag)
  {
  case TOK_ASTERISK:
    // dereference
    {
      HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, n->get_type());
      Operand dest(Operand(Operand::VREG, next_temp_vreg()));
      m_hl_iseq->append(new Instruction(mov_opcode, dest, var->get_operand().to_memref()));
      n->set_operand(dest);
    }
    break;
  case TOK_AMPERSAND:
    // take the address
    {
      n->set_operand(Operand(Operand::VREG, var->get_operand().get_base_reg()));
    }
    break;
  case TOK_MINUS:
  {
    HighLevelOpcode neg_opcode = get_opcode(HINS_neg_b, n->get_type());
    Operand dest(Operand(Operand::VREG, next_temp_vreg()));
    m_hl_iseq->append(new Instruction(neg_opcode, dest, var->get_operand()));
    n->set_operand(dest);
  }
  break;
  case TOK_NOT:
  {
    HighLevelOpcode not_opcode = get_opcode(HINS_not_b, n->get_type());
    Operand dest(Operand(Operand::VREG, next_temp_vreg()));
    m_hl_iseq->append(new Instruction(not_opcode, dest, var->get_operand()));
    n->set_operand(dest);
  }
  break;
    break;
  default:
    break;
  }
  // visit_children(n);
  //  TODO: implement
}

void HighLevelCodegen::visit_function_call_expression(Node *n)
{
  Node *funcall_name = n->get_kid(0);
  visit(funcall_name);
  Node *func_args = n->get_kid(1);
  int start_arg_vreg = 1;
  for (auto i = func_args->cbegin(); i != func_args->cend(); i++)
  {
    visit(*i);
    if ((*i)->has_operand())
    {
      if ((*i)->get_type()->is_array())
      {
        std::shared_ptr<Type> check_type = std::shared_ptr<Type>(new PointerType((*i)->get_type()->get_base_type()));
        // std::shared_ptr<Type> original_type((*i)->get_type());
        std::shared_ptr<Type> dest_type((*i)->get_function_type());
        Operand orig_operand(Operand(Operand::VREG, (*i)->get_operand().get_base_reg()));
        Operand modified_op(check_types(orig_operand, check_type, dest_type));
        HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, dest_type);
        m_hl_iseq->append(new Instruction(mov_opcode, Operand(Operand::VREG, start_arg_vreg), modified_op));
      }
      else
      {
        std::shared_ptr<Type> dest_type((*i)->get_function_type());
        Operand modified_op(check_types((*i)->get_operand(), (*i)->get_type(), dest_type));
        HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, dest_type);
        m_hl_iseq->append(new Instruction(mov_opcode, Operand(Operand::VREG, start_arg_vreg), modified_op));
      }
      start_arg_vreg++;
    }
  }
  m_hl_iseq->append(new Instruction(HINS_call, Operand(Operand::LABEL, funcall_name->get_str())));
  HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, n->get_type());
  Operand dest(Operand::VREG, next_temp_vreg());
  m_hl_iseq->append(new Instruction(mov_opcode, dest, Operand(Operand::VREG, 0)));
  n->set_operand(dest);
  // visit_children(n);

  // TODO: implement
}

void HighLevelCodegen::visit_field_ref_expression(Node *n)
{
  // get struct operand
  Node *struct_var = n->get_kid(0);
  visit(struct_var);
  Operand struct_var_opt = struct_var->get_operand();

  // get struct component
  std::string struct_component_name = n->get_kid(1)->get_str();
  int component_offset = struct_var->get_type()->get_field_offset(struct_component_name);
  std::shared_ptr<Type> component_type_pt(new PointerType(
      std::shared_ptr<Type>(new BasicType(BasicTypeKind::INT, true))));
  HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, component_type_pt);
  Operand dest1(Operand::VREG, next_temp_vreg());
  m_hl_iseq->append(new Instruction(mov_opcode, dest1, Operand(Operand::IMM_IVAL, component_offset)));
  Operand dest2(Operand::VREG, next_temp_vreg());
  HighLevelOpcode add_opcode = get_opcode(HINS_add_b, component_type_pt);
  m_hl_iseq->append(new Instruction(add_opcode, dest2, Operand(Operand::VREG, struct_var_opt.get_base_reg()),
                                    dest1));
  n->set_operand(dest2.to_memref());
  // TODO: implement
}

void HighLevelCodegen::visit_indirect_field_ref_expression(Node *n)
{
  Node *struct_var = n->get_kid(0);
  visit(struct_var);
  Operand struct_var_ptr_opt = struct_var->get_operand();

  // get struct component
  std::string struct_component_name = n->get_kid(1)->get_str();
  int component_offset = struct_var->get_type()->get_base_type()->get_field_offset(struct_component_name);
  std::shared_ptr<Type> component_type_pt(new PointerType(
      std::shared_ptr<Type>(new BasicType(BasicTypeKind::INT, true))));
  HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, component_type_pt);
  Operand dest1(Operand::VREG, next_temp_vreg());
  m_hl_iseq->append(new Instruction(mov_opcode, dest1, Operand(Operand::IMM_IVAL, component_offset)));
  Operand dest2(Operand::VREG, next_temp_vreg());
  HighLevelOpcode add_opcode = get_opcode(HINS_add_b, component_type_pt);
  m_hl_iseq->append(new Instruction(add_opcode, dest2, struct_var_ptr_opt,
                                    dest1));
  n->set_operand(dest2.to_memref());
}

void HighLevelCodegen::visit_array_element_ref_expression(Node *n)
{
  visit_children(n);
  Node *arr = n->get_kid(0);
  int size = arr->get_type()->get_base_type()->get_storage_size();
  Node *idx = n->get_kid(1);

  // now since we are understanding index as addition to pointer, we will
  // reset idx's type
  std::shared_ptr<Type> idx_type(new PointerType(
      (std::shared_ptr<Type>(new BasicType(BasicTypeKind::INT, true)))));

  // if the current idx is lower than this, then upgrade idx

  Operand idx_operand = idx->get_operand();
  Operand idx_moderated = check_types(idx_operand, idx->get_type(), idx_type);
  Operand dest1(Operand::VREG, next_temp_vreg());
  HighLevelOpcode mul_opcode = get_opcode(HINS_mul_b, idx_type);
  m_hl_iseq->append(new Instruction(mul_opcode, dest1, idx_moderated,
                                    Operand(Operand::IMM_IVAL, size)));
  Operand dest2(Operand::VREG, next_temp_vreg());
  HighLevelOpcode add_opcode = get_opcode(HINS_add_b, idx_type);
  m_hl_iseq->append(new Instruction(add_opcode, dest2, Operand(Operand::VREG, arr->get_operand().get_base_reg()),
                                    dest1));
  // Operand dest3(Operand::VREG, next_temp_vreg());
  // HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, n->get_type());
  // m_hl_iseq->append(new Instruction(mov_opcode, dest3, dest2.to_memref()));
  n->set_operand(dest2.to_memref());
  // TODO: implement
}

void HighLevelCodegen::visit_variable_ref(Node *n)
{
  visit_children(n);
  assert(n->has_symbol());
  Symbol *ref_symbol = n->get_symbol();
  if (ref_symbol->is_vreg())
  {
    n->set_operand(Operand(Operand::VREG, ref_symbol->get_vreg()));
  }
  else if (ref_symbol->is_offset())
  {
    Operand mem_ref = Operand(Operand::VREG, next_temp_vreg());
    m_hl_iseq->append(new Instruction(HINS_localaddr, mem_ref, Operand(Operand::IMM_IVAL, ref_symbol->get_offset())));
    n->set_operand(mem_ref.to_memref());
    // do later
  }
  // TODO: implement
}

void HighLevelCodegen::visit_literal_value(Node *n)
{
  // A partial implementation (note that this won't work correctly
  // for string constants!):
  visit_children(n);

  LiteralValue val = n->get_literal_value();
  if (val.get_kind() == LiteralValueKind::INTEGER)
  {
    Operand dest(Operand::VREG, next_temp_vreg());
    HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, n->get_type());
    m_hl_iseq->append(new Instruction(mov_opcode, dest, Operand(Operand::IMM_IVAL, val.get_int_value())));
    n->set_operand(dest);
  }
  else if (val.get_kind() == LiteralValueKind::CHARACTER)
  {
    Operand dest(Operand::VREG, next_temp_vreg());
    HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, n->get_type());
    m_hl_iseq->append(new Instruction(mov_opcode, dest, Operand(Operand::IMM_IVAL, val.get_char_value())));
    n->set_operand(dest);
  }
  else if (val.get_kind() == LiteralValueKind::STRING)
  {
    Operand dest(Operand::VREG, next_temp_vreg());
    HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, n->get_type());
    m_hl_iseq->append(new Instruction(mov_opcode, dest, n->get_operand()));
    n->set_operand(dest);
  }
}

void HighLevelCodegen::visit_implicit_conversion(Node *n)
{
  visit_children(n);
  // TODO: implement
}

void HighLevelCodegen::visit_assign_operation(Node *n)
{
  Node *LHS = n->get_kid(1);
  Node *RHS = n->get_kid(2);
  Operand LHS_op = LHS->get_operand();
  Operand RHS_op = RHS->get_operand();
  HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, n->get_type());
  if (LHS->get_type()->is_pointer() && RHS->get_type()->is_array())
  {
    m_hl_iseq->append(new Instruction(mov_opcode, LHS_op, Operand(Operand::VREG, RHS_op.get_base_reg())));
  }
  else
  {
    m_hl_iseq->append(new Instruction(mov_opcode, LHS_op, RHS_op));
  }
  n->set_operand(LHS_op);
}

void HighLevelCodegen::visit_relational_operation(Node *n)
{
  Node *LHS = n->get_kid(1);
  Node *RHS = n->get_kid(2);
  Operand LHS_op = LHS->get_operand();
  Operand RHS_op = RHS->get_operand();
  Node *op = n->get_kid(0);
  Operand dest(Operand::VREG, next_temp_vreg());
  HighLevelOpcode comp_opcode;
  switch (op->get_tag())
  {

  case TOK_LT:
    comp_opcode = get_opcode(HINS_cmplt_b, n->get_type());
    break;
  case TOK_GT:
    comp_opcode = get_opcode(HINS_cmpgt_b, n->get_type());
    break;
  case TOK_LTE:
    comp_opcode = get_opcode(HINS_cmplte_b, n->get_type());
    break;
  case TOK_GTE:
    comp_opcode = get_opcode(HINS_cmpgte_b, n->get_type());
    break;
  case TOK_EQUALITY:
    comp_opcode = get_opcode(HINS_cmpeq_b, n->get_type());
    break;
  case TOK_INEQUALITY:
    comp_opcode = get_opcode(HINS_cmpneq_b, n->get_type());
    break;
  default:
    break;
  }
  m_hl_iseq->append(new Instruction(comp_opcode, dest, LHS_op, RHS_op));
  n->set_operand(dest);
}

void HighLevelCodegen::visit_plus_minus_operation(Node *n)
{
  Node *LHS = n->get_kid(1);
  Node *RHS = n->get_kid(2);
  Operand LHS_op = LHS->get_operand();
  Operand RHS_op = RHS->get_operand();
  int tag = n->get_kid(0)->get_tag();
  HighLevelOpcode opcode = (tag == TOK_PLUS) ? get_opcode(
                                                   HINS_add_b, n->get_type())
                                             : get_opcode(HINS_sub_b, n->get_type());
  Operand dest(Operand::VREG, next_temp_vreg());
  m_hl_iseq->append(new Instruction(opcode, dest, LHS_op, RHS_op));
  n->set_operand(dest);
}

void HighLevelCodegen::visit_other_arithmetic_operation(Node *n)
{
  int tag = n->get_kid(0)->get_tag();
  Node *LHS = n->get_kid(1);
  Node *RHS = n->get_kid(2);
  Operand LHS_op = LHS->get_operand();
  Operand RHS_op = RHS->get_operand();
  HighLevelOpcode opcode;
  switch (tag)
  {
  case TOK_DIVIDE:
    opcode = get_opcode(HINS_div_b, n->get_type());
    break;
  case TOK_ASTERISK:
    opcode = get_opcode(HINS_mul_b, n->get_type());
    break;
  case TOK_MOD:
    opcode = get_opcode(HINS_mod_b, n->get_type());
  default:
    break;
  }
  Operand dest(Operand::VREG, next_temp_vreg());
  m_hl_iseq->append(new Instruction(opcode, dest, LHS_op, RHS_op));
  n->set_operand(dest);
}

void HighLevelCodegen::visit_logical_operation(Node *n)
{
}

std::string HighLevelCodegen::next_label()
{
  std::string label = ".L" + std::to_string(m_next_label_num++);
  return label;
}

int HighLevelCodegen::next_temp_vreg()
{
  int temp_vreg = next_vreg_num;
  next_vreg_num++;
  return temp_vreg;
}

// TODO: additional private member functions
