#include <cassert>
#include <iostream>
#include <map>
#include "cpputil.h"
#include "node.h"
#include "instruction.h"
#include "operand.h"
#include "local_storage_allocation.h"
#include "highlevel.h"
#include "lowlevel.h"
#include "exceptions.h"
#include "lowlevel_codegen.h"

// This map has some "obvious" translations of high-level opcodes to
// low-level opcodes.
const std::map<HighLevelOpcode, LowLevelOpcode> HL_TO_LL = {
    {HINS_nop, MINS_NOP},
    {HINS_add_b, MINS_ADDB},
    {HINS_add_w, MINS_ADDW},
    {HINS_add_l, MINS_ADDL},
    {HINS_add_q, MINS_ADDQ},
    {HINS_sub_b, MINS_SUBB},
    {HINS_sub_w, MINS_SUBW},
    {HINS_sub_l, MINS_SUBL},
    {HINS_sub_q, MINS_SUBQ},
    {HINS_mul_l, MINS_IMULL},
    {HINS_mul_q, MINS_IMULQ},
    {HINS_mov_b, MINS_MOVB},
    {HINS_mov_w, MINS_MOVW},
    {HINS_mov_l, MINS_MOVL},
    {HINS_mov_q, MINS_MOVQ},
    {HINS_sconv_bw, MINS_MOVSBW},
    {HINS_sconv_bl, MINS_MOVSBL},
    {HINS_sconv_bq, MINS_MOVSBQ},
    {HINS_sconv_wl, MINS_MOVSWL},
    {HINS_sconv_wq, MINS_MOVSWQ},
    {HINS_sconv_lq, MINS_MOVSLQ},
    {HINS_uconv_bw, MINS_MOVZBW},
    {HINS_uconv_bl, MINS_MOVZBL},
    {HINS_uconv_bq, MINS_MOVZBQ},
    {HINS_uconv_wl, MINS_MOVZWL},
    {HINS_uconv_wq, MINS_MOVZWQ},
    {HINS_uconv_lq, MINS_MOVZLQ},
    {HINS_ret, MINS_RET},
    {HINS_jmp, MINS_JMP},
    {HINS_call, MINS_CALL},

    // For comparisons, it is expected that the code generator will first
    // generate a cmpb/cmpw/cmpl/cmpq instruction to compare the operands,
    // and then generate a setXX instruction to put the result of the
    // comparison into the destination operand. These entries indicate
    // the apprpropriate setXX instruction to use.
    {HINS_cmplt_b, MINS_SETL},
    {HINS_cmplt_w, MINS_SETL},
    {HINS_cmplt_l, MINS_SETL},
    {HINS_cmplt_q, MINS_SETL},
    {HINS_cmplte_b, MINS_SETLE},
    {HINS_cmplte_w, MINS_SETLE},
    {HINS_cmplte_l, MINS_SETLE},
    {HINS_cmplte_q, MINS_SETLE},
    {HINS_cmpgt_b, MINS_SETG},
    {HINS_cmpgt_w, MINS_SETG},
    {HINS_cmpgt_l, MINS_SETG},
    {HINS_cmpgt_q, MINS_SETG},
    {HINS_cmpgte_b, MINS_SETGE},
    {HINS_cmpgte_w, MINS_SETGE},
    {HINS_cmpgte_l, MINS_SETGE},
    {HINS_cmpgte_q, MINS_SETGE},
    {HINS_cmpeq_b, MINS_SETE},
    {HINS_cmpeq_w, MINS_SETE},
    {HINS_cmpeq_l, MINS_SETE},
    {HINS_cmpeq_q, MINS_SETE},
    {HINS_cmpneq_b, MINS_SETNE},
    {HINS_cmpneq_w, MINS_SETNE},
    {HINS_cmpneq_l, MINS_SETNE},
    {HINS_cmpneq_q, MINS_SETNE},
};

const std::map<int, enum MachineReg> TO_REG = {
    {0, MREG_RAX},
    {1, MREG_RDI},
    {2, MREG_RSI},
    {3, MREG_RDX},
    {4, MREG_RCX},
    {5, MREG_R8},
    {6, MREG_R9},
};

const std::map<std::string, enum MachineReg> STR_TO_REG = {
    {"%rbx", MREG_RBX},
    {"%r12", MREG_R12},
    {"%r13", MREG_R13},
    {"%r14", MREG_R14},
    {"%r15", MREG_R15},
    {"%r8", MREG_R8},
    {"%r9", MREG_R9},
    {"%rcx", MREG_RCX},
    {"%rdx", MREG_RDX},
    {"%rsi", MREG_RSI},
    {"%rdi", MREG_RDI}};

