#include <cassert>
#include "cfg.h"
#include "cfg_transform.h"

ControlFlowGraphTransform::ControlFlowGraphTransform(const std::shared_ptr<ControlFlowGraph> &cfg)
    : m_cfg(cfg), m_is_modified(false)
{
}

ControlFlowGraphTransform::~ControlFlowGraphTransform()
{
}

std::shared_ptr<ControlFlowGraph> ControlFlowGraphTransform::get_orig_cfg()
{
  return m_cfg;
}

std::shared_ptr<ControlFlowGraph> ControlFlowGraphTransform::transform_cfg()
{
  std::shared_ptr<ControlFlowGraph> result(new ControlFlowGraph());

  // map of basic blocks of original CFG to basic blocks in transformed CFG
  std::map<BasicBlock *, BasicBlock *> block_map;

  // iterate over all basic blocks, transforming each one
  for (auto i = m_cfg->bb_begin(); i != m_cfg->bb_end(); i++)
  {
    BasicBlock *orig = *i;

    // Transform the instructions
    std::shared_ptr<InstructionSequence>
        transformed_bb = transform_basic_block(orig);

    // Create transformed basic block; note that we set its
    // code order to be the same as the code order of the original
    // block (with the hope of eventually reconstructing an InstructionSequence
    // with the transformed blocks in an order that matches the original
    // block order)
    BasicBlock *result_bb = result->create_basic_block(orig->get_kind(), orig->get_code_order(), orig->get_label());
    for (auto j = transformed_bb->cbegin(); j != transformed_bb->cend(); ++j)
      result_bb->append((*j)->duplicate());

    block_map[orig] = result_bb;
  }

  // add edges to transformed CFG
  for (auto i = m_cfg->bb_begin(); i != m_cfg->bb_end(); i++)
  {
    BasicBlock *orig = *i;
    const ControlFlowGraph::EdgeList &outgoing_edges = m_cfg->get_outgoing_edges(orig);
    for (auto j = outgoing_edges.cbegin(); j != outgoing_edges.cend(); j++)
    {
      Edge *orig_edge = *j;

      BasicBlock *transformed_source = block_map[orig_edge->get_source()];
      BasicBlock *transformed_target = block_map[orig_edge->get_target()];

      result->create_edge(transformed_source, transformed_target, orig_edge->get_kind());
    }
  }

  return result;
}

LocalOptimizationHighLevel::LocalOptimizationHighLevel(const std::shared_ptr<ControlFlowGraph> &cfg) : ControlFlowGraphTransform(cfg),
                                                                                                       m_cfg(cfg),
                                                                                                       m_live_vreg(LiveVregs(cfg)),
                                                                                                       m_cur_block(nullptr),
                                                                                                       m_is_modified(false)

{
  m_live_vreg.execute();
}

LocalOptimizationHighLevel::~LocalOptimizationHighLevel()
{
}

std::shared_ptr<ControlFlowGraph> LocalOptimizationHighLevel::transform_cfg()
{
  std::shared_ptr<ControlFlowGraph> result(new ControlFlowGraph());

  // map of basic blocks of original CFG to basic blocks in transformed CFG
  std::map<BasicBlock *, BasicBlock *> block_map;

  // iterate over all basic blocks, transforming each one
  for (auto it = m_cfg->bb_begin(); it != m_cfg->bb_end(); it++)
  {
    // obtain the current block and the liveness analysis at the end of it
    m_cur_block = *it;

    // Transform the instructions
    std::shared_ptr<InstructionSequence>
        transformed_bb = transform_basic_block(m_cur_block);

    // Create transformed basic block; note that we set its
    // code order to be the same as the code order of the original
    // block (with the hope of eventually reconstructing an InstructionSequence
    // with the transformed blocks in an order that matches the original
    // block order)
    BasicBlock *result_bb = result->create_basic_block(m_cur_block->get_kind(),
                                                       m_cur_block->get_code_order(),
                                                       m_cur_block->get_label());
    for (auto j = transformed_bb->cbegin(); j != transformed_bb->cend(); ++j)
      result_bb->append((*j)->duplicate());

    block_map[m_cur_block] = result_bb;
  }

  // add edges to transformed CFG
  for (auto i = m_cfg->bb_begin(); i != m_cfg->bb_end(); i++)
  {
    m_cur_block = *i;
    const ControlFlowGraph::EdgeList &outgoing_edges = m_cfg->get_outgoing_edges(m_cur_block);
    for (auto j = outgoing_edges.cbegin(); j != outgoing_edges.cend(); j++)
    {
      Edge *orig_edge = *j;

      BasicBlock *transformed_source = block_map[orig_edge->get_source()];
      BasicBlock *transformed_target = block_map[orig_edge->get_target()];

      result->create_edge(transformed_source, transformed_target, orig_edge->get_kind());
    }
  }
  return result;
}

