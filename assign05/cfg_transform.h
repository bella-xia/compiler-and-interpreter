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
  void localaddr_optimize(Instruction *hl_ins, bool &opt,
                          std::map<Operand, int> &m_vreg_memoffs,
                          std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals);
  void uscon_optimize(Instruction *hl_ins, bool &opt,
                      std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals);
  void compare_optimize(Instruction *hl_ins, std::shared_ptr<InstructionSequence> return_hl_iseq,
                        bool &opt, std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                        std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops);
  void operation_optimize(Instruction *hl_ins, std::shared_ptr<InstructionSequence> return_hl_iseq, bool &opt,
                          std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                          std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops);
  void move_optimize(Instruction *hl_ins, bool &opt,
                     std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals);
  void neg_optimize(Instruction *hl_ins, std::shared_ptr<InstructionSequence> return_hl_iseq,
                    bool &opt, std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                    std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops);

  void check_end_of_block(LiveVregsAnalysis::FactType &end_of_block_facts,
                          std::shared_ptr<InstructionSequence> return_hl_iseq,
                          std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals);

  Operand find_operation(HighLevelOpcode opcode, Operand src1, Operand src2,
                         std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops);
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

#endif // CFG_TRANSFORM_H