LowLevelCodeGen::LowLevelCodeGen(bool optimize) : m_total_memory_storage(0),
                                                  m_vreg_storage_start(0), m_stack_storage_start(0),
                                                  m_residual(0), m_optimize(optimize), hl_format(HighLevelFormatter()),
                                                  m_callee_mregs(std::vector<std::string>())
{
}

LowLevelCodeGen::~LowLevelCodeGen()
{
}

std::shared_ptr<InstructionSequence> LowLevelCodeGen::generate(const std::shared_ptr<InstructionSequence> &hl_iseq)
{
  // TODO: if optimizations are enabled, could do analysis/transformation of high-level code
  // Node *funcdef_ast = hl_iseq->get_funcdef_ast();
  std::shared_ptr<InstructionSequence> cur_hl_iseq(hl_iseq);
  std::shared_ptr<InstructionSequence> ll_iseq = translate_hl_to_ll(cur_hl_iseq);

  // TODO: if optimizations are enabled, could do analysis/transformation of low-level code

  return ll_iseq;
}

std::shared_ptr<InstructionSequence> LowLevelCodeGen::translate_hl_to_ll(const std::shared_ptr<InstructionSequence> &hl_iseq)
{
  std::shared_ptr<InstructionSequence> ll_iseq(new InstructionSequence());

  // The high-level InstructionSequence will have a pointer to the Node
  // representing the function definition. Useful information could be stored
  // there (for example, about the amount of memory needed for local storage,
  // maximum number of virtual registers used, etc.)
  Node *funcdef_ast = hl_iseq->get_funcdef_ast();
  Symbol *func_symbol = funcdef_ast->get_symbol();
  assert(funcdef_ast != nullptr);

  // It's not a bad idea to store the pointer to the function definition AST
  // in the low-level InstructionSequence as well, in case it's needed by
  // optimization passes.
  ll_iseq->set_funcdef_ast(funcdef_ast);

  // Determine the total number of bytes of memory storage
  // that the function needs. This should include both variables that
  // *must* have storage allocated in memory (e.g., arrays), and also
  // any additional memory that is needed for virtual registers,
  // spilled machine registers, etc.
  std::string func_name = func_symbol->get_name();
  int stack_storage = func_symbol->get_stack_size();
  int offset = (stack_storage % 8 == 0) ? 0 : 8 - stack_storage % 8;
  m_stack_storage_start = -1 * (stack_storage + offset);
  if (stack_storage > 0)
  {
    std::string mem_storage_offset_str = cpputil::format("/* Function '%s' uses %d total bytes of memory storage for stack allocation */",
                                                         func_name.c_str(), -m_stack_storage_start);
    std::cout << mem_storage_offset_str << std::endl;
    std::string mem_start_offset_str = cpputil::format("/* Function '%s': placing memory storage at offset %d from %%rbp */",
                                                       func_name.c_str(), m_stack_storage_start);
    std::cout << mem_start_offset_str << std::endl;
  }
  int max_vreg_used = func_symbol->get_max_vreg();
  int vreg_alloc_size = (max_vreg_used - 9) * 8;
  if (m_optimize)
  {
    vreg_alloc_size = func_symbol->get_total_optimized_stack_size() * 8;
  }
  std::string vreg_storage_str = cpputil::format("/* Function '%s' uses %d total bytes of memory storage for vregs */",
                                                 func_name.c_str(), vreg_alloc_size);
  std::cout << vreg_storage_str << std::endl;

  m_total_memory_storage = vreg_alloc_size + -m_stack_storage_start; // FIXME: determine how much memory storage on the stack is needed
  m_vreg_storage_start = -m_total_memory_storage;
  std::string vreg_start_offset_str = cpputil::format("/* Function '%s': placing vreg storage at offset %d from %%rbp */",
                                                      func_name.c_str(), m_vreg_storage_start);
  std::cout << vreg_start_offset_str << std::endl;
  // The function prologue will push %rbp, which should guarantee that the
  // stack pointer (%rsp) will contain an address that is a multiple of 16.
  // If the total memory storage required is not a multiple of 16, add to
  // it so that it is.
  if (m_optimize)
  {
    m_callee_mregs = funcdef_ast->get_symbol()->get_callee_mreg_used();
    int total_stack_size = m_total_memory_storage + (int)m_callee_mregs.size() * 8;
    if (total_stack_size % 16 != 0)
    {
      m_residual = 16 - (total_stack_size % 16);
      m_total_memory_storage += m_residual;
    }
  }
  else
  {
    if (m_total_memory_storage % 16 != 0)
    {
      m_residual = 16 - (m_total_memory_storage % 16);
      m_total_memory_storage += m_residual;
    }
  }

  std::cout << "/* Function '" << func_name << "': " << m_total_memory_storage << " bytes allocated in stack frame */" << std::endl;

  // Iterate through high level instructions
  for (auto i = hl_iseq->cbegin(); i != hl_iseq->cend(); ++i)
  {
    Instruction *hl_ins = *i;

    // If the high-level instruction has a label, define an equivalent
    // label in the low-level instruction sequence
    if (i.has_label())
      ll_iseq->define_label(i.get_label());

    // Translate the high-level instruction into one or more low-level instructions
    translate_instruction(hl_ins, ll_iseq);
  }

  return ll_iseq;
}

