#ifndef CFG_TRANSFORM_H
#define CFG_TRANSFORM_H

#include <memory>
#include <iostream>
#include <set>
#include "cfg.h"
#include "highlevel.h"
#include "live_vregs.h"

class ControlFlowGraphTransform
{
private:
  std::shared_ptr<ControlFlowGraph> m_cfg;
  bool m_is_modified;

public:
  ControlFlowGraphTransform(const std::shared_ptr<ControlFlowGraph> &cfg);
  bool is_modified() { return m_is_modified; }
  virtual ~ControlFlowGraphTransform();

  std::shared_ptr<ControlFlowGraph> get_orig_cfg();
  virtual std::shared_ptr<ControlFlowGraph> transform_cfg();

  // Create a transformed version of the instructions in a basic block.
  // Note that an InstructionSequence "owns" the Instruction objects it contains,
  // and is responsible for deleting them. Therefore, be careful to avoid
  // having two InstructionSequences contain pointers to the same Instruction.
  // If you need to make an exact copy of an Instruction object, you can
  // do so using the duplicate() member function, as follows:
  //
  //    Instruction *orig_ins = /* an Instruction object */
  //    Instruction *dup_ins = orig_ins->duplicate();
  virtual std::shared_ptr<InstructionSequence> transform_basic_block(const InstructionSequence *orig_bb) = 0;
};

class LocalOptimizationHighLevel : ControlFlowGraphTransform
{
private:
  std::shared_ptr<ControlFlowGraph> m_cfg;
  LiveVregs m_live_vreg;
  BasicBlock *m_cur_block;
  bool m_is_modified;

public:
  LocalOptimizationHighLevel(const std::shared_ptr<ControlFlowGraph> &cfg);
  bool is_modified() { return m_is_modified; }
  virtual ~LocalOptimizationHighLevel();
  virtual std::shared_ptr<ControlFlowGraph> transform_cfg();
  virtual std::shared_ptr<InstructionSequence> transform_basic_block(const InstructionSequence *orig_bb);

private:
  void localaddr_optimize(Instruction *hl_ins, bool &opt, bool &deleted,
                          std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                          std::map<Operand, int> &m_vreg_memoffs,
                          std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                          std::shared_ptr<InstructionSequence> return_hl_iseq);
  void uscon_optimize(Instruction *hl_ins, bool &opt, bool &deleted,
                      std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                      std::map<Operand, int> &m_vreg_memoffs,
                      std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                      std::shared_ptr<InstructionSequence> return_hl_iseq);
  void compare_optimize(Instruction *hl_ins, bool &opt, bool &deleted,
                        std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                        std::map<Operand, int> &m_vreg_memoffs,
                        std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                        std::shared_ptr<InstructionSequence> return_hl_iseq);
  void operation_optimize(Instruction *hl_ins, bool &opt, bool &deleted,
                          std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                          std::map<Operand, int> &m_vreg_memoffs,
                          std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                          std::shared_ptr<InstructionSequence> return_hl_iseq);
  void move_optimize(Instruction *hl_ins, bool &opt, bool &deleted,
                     std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                     std::map<Operand, int> &m_vreg_memoffs,
                     std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                     std::shared_ptr<InstructionSequence> return_hl_iseq);
  void neg_optimize(Instruction *hl_ins, bool &opt, bool &deleted,
                    std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                    std::map<Operand, int> &m_vreg_memoffs,
                    std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                    std::shared_ptr<InstructionSequence> return_hl_iseq);

  void check_end_of_block(LiveVregsAnalysis::FactType &end_of_block_facts,
                          std::shared_ptr<InstructionSequence> return_hl_iseq,
                          std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals);

  Operand find_operation(HighLevelOpcode opcode, Operand src1, Operand src2,
                         std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops);

  void erase_relevant_info(Instruction *hl_ins,
                           std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                           std::map<Operand, int> &m_vreg_memoffs,
                           std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                           std::shared_ptr<InstructionSequence> return_hl_iseq);
  void erase_relevant_operations(Operand dest,
                                 std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops);
  std::vector<Operand> erase_relevant_values(Instruction *hl_ins, Operand dest,
                                             std::shared_ptr<InstructionSequence> return_hl_iseq,
                                             std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals);

  HighLevelOpcode get_opcode(HighLevelOpcode basecode, int size);
  int get_src_size(int offset);

  Operand get_un_dereferenced_operand(Operand mem_rf);
  Operand get_orig_operand(Operand cur_operand, std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals, bool iterative);
};

class LocalMregAssignmentHighLevel : ControlFlowGraphTransform
{
private:
  std::shared_ptr<ControlFlowGraph> m_cfg;
  LiveVregs m_live_vreg;
  BasicBlock *m_cur_block;
  int m_spill_size, m_stack_idx, m_max_stack_idx;
  const std::vector<std::string> m_available_mregs{"%r8", "%r9", "%rcx", "%rdx", "%rsi", "%rdi"};
  const std::map<int, std::string> m_vreg_mreg_correspond{{1, "%rdi"}, {2, "%rsi"}, {3, "%rdx"}, {4, "%rcx"}, {5, "r8"}, {6, "r9"}};

public:
  LocalMregAssignmentHighLevel(const std::shared_ptr<ControlFlowGraph> &cfg);
  virtual ~LocalMregAssignmentHighLevel();
  virtual std::shared_ptr<ControlFlowGraph> transform_cfg();
  virtual std::shared_ptr<InstructionSequence> transform_basic_block(const InstructionSequence *orig_bb);
  void set_stack_idx(int idx) { m_stack_idx = idx; }
  int get_max_stack_idx() const { return m_max_stack_idx; }

private:
  void sort_available_mregs(std::vector<std::string> &available_mregs);
  std::tuple<int, int> get_dest_and_src_size(HighLevelOpcode hl_opcode);
  std::tuple<int, int> operation_size_helper(HighLevelOpcode cur_opcode,
                                             HighLevelOpcode base_opcode);
  HighLevelOpcode get_opcode(HighLevelOpcode basecode, int size);
  int get_src_size(int offset);
};

#endif // CFG_TRANSFORM_H
