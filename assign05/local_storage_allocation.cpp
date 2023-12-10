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
      (*i)->set_operand(Operand(Operand::VREG, m_next_vreg));
      std::cout << "/* variable '" << declarator_symbol->get_name() << "' allocated vreg " << m_next_vreg << " */" << std::endl;
      m_next_vreg++;
      m_var_map.insert({declarator_symbol, 0});
    }
    else
    {
      unsigned offset = m_storage_calc.add_field(declarator_type);
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
  m_var_map = std::map<Symbol *, int>();
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

  std::vector<std::pair<Symbol *, int>> mapVector(m_var_map.begin(), m_var_map.end());

  // Sorting the vector based on the values in descending order
  std::sort(mapVector.begin(), mapVector.end(), [](const auto &a, const auto &b)
            {
              return a.second > b.second; // Change to < for ascending order
            });
  int m_next_mreg = 0;
  int m_next_stack = 0;
  for (auto var : mapVector)
  {
    if (m_next_mreg < (int)m_regs.size())
    {
      var.first->set_mreg(m_regs[m_next_mreg]);
      func_symbol->set_used_callee_mreg(m_regs[m_next_mreg]);
      std::cout << "/* allocate machine register " << m_regs[m_next_mreg] << " to variable '";
      std::cout << var.first->get_name() << "' (vr" << var.first->get_vreg() << ") */" << std::endl;
      m_next_mreg++;
    }
    else
    {
      func_symbol->set_optimized_stack(var.first->get_vreg());
      var.first->set_mreg("stack_" + std::to_string(m_next_stack));
      m_next_stack++;
    }
  }
  std::vector<int> in_stack = func_symbol->get_optimized_stack();
  std::string vreg_in_stack_list = "";
  if ((int)in_stack.size() != 0)
  {
    for (int i = 0; i < (int)in_stack.size(); ++i)
    {
      vreg_in_stack_list += "vr" + std::to_string(in_stack.at(i)) + ", ";
    }
    std::cout << "/* allocated in stack: " << vreg_in_stack_list << "*/" << std::endl;
  }
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
      Operand param_operand = Operand(Operand::VREG, start_vreg_ars);
      (*i)->set_function_operand(Operand(Operand::VREG, start_vreg_ars));
      (*i)->set_operand(Operand(Operand::VREG, m_next_vreg));
      m_var_map.insert({param_symbol, 0});
      start_vreg_ars++;
      m_next_vreg++;
    }
    else
    {
      // other types (most likely structs) get allocation space
      unsigned offset = m_storage_calc.add_field(param_type);
      param_symbol->set_offset(offset);
      std::cout << "/* variable '" << param_symbol->get_name() << "' allocated " << param_type->get_storage_size() << " bytes of storage at offset " << offset << " */" << std::endl;
      m_total_local_storage += offset;
    }
  }
  // TODO: implement
}

void LocalStorageAllocation::visit_variable_ref(Node *n)
{
  if (n->has_symbol())
  {
    Symbol *symb = n->get_symbol();
    if (m_var_map.find(symb) != m_var_map.end())
    {
      m_var_map[symb]++;
    }
  }
  visit_children(n);
}

void LocalStorageAllocation::visit_function_declaration(Node *n)
{
  // don't allocate storage for parameters in a function declaration
}

void LocalStorageAllocation::visit_struct_type_definition(Node *n)
{
  // TODO: implement (if you are going to use this visitor to assign offsets for struct fields)
  // basically nothing happens to prevent additional vreg / offset definition
}
// TODO: implement private member functions