// These helper functions are provided to make it easier to handle
// the way that instructions and operands vary based on operand size
// ('b'=1 byte, 'w'=2 bytes, 'l'=4 bytes, 'q'=8 bytes.)

// Check whether hl_opcode matches a range of opcodes, where base
// is a _b variant opcode. Return true if the hl opcode is any variant
// of that base.
bool match_hl(int base, int hl_opcode)
{
  return hl_opcode >= base && hl_opcode < (base + 4);
}

// For a low-level instruction with 4 size variants, return the correct
// variant. base_opcode should be the "b" variant, and operand_size
// should be the operand size in bytes (1, 2, 4, or 8.)
LowLevelOpcode select_ll_opcode(LowLevelOpcode base_opcode, int operand_size)
{
  int off;

  switch (operand_size)
  {
  case 1: // 'b' variant
    off = 0;
    break;
  case 2: // 'w' variant
    off = 1;
    break;
  case 4: // 'l' variant
    off = 2;
    break;
  case 8: // 'q' variant
    off = 3;
    break;
  default:
    assert(false);
    off = 3;
  }

  return LowLevelOpcode(int(base_opcode) + off);
}

// Get the correct Operand::Kind value for a machine register
// of the specified size (1, 2, 4, or 8 bytes.)
Operand::Kind select_mreg_kind(int operand_size)
{
  switch (operand_size)
  {
  case 1:
    return Operand::MREG8;
  case 2:
    return Operand::MREG16;
  case 4:
    return Operand::MREG32;
  case 8:
    return Operand::MREG64;
  default:
    assert(false);
    return Operand::MREG64;
  }
}

LowLevelOpcode double_size_substript_convert(LowLevelOpcode base_opcode, int src_size, int dest_size)
{
  int offset = 0;
  switch (src_size)
  {
  case 1:
    if (dest_size == 4)
    {
      offset = 1;
    }
    else if (dest_size == 8)
    {
      offset = 2;
    }
    break;
  case 2:
    offset = (dest_size == 4) ? 3 : 4;
    break;
  case 4:
    offset = 5;
    break;
  default:
    break;
  }
  return LowLevelOpcode((int)base_opcode + offset);
}

