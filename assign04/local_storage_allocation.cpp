#include <cassert>
#include "node.h"
#include "ast.h"
#include <iostream>
#include "grammar_symbols.h"
#include "parse.tab.h"
#include "instruction.h"
#include "symtab.h"
#include "local_storage_allocation.h"

LocalStorageAllocation::LocalStorageAllocation()
    : m_total_local_storage(0U), m_next_vreg(VREG_FIRST_LOCAL)
{
}

LocalStorageAllocation::~LocalStorageAllocation()
{
}

void LocalStorageAllocation::visit_declarator_list(Node *n)
{
  // int nkid = n->get_num_kids();
  for (auto i = n->cbegin(); i != n->cend(); i++)
  {
    std::shared_ptr<Type> declarator_type = (*i)->get_type();
    assert((*i)->has_symbol());
    Symbol *declarator_symbol = (*i)->get_symbol();
    if (declarator_symbol->is_vreg())
    {
      // if it is an integral value or a pointer value
      // find its symbol and label the virtual register address
      declarator_symbol->set_vreg(m_next_vreg);
      // std::cout << "defined symbol " << declarator_symbol->get_name() << " in virtual register " << m_next_vreg << std::endl;
      (*i)->set_operand(Operand(Operand::VREG, m_next_vreg));
      std::cout << "/* variable '" << declarator_symbol->get_name() << "' allocated vreg " << m_next_vreg << " */" << std::endl;
      m_next_vreg++;
    }
    else
    {
      unsigned offset = m_storage_calc.add_field(declarator_type);
      // std::cout << "defined symbol " << declarator_symbol->get_name() << " in stack offset " << offset << std::endl;
      declarator_symbol->set_offset(offset);
      std::cout << "/* variable '" << declarator_symbol->get_name() << "' allocated " << declarator_type->get_storage_size() << " bytes of storage at offset " << offset << " */" << std::endl;
      m_total_local_storage += offset;
    }
  }
  // TODO: implement
}

void LocalStorageAllocation::visit_function_definition(Node *n)
{
  // int nkid = n->get_num_kids();
  // initiate a storage calculator so the offset can become 0
  StorageCalculator temp_store_for_parent_storage = m_storage_calc;
  int temp_store_for_curr_vreg = m_next_vreg;
  m_next_vreg = VREG_FIRST_LOCAL;
  m_storage_calc = StorageCalculator();
  visit_children(n);
  m_storage_calc.finish();
  unsigned stack_size = m_storage_calc.get_size();
  assert(n->has_symbol());
  Symbol *func_symbol = n->get_symbol();
  func_symbol->set_stack_size(stack_size);
  func_symbol->set_next_vreg(m_next_vreg);
  std::cout << "/* Function '" << func_symbol->get_name() << "' uses ";
  std::cout << stack_size << " bytes of memory and " << m_next_vreg << " virtual registers */" << std::endl;
  m_storage_calc = temp_store_for_parent_storage;
  m_next_vreg = temp_store_for_curr_vreg;
  // TODO: implement
}

void LocalStorageAllocation::visit_function_parameter_list(Node *n)
{
  int start_vreg_ars = VREG_FIRST_ARG;
  for (auto i = n->cbegin(); i != n->cend(); i++)
  {
    assert((*i)->has_symbol());
    std::shared_ptr<Type> param_type((*i)->get_type());
    Symbol *param_symbol = (*i)->get_symbol();
    if (param_type->is_integral() || param_type->is_pointer() || param_type->is_array())
    {
      // if it is an integral value, find its symbol and label the virtual register address
      // arrays in function parameters get passed in as pointers
      param_symbol->set_function_vreg(start_vreg_ars);
      param_symbol->set_vreg(m_next_vreg);
      std::cout << "/* variable '" << param_symbol->get_name() << "' allocated vreg " << m_next_vreg << " */" << std::endl;
      (*i)->set_function_operand(Operand(Operand::VREG, start_vreg_ars));
      (*i)->set_operand(Operand(Operand::VREG, m_next_vreg));
      // std::cout << "defined function symbol " << param_symbol->get_name() << " in virtual register " << start_vreg_ars << std::endl;
      start_vreg_ars++;
      m_next_vreg++;
    }
    else
    {
      // other types (most likely structs) get allocation space
      unsigned offset = m_storage_calc.add_field(param_type);
      // std::cout << "defined function symbol " << param_symbol->get_name() << " in stack offset " << offset << std::endl;
      param_symbol->set_offset(offset);
      std::cout << "/* variable '" << param_symbol->get_name() << "' allocated " << param_type->get_storage_size() << " bytes of storage at offset " << offset << " */" << std::endl;
      m_total_local_storage += offset;
    }
  }
  // TODO: implement
}

void LocalStorageAllocation::visit_function_declaration(Node *n)
{
  // don't allocate storage for parameters in a function declaration
}

// void LocalStorageAllocation::visit_statement_list(Node *n)
// {
//   for (auto i = n->cbegin(); i != n->cend(); i++)
//   {
//   }
//   // TODO: implement
// }

void LocalStorageAllocation::visit_struct_type_definition(Node *n)
{
  // find the symbol
  // std::shared_ptr<Type> struct_type = n->get_type();
  // assert(n->has_symbol());
  // Symbol *struct_symbol = n->get_symbol();
  // int num_members = struct_type->get_num_members();
  // int offset = 0;
  // for (int i = 0; i < num_members; i++)
  // {
  //   const Member &mem = struct_type->get_member(i);
  //   std::shared_ptr<Type> mem_type(mem.get_type());
  //   std::string mem_name = mem.get_name();
  //   struct_symbol->add_member_offset(mem_name, mem_type, offset);
  //   offset += mem_type->get_storage_size();
  // }

  // TODO: implement (if you are going to use this visitor to assign offsets for struct fields)
  // basically nothing happens to prevent additional vreg / offset definition
}

// void LocalStorageAllocation::visit_unary_expression(Node *n)
// {
//   int unary_tag = n->get_kid(0)->get_tag();
//   Node *var = n->get_kid(1);
//   std::shared_ptr<Type> var_type(var->get_type());
//   // consider the case * and &
//   switch (unary_tag)
//   {
//   case TOK_AMPERSAND: /* ‘&’， taking the address */
//   {
//     assert(var->has_symbol());
//     Symbol *var_symbol = var->get_symbol();
//     std::string var_name = var_symbol->get_name();
//     if (var_symbol->is_vreg())
//     {
//       // if the variable being referenced is currently in a
//       // virtual register, we need to change it to a stack offset
//       unsigned offset = m_storage_calc.add_field(var_type);
//       std::cout << "defined symbol " << var_symbol->get_name() << " switched to stack offset " << offset << std::endl;
//       var_symbol->set_offset(offset);
//       m_total_local_storage += offset;
//     }
//   }
//   break;
//   default:
//     visit(var);
//     break;
//   }
// }

// TODO: implement private member functions
