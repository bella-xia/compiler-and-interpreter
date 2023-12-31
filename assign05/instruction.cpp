#include <cassert>
#include "instruction.h"

Instruction::Instruction(int opcode)
    : Instruction(opcode, Operand(), Operand(), Operand(), 0)
{
}

Instruction::Instruction(int opcode, const Operand &op1)
    : Instruction(opcode, op1, Operand(), Operand(), 1)
{
}

Instruction::Instruction(int opcode, const Operand &op1, const Operand &op2)
    : Instruction(opcode, op1, op2, Operand(), 2)
{
}

Instruction::Instruction(int opcode, const Operand &op1, const Operand &op2, const Operand &op3, unsigned num_operands)
    : Instruction(opcode, op1, op2, op3, std::vector<std::tuple<Operand, int>>(), num_operands)
{
}

Instruction::Instruction(int opcode, const Operand &op1, const Operand &op2, const Operand &op3,
                         const std::vector<std::tuple<Operand, int>> &m_ops, unsigned num_operands)
    : m_opcode(opcode),
      m_num_operands(num_operands),
      m_operands{op1, op2, op3},
      m_moperands{Operand(), Operand(), Operand()},
      m_symbol(nullptr)
{
  for (auto m_op : m_ops)
  {
    int idx = std::get<1>(m_op);
    m_moperands[idx] = Operand(std::get<0>(m_op));
  }
}

Instruction::~Instruction()
{
}

int Instruction::get_opcode() const
{
  return m_opcode;
}

unsigned Instruction::get_num_operands() const
{
  return m_num_operands;
}

const Operand &Instruction::get_operand(unsigned index) const
{
  assert(index < m_num_operands);
  return m_operands[index];
}

void Instruction::set_operand(unsigned index, const Operand &operand)
{
  assert(index < m_num_operands);
  m_operands[index] = operand;
}

Operand Instruction::get_last_operand() const
{
  assert(m_num_operands > 0);
  return m_operands[m_num_operands - 1];
}