void LowLevelCodeGen::translate_instruction(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq)
{
  HighLevelOpcode hl_opcode = HighLevelOpcode(hl_ins->get_opcode());

  if (hl_opcode == HINS_enter)
  {
    if (m_optimize)
    {

      if ((int)m_callee_mregs.size() > 0)
      {
        MachineReg mreg = STR_TO_REG.find(m_callee_mregs.at(0))->second;
        append_first_instruction(hl_ins, new Instruction(MINS_PUSHQ, Operand(Operand::MREG64, mreg)), ll_iseq);
        for (int i = 1; i < (int)m_callee_mregs.size(); ++i)
        {
          mreg = STR_TO_REG.find(m_callee_mregs.at(i))->second;
          ll_iseq->append(new Instruction(MINS_PUSHQ, Operand(Operand::MREG64, mreg)));
        }
        ll_iseq->append(new Instruction(MINS_PUSHQ, Operand(Operand::MREG64, MREG_RBP)));
      }
      else
      {
        append_first_instruction(hl_ins, new Instruction(MINS_PUSHQ, Operand(Operand::MREG64, MREG_RBP)), ll_iseq);
      }
      ll_iseq->append(new Instruction(MINS_MOVQ, Operand(Operand::MREG64, MREG_RSP), Operand(Operand::MREG64, MREG_RBP)));
      if (m_total_memory_storage > 0)
        ll_iseq->append(new Instruction(MINS_SUBQ, Operand(Operand::IMM_IVAL, m_total_memory_storage), Operand(Operand::MREG64, MREG_RSP)));
    }
    else
    {
      // Function prologue: this will create an ABI-compliant stack frame.
      // The local variable area is *below* the address in %rbp, and local storage
      // can be accessed at negative offsets from %rbp. For example, the topmost
      // 4 bytes in the local storage area are at -4(%rbp).
      append_first_instruction(hl_ins, new Instruction(MINS_PUSHQ, Operand(Operand::MREG64, MREG_RBP)), ll_iseq);
      ll_iseq->append(new Instruction(MINS_MOVQ, Operand(Operand::MREG64, MREG_RSP), Operand(Operand::MREG64, MREG_RBP)));
      if (m_total_memory_storage > 0)
        ll_iseq->append(new Instruction(MINS_SUBQ, Operand(Operand::IMM_IVAL, m_total_memory_storage), Operand(Operand::MREG64, MREG_RSP)));

      // save callee-saved registers (if any)
      // TODO: if you allocated callee-saved registers as storage for local variables,
      //       emit pushq instructions to save their original values
    }
    return;
  }

  if (hl_opcode == HINS_leave)
  {
    // Function epilogue: deallocate local storage area and restore original value
    // of %rbp

    // TODO: if you allocated callee-saved registers as storage for local variables,
    //       emit popq instructions to save their original values

    if (m_total_memory_storage > 0)
    {
      append_first_instruction(hl_ins, new Instruction(MINS_ADDQ, Operand(Operand::IMM_IVAL, m_total_memory_storage), Operand(Operand::MREG64, MREG_RSP)), ll_iseq);
      ll_iseq->append(new Instruction(MINS_POPQ, Operand(Operand::MREG64, MREG_RBP)));
    }
    else
    {
      append_first_instruction(hl_ins, new Instruction(MINS_POPQ, Operand(Operand::MREG64, MREG_RBP)), ll_iseq);
    }
    if (m_optimize)
    {
      for (int i = (int)m_callee_mregs.size() - 1; i >= 0; --i)
      {
        MachineReg mreg = STR_TO_REG.find(m_callee_mregs.at(i))->second;
        ll_iseq->append(new Instruction(MINS_POPQ, Operand(Operand::MREG64, mreg)));
      }
    }
    return;
  }
  if (hl_opcode == HINS_ret)
  {
    append_first_instruction(hl_ins, new Instruction(MINS_RET), ll_iseq);
    return;
  }
  if (hl_opcode == HINS_jmp)
  {
    int num_operand = hl_ins->get_num_operands();
    assert(num_operand == 1);
    append_first_instruction(hl_ins, new Instruction(MINS_JMP, hl_ins->get_operand(0)), ll_iseq);
    return;
  }
  if (hl_opcode == HINS_cjmp_t || hl_opcode == HINS_cjmp_f)
  {
    bool next_temp_reg = false;
    bool first_instruction = true;
    int num_operand = hl_ins->get_num_operands();
    assert(num_operand == 2);
    Operand comp_op = hl_ins->get_operand(0);
    Operand label_op = hl_ins->get_operand(1);
    int comp_size = highlevel_opcode_get_source_operand_size(hl_opcode);
    comp_size = (comp_size == 0) ? 4 : comp_size;
    Operand ll_comp_op = convert_low_level_operand(comp_op, comp_size, next_temp_reg, first_instruction, true, hl_ins, ll_iseq);
    LowLevelOpcode comp_opcode = select_ll_opcode(MINS_CMPB, comp_size);
    Operand literal = Operand(Operand::IMM_IVAL, 0);
    LowLevelOpcode jump_if_opcode = (hl_opcode == HINS_cjmp_t) ? MINS_JNE : MINS_JE;
    check_first_instruction(hl_ins, new Instruction(comp_opcode, literal, ll_comp_op), ll_iseq, first_instruction);
    ll_iseq->append(new Instruction(jump_if_opcode, label_op));
    return;
  }
  if (hl_opcode == HINS_call)
  {
    int num_operand = hl_ins->get_num_operands();
    assert(num_operand == 1);
    Operand func_label = hl_ins->get_operand(0);
    Instruction *funcall = new Instruction(MINS_CALL, func_label);
    Symbol *symb = hl_ins->get_symbol();
    funcall->set_symbol(symb);
    append_first_instruction(hl_ins, funcall, ll_iseq);
    return;
  }
  if (hl_opcode >= HINS_neg_b && hl_opcode <= HINS_neg_q)
  {
    neg_opcode(hl_ins, ll_iseq, hl_opcode);
    return;
  }
  if (hl_opcode == HINS_localaddr)
  {
    localaddr_opcode(hl_ins, ll_iseq, hl_opcode);
    return;
  }
  // a move instruction
  if (hl_opcode >= HINS_mov_b && hl_opcode <= HINS_mov_q)
  {
    mov_opcode(hl_ins, ll_iseq, hl_opcode);
    return;
  }
  if (hl_opcode >= HINS_add_b && hl_opcode <= HINS_mul_q)
  {
    operation_opcode(hl_ins, ll_iseq, hl_opcode);
    return;
  }
  if (hl_opcode >= HINS_div_b && hl_opcode <= HINS_mod_q)
  {
    division_opcode(hl_ins, ll_iseq, hl_opcode);
    return;
  }
  if (hl_opcode >= HINS_cmplt_b && hl_opcode <= HINS_cmpneq_q)
  {
    comp_opcode(hl_ins, ll_iseq, hl_opcode);
    return;
  }
  if (hl_opcode >= HINS_sconv_bw && hl_opcode <= HINS_sconv_lq)
  {
    sconv_opcode(hl_ins, ll_iseq, hl_opcode);
    return;
  }

  // TODO: handle other high-level instructions
  // Note that you can use the highlevel_opcode_get_source_operand_size() and
  // highlevel_opcode_get_dest_operand_size() functions to determine the
  // size (in bytes, 1, 2, 4, or 8) of either the source operands or
  // destination operand of a high-level instruction. This should be useful
  // for choosing the appropriate low-level instructions and
  // machine register operands.

  // RuntimeError::raise("high level opcode %d not handled", int(hl_opcode));
}