std::shared_ptr<InstructionSequence> LocalOptimizationHighLevel::transform_basic_block(const InstructionSequence *orig_bb)
{
  std::map<Operand, std::tuple<Operand, int>> m_vreg_vals;
  std::map<Operand, int> m_vreg_memoffs;
  std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> m_vreg_ops;
  std::shared_ptr<InstructionSequence>
      return_hl_iseq(new InstructionSequence());
  bool opt, deleted;
  bool has_jump = false;
  for (auto it = orig_bb->cbegin(); it != orig_bb->cend(); it++)
  {
    Instruction *hl_ins = *it;
    HighLevelOpcode hl_opcode = HighLevelOpcode(hl_ins->get_opcode());
    // // std::cout << hl_opcode << std::endl;
    opt = false;
    deleted = false;
    std::vector<Operand> delete_after_instruction;

    // perform move instruction optimization
    if (hl_ins->get_num_operands() > 1 && hl_opcode < HINS_cjmp_t)
    {
      delete_after_instruction = erase_relevant_values(hl_ins, hl_ins->get_operand(0), return_hl_iseq,
                                                       m_vreg_vals);
    }
    if (hl_opcode == HINS_call)
    {
      Operand identifier = Operand(Operand::VREG, 0);
      erase_relevant_operations(identifier, m_vreg_ops);
      delete_after_instruction = erase_relevant_values(hl_ins, identifier, return_hl_iseq, m_vreg_vals);
    }
    else if (hl_opcode >= HINS_mov_b && hl_opcode <= HINS_mov_q)
    {
      move_optimize(hl_ins, opt, deleted, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
    }
    // perform add / subtract / multiply / divide operation optimization
    else if (hl_opcode >= HINS_add_b && hl_opcode <= HINS_mod_q)
    {
      operation_optimize(hl_ins, opt, deleted, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
    }
    // perform compare instruction optimization
    else if (hl_opcode >= HINS_cmplt_b && hl_opcode <= HINS_cmpneq_q)
    {
      compare_optimize(hl_ins, opt, deleted, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
    }
    // perform upscale instruction optimization
    else if (hl_opcode >= HINS_sconv_bw && hl_opcode <= HINS_uconv_lq)
    {
      uscon_optimize(hl_ins, opt, deleted, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
    }
    // perform getting local address optimization
    else if (hl_opcode == HINS_localaddr)
    {
      localaddr_optimize(hl_ins, opt, deleted, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
    }
    else if (hl_opcode >= HINS_neg_b && hl_opcode <= HINS_neg_q)
    {
      neg_optimize(hl_ins, opt, deleted, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
    }
    // perform jump instruction optimization
    else if (hl_opcode == HINS_jmp || hl_opcode == HINS_cjmp_t || hl_opcode == HINS_cjmp_f)
    {
      has_jump = true;
      LiveVregsAnalysis::FactType end_of_jump_facttype = m_live_vreg.get_fact_before_instruction(m_cur_block, hl_ins);
      check_end_of_block(end_of_jump_facttype, return_hl_iseq, m_vreg_vals);
    }

    // eliminate unnecessary commands
    else if (hl_opcode >= HINS_spill_b && hl_opcode <= HINS_spill_q)
    {
      opt = true;
    }
    // perform optimization for the remaining statements based on local value numbering and
    // copy propagation
    if (!opt)
    {
      // construct a vector to store the updated operands
      std::vector<Operand> new_ops;

      // check all src operand and see if it can be move into a val literal or a copied original operand
      int n_op = hl_ins->get_num_operands();
      // if the instruction has no or one operand, it is not optimizable. Can move on
      if (n_op == 1 || n_op == 0)
      {
        return_hl_iseq->append(new Instruction(*hl_ins));
      }
      else
      {
        // if the destination is a memory reference, see if any copy propagation has been performed on the
        // pointer virtual register
        Operand dest = hl_ins->get_operand(0);
        Operand dest_orig = get_orig_operand(dest, m_vreg_vals, false);
        if (!(dest_orig == dest))
        {
          m_is_modified = true;
          dest = dest_orig;
        }
        // iterate over all the remaining source operands
        for (int i = 1; i < (int)hl_ins->get_num_operands(); ++i)
        {
          // optimize any instance of copy propagation if the original operand is of kind
          // virtual register or virtual register dereferenced
          Operand orig_op = hl_ins->get_operand(i);
          new_ops.push_back(get_orig_operand(orig_op, m_vreg_vals, false));
        }

        // check if there is redundant move
        // specifically, moving a virtual register to itself
        bool redundant = (hl_opcode >= HINS_mov_b && hl_opcode <= HINS_mov_q) &&
                         (dest.get_kind() == Operand::VREG && new_ops.at(0).get_kind() == Operand::VREG &&
                          dest.get_base_reg() == new_ops.at(0).get_base_reg());
        if (!redundant)
        {
          // otherwise, append the instruction to the optimized sequence of instructions
          return_hl_iseq->append(((int)new_ops.size() == 1)
                                     ? new Instruction(hl_ins->get_opcode(), dest, new_ops.at(0))
                                     : new Instruction(hl_ins->get_opcode(), dest, new_ops.at(0), new_ops.at(1)));
        }
      }
    }
    if (!deleted)
    {
      if (hl_ins->get_num_operands() > 1 && hl_opcode < HINS_cjmp_t)
      {
        erase_relevant_info(hl_ins, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
      }
    }

    for (int i = 0; i < (int)delete_after_instruction.size(); ++i)
    {
      m_vreg_vals.erase(delete_after_instruction.at(i));
    }
  }
  if (!has_jump)
  {
    LiveVregsAnalysis::FactType end_of_block_facttype = m_live_vreg.get_fact_at_end_of_block(m_cur_block);
    check_end_of_block(end_of_block_facttype, return_hl_iseq, m_vreg_vals);
  }

  // return optimized instructions
  return return_hl_iseq;
}

/**
 * used to optimize local address instruction
 * @param hl_ins the current instruction
 * @param m_vreg_memoffs storage for all the virtual register with their corresponding offsets in memory
 * @param m_vreg_vals storage for all copy propagation and constant value registers
 * @param opt boolean value used to determine whether optimization has been performed
 */
void LocalOptimizationHighLevel::localaddr_optimize(Instruction *hl_ins, bool &opt, bool &deleted,
                                                    std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                                                    std::map<Operand, int> &m_vreg_memoffs,
                                                    std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                                                    std::shared_ptr<InstructionSequence> return_hl_iseq)
{
  // obtain the destination operand and the offset in memory attempted to be accessed
  Operand dest = hl_ins->get_operand(0);
  int offset = hl_ins->get_operand(1).get_imm_ival();
  if (dest.get_kind() == Operand::VREG)
  {
    // // std::cout << "trying to find vreg" << dest.get_base_reg() << "in localaddr with offset " << offset << std::endl;
    Operand key = Operand();
    // check if a corresponding memory offset is already stored in one of the virtual registers
    // (in which case no need to perform another localaddr)
    for (std::pair<Operand, int> vreg_memoff : m_vreg_memoffs)
    {
      if (offset == vreg_memoff.second)
      {
        key = vreg_memoff.first;
        break;
      }
    }
    // if the current virtual register stores exactly what is needed,
    // then no need for any operation
    if (key == dest)
    {
      // // std::cout << "same as before, no need to move" << std::endl;
      deleted = true;
      opt = true;
      m_is_modified = true;
      // std::cout << "no need to move localaddr bc the current virtual register stores the offset needed is still alive" << std::endl;
    }
    // if some other register stores what is needed,
    // then only a move instruction will be performed
    else if (key.get_kind() != Operand::NONE)
    {
      erase_relevant_info(hl_ins, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
      deleted = true;
      m_vreg_vals.insert(std::pair(dest, std::make_tuple(key, 8)));
      opt = true;
      m_is_modified = true;
    }
    else
    {
      // localaddr still needs to be performed, but retains the current virtual register information for
      // future reference
      erase_relevant_info(hl_ins, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
      deleted = true;
      m_vreg_memoffs.insert(std::pair<Operand, int>(dest, offset));
    }
  }
}

/**
 * used to optimize compare instruction
 * @param hl_ins the current instruction
 * @param return_hl_iseq optimized instruction requence
 * @param m_vreg_vals storage for all copy propagation and constant value registers
 * @param opt boolean value used to determine whether optimization has been performed
 */
void LocalOptimizationHighLevel::compare_optimize(Instruction *hl_ins, bool &opt, bool &deleted,
                                                  std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                                                  std::map<Operand, int> &m_vreg_memoffs,
                                                  std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                                                  std::shared_ptr<InstructionSequence> return_hl_iseq)
{
  // obtain all the necessary information
  int hl_opcode = hl_ins->get_opcode();
  int hl_opcode_div = (hl_opcode - HINS_cmplt_b) / 4;
  int hl_opcode_offset = (hl_opcode - HINS_cmplt_b) % 4;
  Operand dest(hl_ins->get_operand(0));
  Operand src1(hl_ins->get_operand(1));
  Operand src2(hl_ins->get_operand(2));
  int src_size = get_src_size(hl_opcode_offset);
  // if both source one and source two operands are immediate values,
  // can update this to a move instruction
  if (src1.get_kind() == Operand::IMM_IVAL &&
      src2.get_kind() == Operand::IMM_IVAL)
  {
    long src1_val = src1.get_imm_ival();
    long src2_val = src2.get_imm_ival();
    long result;
    switch (hl_opcode_div)
    {
    case 0:
      // compare less than
      result = src1_val < src2_val;
      break;
    case 1:
      // compare less than equal to
      result = src1_val <= src2_val;
      break;
    case 2:
      // compare greater than
      result = src1_val > src2_val;
      break;
    case 3:
      // compare greater than equal to
      result = src1_val >= src2_val;
      break;
    case 4:
      // compare equal to
      result = src1_val == src2_val;
      break;
    case 5:
      // compare not equal to
      result = src1_val != src2_val;
      break;
    default:
      break;
    }
    // if the destination is a deferenced virtual register, then move the result here
    if (dest.get_kind() == Operand::VREG_MEM)
    {
      HighLevelOpcode new_opcode = static_cast<HighLevelOpcode>(HINS_mov_b + hl_opcode_offset);
      return_hl_iseq->append(new Instruction(new_opcode, dest, Operand(Operand::IMM_IVAL, result)));
      opt = true;
      m_is_modified = true;
    }
    else if (dest.get_kind() == Operand::VREG)
    {
      // otherwise if it is a virtual register, can just store the value somewhere
      erase_relevant_info(hl_ins, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
      deleted = true;
      m_vreg_vals.insert(std::pair(dest, std::make_tuple(Operand(Operand::IMM_IVAL, result), 4)));
      //}
      opt = true;
      m_is_modified = true;
    }
  }
  else if (dest.get_kind() == Operand::VREG &&
           (src1.get_kind() == Operand::IMM_IVAL || src1.get_kind() == Operand::VREG || src1.get_kind() == Operand::VREG_MEM) &&
           (src2.get_kind() == Operand::IMM_IVAL || src2.get_kind() == Operand::VREG || src2.get_kind() == Operand::VREG_MEM))
  {
    src1 = get_orig_operand(src1, m_vreg_vals, true);
    src2 = get_orig_operand(src2, m_vreg_vals, true);
    Operand get_op = find_operation(static_cast<HighLevelOpcode>(hl_opcode), src1, src2, m_vreg_ops);
    if (get_op == dest)
    {
      deleted = true;
      opt = true;
      m_is_modified = true;
    }
    else if (get_op.get_kind() != Operand::NONE)
    {
      erase_relevant_info(hl_ins, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
      deleted = true;
      m_vreg_vals.insert(std::pair(dest, std::make_tuple(get_op, src_size)));
      opt = true;
      m_is_modified = true;
    }
    else
    {
      erase_relevant_info(hl_ins, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
      deleted = true;
      m_vreg_ops.insert(std::pair(std::make_tuple(static_cast<HighLevelOpcode>(hl_opcode), src1, src2),
                                  dest));
    }
  }
}

/**
 * used to optimize compare instruction
 * @param hl_ins the current instruction
 * @param m_vreg_vals storage for all copy propagation and constant value registers
 * @param opt boolean value used to determine whether optimization has been performed
 */
void LocalOptimizationHighLevel::uscon_optimize(Instruction *hl_ins, bool &opt, bool &deleted,
                                                std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                                                std::map<Operand, int> &m_vreg_memoffs,
                                                std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                                                std::shared_ptr<InstructionSequence> return_hl_iseq)
{
  // get necessary sources
  Operand dest = hl_ins->get_operand(0);
  Operand src = hl_ins->get_operand(1);
  int dest_size = 0;
  int opcode = hl_ins->get_opcode();
  // ensure the destination size
  switch (opcode)
  {
  case HINS_sconv_bw:
  case HINS_uconv_bw:
    dest_size = 2;
    break;
  case HINS_sconv_bl:
  case HINS_uconv_bl:
  case HINS_sconv_wl:
  case HINS_uconv_wl:
    dest_size = 4;
    break;
  default:
    dest_size = 8;
    break;
  }
  // if the current usconv is on an immediate value, can use it
  if (dest.get_kind() == Operand::VREG &&
      src.get_kind() == Operand::IMM_IVAL)
  {
    erase_relevant_info(hl_ins, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
    deleted = true;
    m_vreg_vals.insert(std::pair(dest, std::make_tuple(Operand(src), dest_size)));
    //}
    opt = true;
    m_is_modified = true;
  }
}

/**
 * used to optimize operation instruction
 * @param hl_ins the current instruction
 * @param m_vreg_vals storage for all copy propagation and constant value registers
 * @param m_vreg_ops storage for all operations
 * @param return_hl_iseq optimized instruction requence
 * @param opt boolean value used to determine whether optimization has been performed
 */
void LocalOptimizationHighLevel::operation_optimize(Instruction *hl_ins, bool &opt, bool &deleted,
                                                    std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                                                    std::map<Operand, int> &m_vreg_memoffs,
                                                    std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                                                    std::shared_ptr<InstructionSequence> return_hl_iseq)
{
  // get necessary info
  int hl_opcode = hl_ins->get_opcode();
  int hl_opcode_div = (hl_opcode - HINS_add_b) / 4;
  int hl_opcode_offset = (hl_opcode - HINS_add_b) % 4;
  int src_size = get_src_size(hl_opcode_offset);
  Operand dest(hl_ins->get_operand(0));
  Operand src1(hl_ins->get_operand(1));
  Operand src2(hl_ins->get_operand(2));
  // check if uncessary operations are performed: e.g. adding / subtracting 0; multiplying / dividing 1
  bool add_zero = dest.get_kind() == Operand::VREG && hl_opcode >= HINS_add_b && hl_opcode <= HINS_add_q &&
                  ((src1.get_kind() == Operand::VREG && src2.get_kind() == Operand::IMM_IVAL && src2.get_imm_ival() == 0) ||
                   (src1.get_kind() == Operand::IMM_IVAL && src2.get_kind() == Operand::VREG && src1.get_imm_ival() == 0));
  bool minus_zero = dest.get_kind() == Operand::VREG && hl_opcode >= HINS_sub_b && hl_opcode <= HINS_sub_q &&
                    src1.get_kind() == Operand::VREG && src2.get_kind() == Operand::IMM_IVAL && src2.get_imm_ival() == 0;
  bool mult_one = dest.get_kind() == Operand::VREG && hl_opcode >= HINS_mul_b && hl_opcode <= HINS_mul_q &&
                  ((src1.get_kind() == Operand::VREG && src2.get_kind() == Operand::IMM_IVAL &&
                    src2.get_imm_ival() == 1) ||
                   (src1.get_kind() == Operand::IMM_IVAL &&
                    src2.get_kind() == Operand::VREG && src1.get_imm_ival() == 1));
  bool div_one = dest.get_kind() == Operand::VREG && hl_opcode >= HINS_div_b && hl_opcode <= HINS_div_q &&
                 src1.get_kind() == Operand::VREG && src2.get_kind() == Operand::IMM_IVAL &&
                 src2.get_imm_ival() == 1;
  if (add_zero || minus_zero || mult_one || div_one)
  {
    Operand identifier = (src1.get_kind() == Operand::VREG) ? Operand(src1) : Operand(src2);
    identifier = get_orig_operand(identifier, m_vreg_vals, true);
    erase_relevant_info(hl_ins, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
    deleted = true;
    m_vreg_vals.insert(std::pair(dest, std::make_tuple(identifier, src_size)));
    opt = true;
    m_is_modified = true;
  }
  else if (dest.get_kind() == Operand::VREG &&
           src1.get_kind() == Operand::IMM_IVAL &&
           src2.get_kind() == Operand::IMM_IVAL)
  {
    // immediate values that can be calculated
    long src1_val = src1.get_imm_ival();
    long src2_val = src2.get_imm_ival();
    long result;
    switch (hl_opcode_div)
    {
    case 0:
      // add operation
      result = src1_val + src2_val;
      break;
    case 1:
      // minus operation
      result = src1_val - src2_val;
      break;
    case 2:
      // times operation
      result = src1_val * src2_val;
      break;
    case 3:
      // divide operation
      result = src1_val / src2_val;
      break;
    case 4:
      // mod operation
      result = src1_val % src2_val;
      break;
    default:
      break;
    }
    HighLevelOpcode new_opcode = static_cast<HighLevelOpcode>(HINS_mov_b + hl_opcode_offset);
    return_hl_iseq->append(new Instruction(new_opcode, dest, Operand(Operand::IMM_IVAL, result)));
    opt = true;
    m_is_modified = true;
  }
  else if (dest.get_kind() == Operand::VREG &&
           (src1.get_kind() == Operand::VREG || src1.get_kind() == Operand::VREG_MEM || src1.get_kind() == Operand::IMM_IVAL) &&
           (src2.get_kind() == Operand::VREG || src2.get_kind() == Operand::VREG_MEM || src2.get_kind() == Operand::IMM_IVAL))
  {
    src1 = get_orig_operand(src1, m_vreg_vals, true);
    src2 = get_orig_operand(src2, m_vreg_vals, true);
    Operand get_op = find_operation(static_cast<HighLevelOpcode>(hl_opcode), src1, src2, m_vreg_ops);
    if (get_op == dest)
    {
      deleted = true;
      opt = true;
      m_is_modified = true;
    }
    else if (get_op.get_kind() != Operand::NONE)
    {
      erase_relevant_info(hl_ins, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
      deleted = true;
      m_vreg_vals.insert(std::pair(dest, std::make_tuple(get_op, src_size)));
      opt = true;
      m_is_modified = true;
    }
    else
    {
      erase_relevant_info(hl_ins, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
      deleted = true;
      m_vreg_ops.insert(std::pair(std::make_tuple(static_cast<HighLevelOpcode>(hl_opcode), src1, src2),
                                  dest));
    }
  }
}

/**
 * used to optimize move instruction
 * @param hl_ins the current instruction
 * @param m_vreg_vals storage for all copy propagation and constant value registers
 * @param opt boolean value used to determine whether optimization has been performed
 */
void LocalOptimizationHighLevel::move_optimize(Instruction *hl_ins, bool &opt, bool &deleted,
                                               std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                                               std::map<Operand, int> &m_vreg_memoffs,
                                               std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                                               std::shared_ptr<InstructionSequence> return_hl_iseq)
{
  // obtain necessary info
  Operand dest(hl_ins->get_operand(0));
  Operand src(hl_ins->get_operand(1));
  int opcode = hl_ins->get_opcode();
  int src_size = get_src_size(opcode - HINS_mov_b);
  // if the current destination is vreg, then store the value in the temporary data structure
  // until needed
  if (dest.get_kind() == Operand::VREG)
  {
    if (dest.get_base_reg() >= 10 && src.get_kind() >= 10 &&
        (src.get_kind() == Operand::IMM_IVAL || src.get_kind() == Operand::VREG))
    {
      Operand identifier = Operand(src);
      identifier = get_orig_operand(identifier, m_vreg_vals, true);
      if (identifier == dest)
      {
        deleted = true;
        opt = true;
        m_is_modified = true;
      }
      else
      {
        erase_relevant_info(hl_ins, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
        deleted = true;
        m_vreg_vals.insert(std::pair(dest, std::make_tuple(identifier, src_size)));
        opt = true;
        m_is_modified = true;
      }
    }
  }
}

void LocalOptimizationHighLevel::neg_optimize(Instruction *hl_ins, bool &opt, bool &deleted,
                                              std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                                              std::map<Operand, int> &m_vreg_memoffs,
                                              std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                                              std::shared_ptr<InstructionSequence> return_hl_iseq)
{
  HighLevelOpcode hl_opcode = static_cast<HighLevelOpcode>(hl_ins->get_opcode());
  Operand dest = hl_ins->get_operand(0);
  Operand src = hl_ins->get_operand(1);
  int src_size = get_src_size(hl_opcode - HINS_neg_b);
  src = get_orig_operand(src, m_vreg_vals, true);
  if (dest.get_kind() == Operand::VREG && src.get_kind() == Operand::IMM_IVAL)
  {
    m_vreg_vals.insert(std::pair(dest, std::make_tuple(
                                           Operand(Operand::IMM_IVAL, src.get_imm_ival() * -1), src_size)));
    opt = true;
    m_is_modified = true;
  }
  else
  {
    Operand similar_op = find_operation(hl_opcode, src, Operand(Operand::LABEL, "0"), m_vreg_ops);
    if (similar_op == dest)
    {
      deleted = true;
      m_is_modified = true;
      opt = true;
    }
    else if (similar_op.get_kind() != Operand::NONE)
    {
      HighLevelOpcode move_opcode = static_cast<HighLevelOpcode>(HINS_mov_b + hl_opcode - HINS_neg_b);
      return_hl_iseq->append(new Instruction(move_opcode, dest, similar_op));
      opt = true;
      m_is_modified = true;
    }
    else
    {
      erase_relevant_info(hl_ins, m_vreg_vals, m_vreg_memoffs, m_vreg_ops, return_hl_iseq);
      deleted = true;
      m_vreg_ops.insert(std::pair(std::make_tuple(hl_opcode, src, Operand(Operand::LABEL, "0")), dest));
    }
  }
}

Operand LocalOptimizationHighLevel::find_operation(HighLevelOpcode opcode, Operand src1, Operand src2,
                                                   std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops)
{
  if (src1.is_imm_ival() && src2.is_imm_ival())
  {
    return Operand();
  }
  if (m_vreg_ops.find(std::make_tuple(opcode, src1, src2)) != m_vreg_ops.end())
  {
    return m_vreg_ops.find(std::make_tuple(opcode, src1, src2))->second;
  }
  if (opcode <= HINS_add_q || (opcode >= HINS_mul_b && opcode <= HINS_mul_q))
  {
    if (m_vreg_ops.find(std::make_tuple(opcode, src2, src1)) != m_vreg_ops.end())
    {
      return m_vreg_ops.find(std::make_tuple(opcode, src2, src1))->second;
    }
  }
  return Operand();
}

void LocalOptimizationHighLevel::erase_relevant_operations(Operand dest, std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops)
{
  std::vector<std::tuple<HighLevelOpcode, Operand, Operand>> items_to_delete;
  for (std::pair<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> ele : m_vreg_ops)
  {
    Operand map_dest = ele.second;
    Operand map_src1 = std::get<1>(ele.first);
    Operand map_src2 = std::get<2>(ele.first);
    if (map_dest == dest || map_src1 == dest || map_src2 == dest)
    {
      items_to_delete.push_back(ele.first);
    }
  }
  for (std::tuple<HighLevelOpcode, Operand, Operand> key : items_to_delete)
  {
    m_vreg_ops.erase(key);
  }
}

std::vector<Operand> LocalOptimizationHighLevel::erase_relevant_values(Instruction *hl_ins, Operand dest,
                                                                       std::shared_ptr<InstructionSequence> return_hl_iseq,
                                                                       std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals)
{
  std::vector<Operand> items_to_delete_after_instruction;
  std::vector<Operand> items_to_delete;
  LiveVregsAnalysis::FactType before_liveness = m_live_vreg.get_fact_before_instruction(m_cur_block, hl_ins);
  LiveVregsAnalysis::FactType after_liveness = m_live_vreg.get_fact_after_instruction(m_cur_block, hl_ins);
  for (std::pair<Operand, std::tuple<Operand, int>> ele : m_vreg_vals)
  {
    Operand map_dest = ele.first;
    Operand map_src = std::get<0>(ele.second);
    int src_size = std::get<1>(ele.second);
    if ((map_dest.get_kind() != Operand::NONE && map_dest == dest) ||
        (map_src.get_kind() != Operand::NONE && map_src == dest))
    {
      if ((map_src.get_kind() != Operand::NONE && map_src == dest) &&
          !after_liveness.test(ele.first.get_base_reg()) && before_liveness.test(ele.first.get_base_reg()))
      {
        items_to_delete_after_instruction.push_back(map_dest);
      }
      else
      {
        items_to_delete.push_back(ele.first);

        if ((map_src.get_kind() != Operand::NONE && map_src == dest) &&
            after_liveness.test(ele.first.get_base_reg()))
        {
          HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, src_size);
          return_hl_iseq->append(new Instruction(mov_opcode, map_dest, map_src));
          items_to_delete.push_back(ele.first);
        }
      }
    }
  }
  for (Operand key : items_to_delete)
  {
    m_vreg_vals.erase(key);
  }
  return items_to_delete_after_instruction;
}

void LocalOptimizationHighLevel::erase_relevant_info(Instruction *hl_ins,
                                                     std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                                                     std::map<Operand, int> &m_vreg_memoffs,
                                                     std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops,
                                                     std::shared_ptr<InstructionSequence> return_hl_iseq)
{
  Operand dest = hl_ins->get_operand(0);
  // if the destination of the current instruction is contained in memory offset / vreg data structure,
  // then remove it
  if (m_vreg_memoffs.find(dest) != m_vreg_memoffs.end())
  {
    m_vreg_memoffs.erase(dest);
  }
  if (dest.get_kind() == Operand::VREG)
  {
    erase_relevant_operations(dest, m_vreg_ops);
  }
  // if a value is assigned to the dereferenced format,
  // then change any reference to its dereferenced value
  if (dest.get_kind() == Operand::VREG_MEM)
  {
    Operand identifier = get_orig_operand(dest, m_vreg_vals, false);
    erase_relevant_operations(identifier, m_vreg_ops);
  }
}

HighLevelOpcode LocalOptimizationHighLevel::get_opcode(HighLevelOpcode basecode, int size)
{
  switch (size)
  {
  case 2:
    return static_cast<HighLevelOpcode>(basecode + 1);
  case 4:
    return static_cast<HighLevelOpcode>(basecode + 2);
  case 8:
    return static_cast<HighLevelOpcode>(basecode + 3);
  default:
    break;
  }
  return basecode;
}

int LocalOptimizationHighLevel::get_src_size(int offset)
{
  switch (offset)
  {
  case 0:
    return 1;
  case 1:
    return 2;
  case 2:
    return 4;
  default:
    break;
  }
  return 8;
}

void LocalOptimizationHighLevel::check_end_of_block(LiveVregsAnalysis::FactType &end_of_block_facts,
                                                    std::shared_ptr<InstructionSequence> return_hl_iseq,
                                                    std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals)
{
  // after iteration, check the end-of-block liveness value, and if applicable, move it to the corresponding place
  for (std::pair<Operand, std::tuple<Operand, int>> ele : m_vreg_vals)
  {
    Operand dest_vreg = ele.first;
    if (end_of_block_facts.test(dest_vreg.get_base_reg()))
    {
      Operand src = std::get<0>(ele.second);
      int src_size = std::get<1>(ele.second);
      HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, src_size);
      return_hl_iseq->append(new Instruction(mov_opcode, dest_vreg, src));
    }
  }
  m_vreg_vals.clear();
}

Operand LocalOptimizationHighLevel::get_un_dereferenced_operand(Operand mem_rf)
{
  assert(mem_rf.get_kind() == Operand::VREG_MEM);
  return Operand(Operand::VREG, mem_rf.get_base_reg());
}

Operand LocalOptimizationHighLevel::get_orig_operand(Operand cur_operand, std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                                                     bool iterative)
{
  if (cur_operand.get_kind() == Operand::VREG)
  {
    if (iterative)
    {
      Operand op = Operand(cur_operand);
      while (m_vreg_vals.find(op) != m_vreg_vals.end())
      {
        op = std::get<0>(m_vreg_vals[op]);
      }
      return op;
    }
    else
    {
      if (m_vreg_vals.find(cur_operand) != m_vreg_vals.end())
      {
        return std::get<0>(m_vreg_vals[cur_operand]);
      }
    }
  }
  else if (cur_operand.get_kind() == Operand::VREG_MEM)
  {
    if (iterative)
    {
      Operand deref_operand = get_un_dereferenced_operand(cur_operand);
      while (m_vreg_vals.find(deref_operand) != m_vreg_vals.end())
      {
        deref_operand = std::get<0>(m_vreg_vals[deref_operand]);
      }
      return Operand(Operand::VREG_MEM, deref_operand.get_base_reg());
    }
    else
    {
      Operand deref_operand = get_un_dereferenced_operand(cur_operand);
      if (m_vreg_vals.find(deref_operand) != m_vreg_vals.end())
      {
        return Operand(Operand::VREG_MEM, std::get<0>(m_vreg_vals[deref_operand]).get_base_reg());
      }
    }
  }
  return cur_operand;
}

////////////////////////////////////////////////////////////////////////
// LocalMregAssignmentHighLevel
////////////////////////////////////////////////////////////////////////
LocalMregAssignmentHighLevel::LocalMregAssignmentHighLevel(const std::shared_ptr<ControlFlowGraph> &cfg) : ControlFlowGraphTransform(cfg),
                                                                                                           m_cfg(cfg),
                                                                                                           m_live_vreg(LiveVregs(cfg)),
                                                                                                           m_cur_block(nullptr),
                                                                                                           m_spill_size(0),
                                                                                                           m_stack_idx(0),
                                                                                                           m_max_stack_idx(0)
{
  m_live_vreg.execute();
}

LocalMregAssignmentHighLevel::~LocalMregAssignmentHighLevel()
{
}

std::shared_ptr<ControlFlowGraph> LocalMregAssignmentHighLevel::transform_cfg()
{
  std::shared_ptr<ControlFlowGraph> result(new ControlFlowGraph());

  // map of basic blocks of original CFG to basic blocks in transformed CFG
  std::map<BasicBlock *, BasicBlock *> block_map;

  // iterate over all basic blocks, transforming each one
  for (auto i = m_cfg->bb_begin(); i != m_cfg->bb_end(); i++)
  {
    m_cur_block = *i;

    // Transform the instructions
    std::shared_ptr<InstructionSequence>
        transformed_bb = transform_basic_block(m_cur_block);

    // Create transformed basic block; note that we set its
    // code order to be the same as the code order of the original
    // block (with the hope of eventually reconstructing an InstructionSequence
    // with the transformed blocks in an order that matches the original
    // block order)
    BasicBlock *result_bb = result->create_basic_block(m_cur_block->get_kind(), m_cur_block->get_code_order(), m_cur_block->get_label());
    for (auto j = transformed_bb->cbegin(); j != transformed_bb->cend(); ++j)
      result_bb->append((*j)->duplicate());

    block_map[m_cur_block] = result_bb;
  }

  // add edges to transformed CFG
  for (auto i = m_cfg->bb_begin(); i != m_cfg->bb_end(); i++)
  {
    BasicBlock *orig = *i;
    const ControlFlowGraph::EdgeList &outgoing_edges = m_cfg->get_outgoing_edges(orig);
    for (auto j = outgoing_edges.cbegin(); j != outgoing_edges.cend(); j++)
    {
      Edge *orig_edge = *j;

      BasicBlock *transformed_source = block_map[orig_edge->get_source()];
      BasicBlock *transformed_target = block_map[orig_edge->get_target()];

      result->create_edge(transformed_source, transformed_target, orig_edge->get_kind());
    }
  }

  return result;
}

std::shared_ptr<InstructionSequence> LocalMregAssignmentHighLevel::transform_basic_block(const InstructionSequence *orig_bb)
{
  int block_stack_idx = m_stack_idx;
  std::shared_ptr<InstructionSequence>
      return_hl_iseq(new InstructionSequence());
  std::vector<std::string> available_mregs;
  std::vector<std::string> restore_until_next_call;
  std::map<int, int> stored_before_call;
  for (std::string mreg : m_available_mregs)
  {
    available_mregs.push_back(mreg);
  }
  std::map<int, std::string> pairings;
  for (auto it = orig_bb->cbegin(); it != orig_bb->cend(); it++)
  {
    Instruction *hl_ins = *it;
    HighLevelOpcode hl_opcode = HighLevelOpcode(hl_ins->get_opcode());
    int hl_num_operand = hl_ins->get_num_operands();
    std::vector<Operand> modified_operands;
    std::tuple<int, int> op_sizes = get_dest_and_src_size(hl_opcode);
    for (int i = 0; i < hl_num_operand; i++)
    {
      int cur_size = (i == 0) ? std::get<0>(op_sizes) : std::get<1>(op_sizes);
      Operand cur_operand = hl_ins->get_operand(i);
      if ((cur_operand.get_kind() == Operand::VREG || cur_operand.get_kind() == Operand::VREG_MEM) &&
          (!cur_operand.has_comment()) && cur_operand.get_base_reg() >= 10)
      {
        if (pairings.find(cur_operand.get_base_reg()) != pairings.end())
        {
          cur_operand.set_comment(pairings[cur_operand.get_base_reg()]);
        }
        else
        {
          if ((int)available_mregs.size() > 0)
          {
            std::string m_reg = available_mregs.at(0);
            available_mregs.erase(available_mregs.begin());
            pairings.insert(std::make_pair(cur_operand.get_base_reg(), m_reg));
            cur_operand.set_comment(m_reg);
          }
          else
          {
            pairings.insert(std::make_pair(cur_operand.get_base_reg(), "stack_" + std::to_string(block_stack_idx)));
            cur_operand.set_comment("stack_" + std::to_string(block_stack_idx));
            block_stack_idx++;
          }
        }
      }
      else if (cur_operand.get_kind() == Operand::VREG && cur_operand.get_base_reg() >= 1 &&
               cur_operand.get_base_reg() <= 6)
      {
        std::string mreg = m_vreg_mreg_correspond.find(cur_operand.get_base_reg())->second;
        int key = -1;
        for (auto it : pairings)
        {
          if (it.second == mreg)
          {
            key = it.first;
            break;
          }
        }
        if (key != -1)
        {
          pairings.erase(key);
          if ((int)available_mregs.size() > 0)
          {
            std::string m_reg = available_mregs.at(0);
            available_mregs.erase(available_mregs.begin());
            pairings.insert(std::make_pair(cur_operand.get_base_reg(), m_reg));
            Operand src = Operand(cur_operand);
            cur_operand.set_comment(m_reg);
            Operand dest = Operand(cur_operand);
            HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, cur_size);
            return_hl_iseq->append(new Instruction(mov_opcode, dest, src));
          }
          else
          {
            pairings.insert(std::make_pair(cur_operand.get_base_reg(), "stack_" + std::to_string(block_stack_idx)));
            Operand src = Operand(cur_operand);
            cur_operand.set_comment("stack_" + std::to_string(block_stack_idx));
            block_stack_idx++;
            Operand dest = Operand(cur_operand);
            HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, cur_size);
            return_hl_iseq->append(new Instruction(mov_opcode, dest, src));
          }
        }
        else
        {
          int idx = -1;
          for (int i = 0; i < (int)available_mregs.size(); ++i)
          {
            if (available_mregs.at(i) == mreg)
            {
              idx = i;
              break;
            }
          }
          if (idx != -1)
          {
            available_mregs.erase(available_mregs.begin() + idx);
          }
        }
        restore_until_next_call.push_back(mreg);
      }
      modified_operands.push_back(cur_operand);
    }
    switch (hl_num_operand)
    {
    case 0:
      return_hl_iseq->append(new Instruction(hl_opcode));
      break;
    case 1:
    {
      Instruction *instr = new Instruction(hl_opcode, modified_operands.at(0));
      if (hl_ins->get_opcode() == HINS_call)
      {
        instr->set_symbol(hl_ins->get_symbol());
      }
      return_hl_iseq->append(instr);
      break;
    }
    case 2:
      return_hl_iseq->append(new Instruction(hl_opcode, modified_operands.at(0), modified_operands.at(1)));
      break;
    case 3:
      return_hl_iseq->append(new Instruction(hl_opcode, modified_operands.at(0), modified_operands.at(1),
                                             modified_operands.at(2)));
      break;
    default:
      assert(false);
    }

    if (hl_opcode == HINS_call)
    {
      for (std::string mreg_reserved : restore_until_next_call)
      {
        if (std::find(available_mregs.begin(), available_mregs.end(), mreg_reserved) == available_mregs.end())
        {
          available_mregs.push_back(mreg_reserved);
        }
      }
      restore_until_next_call.clear();
    }

    std::vector<int> mregs_to_delete;
    LiveVregsAnalysis::FactType instruction_after_liveness = m_live_vreg.get_fact_after_instruction(m_cur_block, hl_ins);
    for (auto it : pairings)
    {
      int vreg_num = it.first;
      std::string corr_mreg = it.second;
      if (!instruction_after_liveness.test(vreg_num))
      {
        available_mregs.push_back(corr_mreg);
        mregs_to_delete.push_back(vreg_num);
      }
    }
    for (int mreg : mregs_to_delete)
    {
      pairings.erase(mreg);
    }
    sort_available_mregs(available_mregs);
  }
  if (block_stack_idx > m_max_stack_idx)
  {
    m_max_stack_idx = block_stack_idx;
  }
  return return_hl_iseq;
}

void LocalMregAssignmentHighLevel::sort_available_mregs(std::vector<std::string> &available_mregs)
{
  std::vector<std::string> available_mregs_copy = available_mregs;
  available_mregs.clear();
  for (auto it = m_available_mregs.cbegin(); it != m_available_mregs.cend(); ++it)
  {
    if (std::find(available_mregs_copy.begin(), available_mregs_copy.end(), *it) != available_mregs_copy.end())
    {
      available_mregs.push_back(*it);
    }
  }
}

std::tuple<int, int> LocalMregAssignmentHighLevel::get_dest_and_src_size(HighLevelOpcode hl_opcode)
{
  if (hl_opcode > HINS_nop && hl_opcode <= HINS_restore_q)
  {
    int base_opcode = (((int)hl_opcode - 1) / 4) * 4 + 1;
    return operation_size_helper(hl_opcode, static_cast<HighLevelOpcode>(base_opcode));
  }
  if (hl_opcode >= HINS_sconv_bw && hl_opcode <= HINS_uconv_lq)
  {
    int src_size = -1;
    int dest_size = -1;
    switch (hl_opcode)
    {
    case HINS_sconv_bw:
    case HINS_uconv_bw:
      src_size = 1;
      dest_size = 2;
      break;
    case HINS_sconv_bl:
    case HINS_uconv_bl:
      src_size = 1;
      dest_size = 4;
      break;
    case HINS_sconv_bq:
    case HINS_uconv_bq:
      src_size = 1;
      dest_size = 8;
      break;
    case HINS_sconv_wl:
    case HINS_uconv_wl:
      src_size = 2;
      dest_size = 4;
      break;
    case HINS_sconv_wq:
    case HINS_uconv_wq:
      src_size = 2;
      dest_size = 8;
      break;
    case HINS_sconv_lq:
    case HINS_uconv_lq:
      src_size = 4;
      dest_size = 8;
      break;
    default:
      src_size = -1;
      dest_size = -1;
    }
    return std::make_tuple(dest_size, src_size);
  }
  if (hl_opcode == HINS_localaddr)
  {
    return std::make_tuple(8, 8);
  }
  return std::make_tuple(0, 0);
}

int LocalMregAssignmentHighLevel::get_src_size(int offset)
{
  switch (offset)
  {
  case 0:
    return 1;
  case 1:
    return 2;
  case 2:
    return 4;
  default:
    break;
  }
  return 8;
}

std::tuple<int, int> LocalMregAssignmentHighLevel::operation_size_helper(HighLevelOpcode cur_opcode,
                                                                         HighLevelOpcode base_opcode)
{
  int offset = cur_opcode - base_opcode;
  int size = get_src_size(offset);
  return std::make_tuple(size, size);
}

HighLevelOpcode LocalMregAssignmentHighLevel::get_opcode(HighLevelOpcode basecode, int size)
{
  switch (size)
  {
  case 2:
    return static_cast<HighLevelOpcode>(basecode + 1);
  case 4:
    return static_cast<HighLevelOpcode>(basecode + 2);
  case 8:
    return static_cast<HighLevelOpcode>(basecode + 3);
  default:
    break;
  }
  return basecode;
}