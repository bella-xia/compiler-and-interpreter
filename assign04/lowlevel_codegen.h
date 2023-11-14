#ifndef LOWLEVEL_CODEGEN_H
#define LOWLEVEL_CODEGEN_H

#include <memory>
#include "operand.h"
#include "instruction.h"
#include "instruction_seq.h"
#include "highlevel_formatter.h"

// A LowLevelCodeGen object transforms an InstructionSequence containing
// high-level instructions into an InstructionSequence containing
// low-level (x86-64) instructions.
class LowLevelCodeGen
{
private:
  int m_total_memory_storage, m_vreg_storage_start, m_stack_storage_start, m_residual;
  bool m_optimize;
  HighLevelFormatter hl_format;

public:
  LowLevelCodeGen(bool optimize);
  virtual ~LowLevelCodeGen();

  std::shared_ptr<InstructionSequence> generate(const std::shared_ptr<InstructionSequence> &hl_iseq);

private:
  std::shared_ptr<InstructionSequence> translate_hl_to_ll(const std::shared_ptr<InstructionSequence> &hl_iseq);
  void translate_instruction(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq);
  Operand get_ll_operand(Operand hl_opcode, int size, const std::shared_ptr<InstructionSequence> &ll_iseq);
  Operand convert_low_level_operand(Operand hl_operand, int operand_size, bool &next_temp_reg, bool &first_instruction,
                                    bool is_src, Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq);
  Operand create_next_temp_reg(int next_reg, int operand_size);
  void append_first_instruction(Instruction *hl_ins, Instruction *ll_ins, const std::shared_ptr<InstructionSequence> &ll_iseq);
  void check_first_instruction(Instruction *hl_ins, Instruction *ll_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, bool &first_instruction);

  void mov_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode);
  void operation_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode);
  void division_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode);
  void comp_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode);
  void sconv_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode);
  void localaddr_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode);
  void neg_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode);
};

#endif // LOWLEVEL_CODEGEN_H