void LowLevelCodeGen::mov_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode)
{
  bool next_temp_reg = false;
  bool first_instruction = true;
  int num_operand = hl_ins->get_num_operands();
  assert(num_operand == 2);
  Operand dest_op = hl_ins->get_operand(0);
  Operand src_op = hl_ins->get_operand(1);
  int src_size = highlevel_opcode_get_source_operand_size(hl_opcode);
  int dest_size = highlevel_opcode_get_dest_operand_size(hl_opcode);

  Operand ll_dest_op = convert_low_level_operand(dest_op, dest_size, next_temp_reg, first_instruction, false, hl_ins, ll_iseq);
  Operand ll_src_op = convert_low_level_operand(src_op, src_size, next_temp_reg, first_instruction, true, hl_ins, ll_iseq);
  auto it = HL_TO_LL.find(hl_opcode);
  LowLevelOpcode ll_opcode;
  if (it != HL_TO_LL.end())
  {
    ll_opcode = it->second;
  }
  else
  {
    RuntimeError::raise("cannot find opcode");
  }
  if (ll_dest_op.is_in_stack() && ll_src_op.is_in_stack())
  {
    Operand temp = Operand(create_next_temp_reg(next_temp_reg, dest_size));
    next_temp_reg = !next_temp_reg;
    check_first_instruction(hl_ins, new Instruction(ll_opcode, ll_src_op, temp), ll_iseq, first_instruction);
    ll_iseq->append(new Instruction(ll_opcode, temp, ll_dest_op));
  }
  else
  {
    check_first_instruction(hl_ins, new Instruction(ll_opcode, ll_src_op, ll_dest_op), ll_iseq, first_instruction);
  }
}

void LowLevelCodeGen::operation_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode)
{
  bool next_temp_reg = false;
  bool first_instruction = true;
  int num_operand = hl_ins->get_num_operands();
  assert(num_operand == 3);
  Operand dest_op = hl_ins->get_operand(0);
  Operand src_op1 = hl_ins->get_operand(1);
  Operand src_op2 = hl_ins->get_operand(2);
  int src_size = highlevel_opcode_get_source_operand_size(hl_opcode);
  int dest_size = highlevel_opcode_get_dest_operand_size(hl_opcode);
  Operand temp = create_next_temp_reg(next_temp_reg, dest_size);
  next_temp_reg = !next_temp_reg;

  Operand ll_dest_op = convert_low_level_operand(dest_op, dest_size, next_temp_reg, first_instruction, false, hl_ins, ll_iseq);
  Operand ll_src_op1 = convert_low_level_operand(src_op1, src_size, next_temp_reg, first_instruction, true, hl_ins, ll_iseq);
  Operand ll_src_op2 = convert_low_level_operand(src_op2, src_size, next_temp_reg, first_instruction, true, hl_ins, ll_iseq);
  auto it = HL_TO_LL.find(hl_opcode);
  LowLevelOpcode mov_src_opcode = select_ll_opcode(MINS_MOVB, src_size);
  LowLevelOpcode mov_dest_opcode = select_ll_opcode(MINS_MOVB, dest_size);
  LowLevelOpcode operator_opcode;
  if (it != HL_TO_LL.end())
  {
    operator_opcode = it->second;
  }
  else
  {
    RuntimeError::raise("cannot find opcode");
  }
  // first move the first source into the temp
  check_first_instruction(hl_ins, new Instruction(mov_src_opcode, ll_src_op1, temp), ll_iseq, first_instruction);
  ll_iseq->append(new Instruction(operator_opcode, ll_src_op2, temp));
  ll_iseq->append(new Instruction(mov_dest_opcode, temp, ll_dest_op));
}

void LowLevelCodeGen::division_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode)
{
  bool next_temp_reg = false;
  bool first_instruction = true;
  int num_operand = hl_ins->get_num_operands();
  assert(num_operand == 3);
  Operand dest_op = hl_ins->get_operand(0);
  Operand src_op1 = hl_ins->get_operand(1);
  Operand src_op2 = hl_ins->get_operand(2);
  int src_size = highlevel_opcode_get_source_operand_size(hl_opcode);
  int dest_size = highlevel_opcode_get_dest_operand_size(hl_opcode);
  Operand ll_src_op1 = convert_low_level_operand(src_op1, src_size, next_temp_reg, first_instruction, true, hl_ins, ll_iseq);

  LowLevelOpcode src1_mov_opcode = select_ll_opcode(MINS_MOVB, src_size);
  LowLevelOpcode src2_div_opcode = (src_size == 4) ? MINS_IDIVL : MINS_IDIVQ;
  LowLevelOpcode dest_mov_opcode = select_ll_opcode(MINS_MOVB, dest_size);
  Operand ll_rax_op = Operand(select_mreg_kind(src_size), MREG_RAX);
  Operand ll_result_op = Operand(select_mreg_kind(dest_size), (hl_opcode <= HINS_div_q) ? MREG_RAX : MREG_RDX);

  check_first_instruction(hl_ins, new Instruction(src1_mov_opcode, ll_src_op1, ll_rax_op), ll_iseq, first_instruction);
  ll_iseq->append(new Instruction(MINS_CDQ));

  Operand ll_src_op2 = convert_low_level_operand(src_op2, src_size, next_temp_reg, first_instruction, true, hl_ins, ll_iseq);
  if (ll_src_op2.get_kind() == Operand::MREG64_MEM_OFF)
  {
    LowLevelOpcode src2_mov_opcode = select_ll_opcode(MINS_MOVB, src_size);
    Operand temp = create_next_temp_reg(next_temp_reg, dest_size);
    next_temp_reg = !next_temp_reg;
    ll_iseq->append(new Instruction(src2_mov_opcode, ll_src_op2, temp));
    ll_src_op2 = temp;
  }
  ll_iseq->append(new Instruction(src2_div_opcode, ll_src_op2));

  Operand ll_dest_op = convert_low_level_operand(dest_op, dest_size, next_temp_reg, first_instruction, false, hl_ins, ll_iseq);
  ll_iseq->append(new Instruction(dest_mov_opcode, ll_result_op, ll_dest_op));
}

void LowLevelCodeGen::comp_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode)
{
  bool next_temp_reg = false;
  bool first_instruction = true;
  int num_operand = hl_ins->get_num_operands();
  assert(num_operand == 3);
  Operand dest_op = hl_ins->get_operand(0);
  Operand src_op1 = hl_ins->get_operand(1);
  Operand src_op2 = hl_ins->get_operand(2);
  int src_size = highlevel_opcode_get_source_operand_size(hl_opcode);
  int dest_size = highlevel_opcode_get_dest_operand_size(hl_opcode);

  Operand temp1 = create_next_temp_reg(next_temp_reg, src_size);
  Operand temp2 = create_next_temp_reg(next_temp_reg, 1);
  next_temp_reg = !next_temp_reg;
  Operand temp3 = create_next_temp_reg(next_temp_reg, dest_size);
  next_temp_reg = !next_temp_reg;

  Operand ll_dest_op = convert_low_level_operand(dest_op, dest_size, next_temp_reg, first_instruction, false, hl_ins, ll_iseq);
  Operand ll_src_op1 = convert_low_level_operand(src_op1, src_size, next_temp_reg, first_instruction, true, hl_ins, ll_iseq);
  Operand ll_src_op2 = convert_low_level_operand(src_op2, src_size, next_temp_reg, first_instruction, true, hl_ins, ll_iseq);

  LowLevelOpcode mov_src_opcode = select_ll_opcode(MINS_MOVB, src_size);
  LowLevelOpcode size_convert_opcode = double_size_substript_convert(MINS_MOVZBW, 1, dest_size);
  LowLevelOpcode mov_dest_opcode = select_ll_opcode(MINS_MOVB, dest_size);
  LowLevelOpcode comp_opcode = select_ll_opcode(MINS_CMPB, src_size);

  auto it = HL_TO_LL.find(hl_opcode);
  LowLevelOpcode set_opcode;
  if (it != HL_TO_LL.end())
  {
    set_opcode = it->second;
  }
  else
  {
    RuntimeError::raise("cannot find opcode");
  }

  // first move the first source into the temp
  check_first_instruction(hl_ins, new Instruction(mov_src_opcode, ll_src_op1, temp1), ll_iseq, first_instruction);
  ll_iseq->append(new Instruction(comp_opcode, ll_src_op2, temp1));
  ll_iseq->append(new Instruction(set_opcode, temp2));
  ll_iseq->append(new Instruction(size_convert_opcode, temp2, temp3));
  ll_iseq->append(new Instruction(mov_dest_opcode, temp3, ll_dest_op));
}

void LowLevelCodeGen::sconv_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode)
{
  bool next_temp_reg = false;
  bool first_instruction = true;
  int num_operand = hl_ins->get_num_operands();
  assert(num_operand == 2);
  Operand dest_op = hl_ins->get_operand(0);
  Operand src_op = hl_ins->get_operand(1);
  int src_size = highlevel_opcode_get_source_operand_size(hl_opcode);
  int dest_size = highlevel_opcode_get_dest_operand_size(hl_opcode);

  Operand temp1 = create_next_temp_reg(next_temp_reg, src_size);
  Operand temp2 = create_next_temp_reg(next_temp_reg, dest_size);
  next_temp_reg = !next_temp_reg;

  Operand ll_dest_op = convert_low_level_operand(dest_op, dest_size, next_temp_reg, first_instruction, false, hl_ins, ll_iseq);
  Operand ll_src_op1 = convert_low_level_operand(src_op, src_size, next_temp_reg, first_instruction, true, hl_ins, ll_iseq);

  LowLevelOpcode mov_src_opcode = select_ll_opcode(MINS_MOVB, src_size);
  LowLevelOpcode size_convert_opcode = double_size_substript_convert(MINS_MOVSBW, src_size, dest_size);
  LowLevelOpcode mov_dest_opcode = select_ll_opcode(MINS_MOVB, dest_size);

  // first move the first source into the temp
  check_first_instruction(hl_ins, new Instruction(mov_src_opcode, ll_src_op1, temp1), ll_iseq, first_instruction);
  ll_iseq->append(new Instruction(size_convert_opcode, temp1, temp2));
  ll_iseq->append(new Instruction(mov_dest_opcode, temp2, ll_dest_op));
}

void LowLevelCodeGen::localaddr_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode)
{
  bool next_temp_reg = false;
  bool first_instruction = true;
  int num_operand = hl_ins->get_num_operands();
  assert(num_operand == 2);
  Operand vreg_op = hl_ins->get_operand(0);
  Operand offset_op = hl_ins->get_operand(1);
  assert(offset_op.is_imm_ival());

  int dest_size = highlevel_opcode_get_dest_operand_size(hl_opcode);
  int offset = m_stack_storage_start + offset_op.get_imm_ival();

  Operand ll_vreg_op = convert_low_level_operand(vreg_op, dest_size, next_temp_reg, first_instruction, false, hl_ins, ll_iseq);
  Operand ll_offset_op = Operand(Operand::MREG64_MEM_OFF, MREG_RBP, offset);
  LowLevelOpcode lea_dest_opcode = MINS_LEAQ;

  if (ll_vreg_op.get_kind() == Operand::MREG64_MEM_OFF)
  {
    Operand temp = create_next_temp_reg(next_temp_reg, dest_size);
    LowLevelOpcode mov_dest_opcode = select_ll_opcode(MINS_MOVB, dest_size);
    check_first_instruction(hl_ins, new Instruction(lea_dest_opcode, ll_offset_op, temp), ll_iseq, first_instruction);
    ll_iseq->append(new Instruction(mov_dest_opcode, temp, ll_vreg_op));
    return;
  }
  check_first_instruction(hl_ins, new Instruction(lea_dest_opcode, ll_offset_op, ll_vreg_op), ll_iseq, first_instruction);
}

void LowLevelCodeGen::neg_opcode(Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, HighLevelOpcode hl_opcode)
{
  bool next_temp_reg = false;
  bool first_instruction = true;
  int num_operand = hl_ins->get_num_operands();
  assert(num_operand == 2);
  Operand dest_op = hl_ins->get_operand(0);
  Operand src_op = hl_ins->get_operand(1);
  int src_size = highlevel_opcode_get_source_operand_size(hl_opcode);
  int dest_size = highlevel_opcode_get_dest_operand_size(hl_opcode);

  Operand ll_dest_op = convert_low_level_operand(dest_op, dest_size, next_temp_reg, first_instruction, false, hl_ins, ll_iseq);
  Operand ll_src_op = convert_low_level_operand(src_op, src_size, next_temp_reg, first_instruction, true, hl_ins, ll_iseq);

  LowLevelOpcode ll_mov_dest_opcode = select_ll_opcode(MINS_MOVB, dest_size);
  LowLevelOpcode ll_sub_opcode = select_ll_opcode(MINS_SUBB, dest_size);

  Operand zero_literal = Operand(Operand::IMM_IVAL, 0);

  if (ll_dest_op.is_in_stack() && ll_src_op.is_in_stack())
  {
    LowLevelOpcode ll_mov_src_opcode = select_ll_opcode(MINS_MOVB, src_size);
    Operand temp = Operand(create_next_temp_reg(next_temp_reg, src_size));
    next_temp_reg = !next_temp_reg;
    check_first_instruction(hl_ins, new Instruction(ll_mov_src_opcode, ll_src_op, temp), ll_iseq, first_instruction);
    ll_src_op = temp;
  }
  check_first_instruction(hl_ins, new Instruction(ll_mov_dest_opcode, zero_literal, ll_dest_op), ll_iseq, first_instruction);
  ll_iseq->append(new Instruction(ll_sub_opcode, ll_src_op, ll_dest_op));
}

Operand LowLevelCodeGen::convert_low_level_operand(Operand hl_operand, int operand_size,
                                                   bool &next_temp_reg, bool &first_instruction, bool is_src,
                                                   Instruction *hl_ins, const std::shared_ptr<InstructionSequence> &ll_iseq)
{
  if (m_optimize && hl_operand.has_comment())
  {
    if (STR_TO_REG.find(hl_operand.get_comment()) == STR_TO_REG.end())
    {
      // memory
      std::string index_idx = hl_operand.get_comment();
      std::size_t pos = index_idx.find("_"); // position of "live" in str
      std::string str3 = index_idx.substr(pos + 1);
      // std::cout << "index is " << str3 << std::endl;
      int idx = std::stoi(str3);
      int offset = m_vreg_storage_start + idx * 8;
      return Operand(Operand::MREG64_MEM_OFF, MREG_RBP, offset);
    }
    // mreg
    MachineReg cur_mreg = STR_TO_REG.find(hl_operand.get_comment())->second;
    if (hl_operand.get_kind() == Operand::VREG)
    {
      Operand::Kind kind = select_mreg_kind(operand_size);
      return Operand(kind, cur_mreg);
    }
    else if (hl_operand.get_kind() == Operand::VREG_MEM)
    {
      return Operand(Operand::MREG64_MEM, cur_mreg);
    }
  }
  if (hl_operand.get_kind() == Operand::VREG || hl_operand.get_kind() == Operand::VREG_MEM)
  {
    int base_reg = hl_operand.get_base_reg();
    Operand return_op;
    if (base_reg >= 10)
    {
      int offset = m_vreg_storage_start + (base_reg - 10) * 8;
      return_op = Operand(Operand::MREG64_MEM_OFF, MREG_RBP, offset);
    }
    else if (base_reg <= 6)
    {
      enum MachineReg reg_name;
      auto it = TO_REG.find(base_reg);
      if (it != TO_REG.end())
      {
        reg_name = it->second;
      }
      else
      {
        RuntimeError::raise("cannot find reg type");
      }
      Operand::Kind kind = select_mreg_kind(operand_size);
      return_op = Operand(kind, reg_name);
    }
    else
    {
      // no need to implement for this assignment
    }
    if (hl_operand.get_kind() == Operand::VREG)
    {
      return return_op;
    }
    else
    {
      Operand temp1 = create_next_temp_reg(next_temp_reg, 8);
      Operand temp2 = Operand(Operand::MREG64_MEM, temp1.get_base_reg());
      next_temp_reg = !next_temp_reg;
      LowLevelOpcode mov_ptr_opcode = select_ll_opcode(MINS_MOVB, 8);
      check_first_instruction(hl_ins, new Instruction(mov_ptr_opcode, return_op, temp1), ll_iseq, first_instruction);
      if (!is_src)
      {
        return temp2;
      }
      Operand temp3 = create_next_temp_reg(next_temp_reg, operand_size);
      next_temp_reg = !next_temp_reg;
      LowLevelOpcode mov_value_opcode = select_ll_opcode(MINS_MOVB, operand_size);
      ll_iseq->append(new Instruction(mov_value_opcode, temp2, temp3));
      return temp3;
    }
  }
  else if (hl_operand.get_kind() == Operand::IMM_IVAL || hl_operand.get_kind() == Operand::IMM_LABEL)
  {
    return Operand(hl_operand);
  }
  return Operand();
}

Operand LowLevelCodeGen::create_next_temp_reg(int next_reg, int operand_size)
{

  enum MachineReg base_reg = (enum MachineReg)(next_reg == 0) ? MREG_R10 : MREG_R11;
  Operand::Kind kind = select_mreg_kind(operand_size);
  return Operand(kind, base_reg);
}

void LowLevelCodeGen::append_first_instruction(Instruction *hl_ins, Instruction *ll_ins, const std::shared_ptr<InstructionSequence> &ll_iseq)
{
  ll_ins->set_comment(hl_format.format_instruction(hl_ins));
  ll_iseq->append(ll_ins);
}

void LowLevelCodeGen::check_first_instruction(Instruction *hl_ins, Instruction *ll_ins, const std::shared_ptr<InstructionSequence> &ll_iseq, bool &first_instruction)
{
  if (first_instruction)
  {
    append_first_instruction(hl_ins, ll_ins, ll_iseq);
    first_instruction = !first_instruction;
  }
  else
  {
    ll_iseq->append(ll_ins);
  }
}

// TODO: implement other private member functions
