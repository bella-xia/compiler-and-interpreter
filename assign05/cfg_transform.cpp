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
  bool opt;
  bool has_jump = false;
  for (auto it = orig_bb->cbegin(); it != orig_bb->cend(); it++)
  {
    Instruction *hl_ins = *it;
    HighLevelOpcode hl_opcode = HighLevelOpcode(hl_ins->get_opcode());
    // // std::cout << hl_opcode << std::endl;
    opt = false;
    std::vector<Operand> delete_after_instruction;

    if (hl_ins->get_num_operands() > 1 && hl_opcode < HINS_cjmp_t)
    {
      // if the destination of the current instruction is contained in memory offset / vreg data structure,
      // then remove it
      Operand dest = hl_ins->get_operand(0);
      if (dest.get_kind() == Operand::VREG && m_vreg_memoffs.find(dest) != m_vreg_memoffs.end() &&
          hl_opcode != HINS_localaddr)
      {
        // std::cout << " m_vreg_memoffs erased vr" << dest.get_base_reg() << std::endl;
        m_vreg_memoffs.erase(dest);
      }
      if (dest.get_kind() == Operand::VREG)
      {
        // std::cout << "erasing vr " << dest.get_base_reg() << std::endl;
        erase_relevant_operations(dest, m_vreg_ops);
        delete_after_instruction = erase_relevant_values(hl_ins, dest, return_hl_iseq, m_vreg_vals);
      }
      // if a value is assigned to the dereferenced format,
      // then change any reference to its dereferenced value
      if (dest.get_kind() == Operand::VREG_MEM)
      {
        // erase_relevant_operations(dest, m_vreg_ops);
        Operand identifier = get_orig_operand(dest, m_vreg_vals, false);
        /*
        if (m_vreg_vals.find(Operand(Operand::VREG, identifier.get_base_reg())) != m_vreg_vals.end())
        {
          std::cout << "got here with (vr " << dest.get_base_reg() << ")" << std::endl;
          Operand target = std::get<0>(m_vreg_vals[Operand(Operand::VREG, identifier.get_base_reg())]);
          std::string object_str = (target.is_imm_ival()) ? std::to_string(target.get_imm_ival()) : "vr" + std::to_string(target.get_base_reg());
          std::cout << "vreg mem vr" << identifier.get_base_reg() << "gets " << object_str << std::endl;
          identifier = Operand(Operand::VREG_MEM, target.get_base_reg());
        }
        */
        erase_relevant_operations(identifier, m_vreg_ops);
      }
    }
    else if (hl_opcode == HINS_call)
    {
      Operand identifier = Operand(Operand::VREG, 0);
      erase_relevant_operations(identifier, m_vreg_ops);
      delete_after_instruction = erase_relevant_values(hl_ins, identifier, return_hl_iseq, m_vreg_vals);
    }
    // perform move instruction optimization
    if (hl_opcode >= HINS_mov_b && hl_opcode <= HINS_mov_q)
    {
      move_optimize(hl_ins, opt, m_vreg_vals);
    }
    // perform add / subtract / multiply / divide operation optimization
    else if (hl_opcode >= HINS_add_b && hl_opcode <= HINS_mod_q)
    {
      operation_optimize(hl_ins, return_hl_iseq, opt, m_vreg_vals, m_vreg_ops);
    }
    // perform compare instruction optimization
    else if (hl_opcode >= HINS_cmplt_b && hl_opcode <= HINS_cmpneq_q)
    {
      compare_optimize(hl_ins, return_hl_iseq, opt, m_vreg_vals, m_vreg_ops);
    }
    // perform upscale instruction optimization
    else if (hl_opcode >= HINS_sconv_bw && hl_opcode <= HINS_uconv_lq)
    {
      uscon_optimize(hl_ins, opt, m_vreg_vals);
    }
    // perform getting local address optimization
    else if (hl_opcode == HINS_localaddr)
    {
      localaddr_optimize(hl_ins, opt, m_vreg_memoffs, m_vreg_vals);
    }
    else if (hl_opcode >= HINS_neg_b && hl_opcode <= HINS_neg_q)
    {
      neg_optimize(hl_ins, return_hl_iseq, opt, m_vreg_vals, m_vreg_ops);
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
        /*
        if (dest.get_kind() == Operand::VREG_MEM && m_vreg_vals.find(Operand(Operand::VREG, dest.get_base_reg())) != m_vreg_vals.end())
        {
          dest = Operand(Operand::VREG_MEM, std::get<0>(m_vreg_vals[Operand(Operand::VREG, dest.get_base_reg())]).get_base_reg());
          m_is_modified = true;
          // std::cout << "optimization by removing uncessary virtual register for deference is still live" << std::endl;
        }
        */
        // iterate over all the remaining source operands
        for (int i = 1; i < (int)hl_ins->get_num_operands(); ++i)
        {
          // optimize any instance of copy propagation if the original operand is of kind
          // virtual register or virtual register dereferenced
          Operand orig_op = hl_ins->get_operand(i);
          new_ops.push_back(get_orig_operand(orig_op, m_vreg_vals, false));
          /*
          if (orig_op.get_kind() == Operand::VREG && m_vreg_vals.find(orig_op) != m_vreg_vals.end())
          {
            new_ops.push_back(std::get<0>(m_vreg_vals.find(orig_op)->second));
          }
          else if (orig_op.get_kind() == Operand::VREG_MEM && m_vreg_vals.find(Operand(Operand::VREG, orig_op.get_base_reg())) != m_vreg_vals.end())
          {
            new_ops.push_back(Operand(Operand::VREG_MEM, std::get<0>((m_vreg_vals[Operand(Operand::VREG, orig_op.get_base_reg())])).get_base_reg()));
          }
          else
          {
            // if no optimization can be performend, rertain
            // the original operand
            new_ops.push_back(orig_op);
          }
                  */
        }

        // check if there is redundant move
        // specifically, moving a virtual register to itself
        bool redundant = (hl_opcode >= HINS_mov_b && hl_opcode <= HINS_mov_q) &&
                         (dest.get_kind() == Operand::VREG && new_ops.at(0).get_kind() == Operand::VREG &&
                          dest.get_base_reg() == new_ops.at(0).get_base_reg());
        if (!redundant)
        {
          // otherwise, append the instruction to the optimized sequence of instructions
          return_hl_iseq->append(
              ((int)new_ops.size() == 1)
                  ? new Instruction(hl_ins->get_opcode(), dest, new_ops.at(0))
                  : new Instruction(hl_ins->get_opcode(), dest, new_ops.at(0), new_ops.at(1)));
        }
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
void LocalOptimizationHighLevel::localaddr_optimize(Instruction *hl_ins, bool &opt,
                                                    std::map<Operand, int> &m_vreg_memoffs,
                                                    std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals)
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
      opt = true;
      m_is_modified = true;
      // std::cout << "no need to move localaddr bc the current virtual register stores the offset needed is still alive" << std::endl;
    }
    // if some other register stores what is needed,
    // then only a move instruction will be performed
    else if (key.get_kind() != Operand::NONE)
    {
      // // std::cout << "refering to vr" << key << std::endl;
      if (m_vreg_vals.find(dest) != m_vreg_vals.end())
      {
        m_vreg_vals[dest] = std::make_tuple(key, 8);
      }
      else
      {
        m_vreg_vals.insert(std::pair(dest, std::make_tuple(key, 8)));
      }
      opt = true;
      m_is_modified = true;
      // std::cout << "no need to move localaddr bc another virtual register stores the offset needed is still alive" << std::endl;
    }
    else
    {
      // localaddr still needs to be performed, but retains the current virtual register information for
      // future reference
      if (m_vreg_vals.find(dest) != m_vreg_vals.end())
      {
        m_vreg_vals.erase(dest);
      }
      if (m_vreg_memoffs.find(dest) != m_vreg_memoffs.end())
      {
        m_vreg_memoffs[dest] = offset;
      }
      else
      {
        m_vreg_memoffs.insert(std::pair<Operand, int>(dest, offset));
      }
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
void LocalOptimizationHighLevel::compare_optimize(Instruction *hl_ins, std::shared_ptr<InstructionSequence> return_hl_iseq,
                                                  bool &opt, std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                                                  std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops)
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
      // std::cout << "move the result into a dereferenced regester instead of using conditional operation is still alive " << std::endl;
    }
    else if (dest.get_kind() == Operand::VREG)
    {
      // otherwise if it is a virtual register, can just store the value somewhere
      if (m_vreg_vals.find(dest) != m_vreg_vals.end())
      {
        m_vreg_vals[dest] = std::make_tuple(Operand(Operand::IMM_IVAL, result), 4);
      }
      else
      {
        m_vreg_vals.insert(std::pair(dest, std::make_tuple(Operand(Operand::IMM_IVAL, result), 4)));
      }
      opt = true;
      m_is_modified = true;
      // std::cout << "storing result into a virtual register mapping instead of using conditional operation is still alive " << std::endl;
    }
  }
  else if (dest.get_kind() == Operand::VREG &&
           (src1.get_kind() == Operand::IMM_IVAL || src1.get_kind() == Operand::VREG || src1.get_kind() == Operand::VREG_MEM) &&
           (src2.get_kind() == Operand::IMM_IVAL || src2.get_kind() == Operand::VREG || src2.get_kind() == Operand::VREG_MEM))
  {
    src1 = get_orig_operand(src1, m_vreg_vals, true);
    src2 = get_orig_operand(src2, m_vreg_vals, true);
    /*
    while (src1.get_kind() == Operand::VREG &&
           m_vreg_vals.find(src1) != m_vreg_vals.end())
    {
      src1 = std::get<0>(m_vreg_vals[src1]);
    }
    while (src1.get_kind() == Operand::VREG_MEM && m_vreg_vals.find(Operand(Operand::VREG, src1.get_base_reg())) != m_vreg_vals.end())
    {
      src1 = Operand(Operand::VREG_MEM, std::get<0>(m_vreg_vals[src1]).get_base_reg());
    }
    while ((src2.get_kind() == Operand::VREG || src2.get_kind() == Operand::VREG_MEM) &&
           m_vreg_vals.find(src2) != m_vreg_vals.end())
    {
      src2 = (src2.get_kind() == Operand::VREG)
                 ? std::get<0>(m_vreg_vals[src2])
                 : Operand(Operand::VREG_MEM, std::get<0>(m_vreg_vals[src2]).get_base_reg());
    }
    */
    Operand get_op = find_operation(static_cast<HighLevelOpcode>(hl_opcode), src1, src2, m_vreg_ops);
    if (get_op.get_kind() != Operand::NONE)
    {
      if (m_vreg_vals.find(dest) != m_vreg_vals.end())
      {
        m_vreg_vals[dest] = std::make_tuple(get_op, src_size);
      }
      else
      {
        m_vreg_vals.insert(std::pair(dest, std::make_tuple(get_op, src_size)));
      }
      opt = true;
      m_is_modified = true;
      // std::cout << "replace condition operation with variable still active" << std::endl;
    }
    else
    {
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
void LocalOptimizationHighLevel::uscon_optimize(Instruction *hl_ins, bool &opt,
                                                std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals)
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
    if (m_vreg_vals.find(dest) != m_vreg_vals.end())
    {
      m_vreg_vals[dest] = std::make_tuple(Operand(src), dest_size);
    }
    else
    {
      m_vreg_vals.insert(std::pair(dest, std::make_tuple(Operand(src), dest_size)));
    }
    opt = true;
    m_is_modified = true;
    // // // std::cout << "optimization by storing the value instead of explicitly upscaling it" << std::endl;
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
void LocalOptimizationHighLevel::operation_optimize(Instruction *hl_ins, std::shared_ptr<InstructionSequence> return_hl_iseq, bool &opt,
                                                    std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                                                    std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops)
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
    /*
    std::string src1_str = "";
    std::string src2_str = "";
    if (src1.get_kind() == Operand::VREG)
    {
      src1_str = "vr " + std::to_string(src1.get_base_reg());
    }
    else if (src1.get_kind() == Operand::VREG_MEM)
    {
      src1_str = "vr mem ref " + std::to_string(src1.get_base_reg());
    }
    else if (src1.get_kind() == Operand::IMM_IVAL)
    {
      src1_str = std::to_string(src1.get_imm_ival());
    }
    if (src2.get_kind() == Operand::VREG)
    {
      src2_str = "vr " + std::to_string(src2.get_base_reg());
    }
    else if (src2.get_kind() == Operand::VREG_MEM)
    {
      src2_str = "vr mem ref " + std::to_string(src2.get_base_reg());
    }
    else if (src2.get_kind() == Operand::IMM_IVAL)
    {
      src2_str = std::to_string(src2.get_imm_ival());
    }
    */
    Operand identifier = (src1.get_kind() == Operand::VREG) ? Operand(src1) : Operand(src2);
    identifier = get_orig_operand(identifier, m_vreg_vals, true);
    /*
    while (identifier.get_kind() == Operand::VREG &&
           m_vreg_vals.find(identifier) != m_vreg_vals.end())
    {
      identifier = std::get<0>(m_vreg_vals[identifier]);
    }
    */
    if (m_vreg_vals.find(dest) != m_vreg_vals.end())
    {
      m_vreg_vals[dest] = std::make_tuple(identifier, src_size);
    }
    else
    {
      m_vreg_vals.insert(std::pair(dest, std::make_tuple(identifier, src_size)));
    }
    opt = true;
    m_is_modified = true;
    // std::cout << "removing non-necessary operations still active" << std::endl;
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
      // std::cout << "add " << src1_val << " and " << src2_val << std::endl;
      break;
    case 1:
      // minus operation
      result = src1_val - src2_val;
      // Wstd::cout << "minus " << src1_val << " and " << src2_val << std::endl;
      break;
    case 2:
      // times operation
      result = src1_val * src2_val;
      // Wstd::cout << "multiply " << src1_val << " and " << src2_val << std::endl;
      break;
    case 3:
      // divide operation
      result = src1_val / src2_val;
      // std::cout << "divide " << src1_val << " and " << src2_val << std::endl;
      break;
    case 4:
      // mod operation
      result = src1_val % src2_val;
      // std::cout << "mod " << src1_val << " and " << src2_val << std::endl;
      break;
    default:
      break;
    }
    HighLevelOpcode new_opcode = static_cast<HighLevelOpcode>(HINS_mov_b + hl_opcode_offset);
    return_hl_iseq->append(new Instruction(new_opcode, dest, Operand(Operand::IMM_IVAL, result)));
    opt = true;
    m_is_modified = true;
    // std::cout << "converting constant calculations to mov instruction still active" << std::endl;
  }
  else if (dest.get_kind() == Operand::VREG &&
           (src1.get_kind() == Operand::VREG || src1.get_kind() == Operand::VREG_MEM || src1.get_kind() == Operand::IMM_IVAL) &&
           (src2.get_kind() == Operand::VREG || src2.get_kind() == Operand::VREG_MEM || src2.get_kind() == Operand::IMM_IVAL))
  {
    src1 = get_orig_operand(src1, m_vreg_vals, true);
    src2 = get_orig_operand(src2, m_vreg_vals, true);
    // case of operations that can be stored and referenced in the future
    /*
    while ((src1.get_kind() == Operand::VREG || src1.get_kind() == Operand::VREG_MEM) &&
           m_vreg_vals.find(src1) != m_vreg_vals.end())
    {
      src1 = (src1.get_kind() == Operand::VREG)
                 ? std::get<0>(m_vreg_vals[src1])
                 : Operand(Operand::VREG_MEM, std::get<0>(m_vreg_vals[src1]).get_base_reg());
    }
    while ((src2.get_kind() == Operand::VREG || src2.get_kind() == Operand::VREG_MEM) &&
           m_vreg_vals.find(src2) != m_vreg_vals.end())
    {
      src2 = (src2.get_kind() == Operand::VREG)
                 ? std::get<0>(m_vreg_vals[src2])
                 : Operand(Operand::VREG_MEM, std::get<0>(m_vreg_vals[src2]).get_base_reg());
    }
    */
    std::string src1_str = "";
    std::string src2_str = "";
    if (src1.get_kind() == Operand::VREG)
    {
      src1_str = "vr " + std::to_string(src1.get_base_reg());
    }
    else if (src1.get_kind() == Operand::VREG_MEM)
    {
      src1_str = "vr mem ref " + std::to_string(src1.get_base_reg());
    }
    else if (src1.get_kind() == Operand::IMM_IVAL)
    {
      src1_str = std::to_string(src1.get_imm_ival());
    }
    if (src2.get_kind() == Operand::VREG)
    {
      src2_str = "vr " + std::to_string(src2.get_base_reg());
    }
    else if (src2.get_kind() == Operand::VREG_MEM)
    {
      src2_str = "vr mem ref " + std::to_string(src2.get_base_reg());
    }
    else if (src2.get_kind() == Operand::IMM_IVAL)
    {
      src2_str = std::to_string(src2.get_imm_ival());
    }
    Operand get_op = find_operation(static_cast<HighLevelOpcode>(hl_opcode), src1, src2, m_vreg_ops);
    if (get_op.get_kind() != Operand::NONE)
    {
      if (m_vreg_vals.find(dest) != m_vreg_vals.end())
      {
        m_vreg_vals[dest] = std::make_tuple(get_op, src_size);
      }
      else
      {
        m_vreg_vals.insert(std::pair(dest, std::make_tuple(get_op, src_size)));
      }
      opt = true;
      m_is_modified = true;
      // std::cout << "replace operation with variable still active" << std::endl;
    }
    else
    {
      if (src1.get_kind() == Operand::NONE || src2.get_kind() == Operand::NONE)
      {
        // std::cout << hl_opcode << ", " << src1_str << ", " << src2_str << std::endl;
      }
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
void LocalOptimizationHighLevel::move_optimize(Instruction *hl_ins, bool &opt,
                                               std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals)
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
    // std::cout << "got here with vr" << dest.get_base_reg() << std::endl;
    if (dest.get_base_reg() >= 10 &&
        (src.get_kind() == Operand::IMM_IVAL || src.get_kind() == Operand::VREG))
    {
      // std::cout << "got here with vr" << dest.get_base_reg() << std::endl;
      Operand identifier = Operand(src);
      identifier = get_orig_operand(identifier, m_vreg_vals, true);
      /*
      while (identifier.get_kind() == Operand::VREG &&
             m_vreg_vals.find(identifier) != m_vreg_vals.end())
      {
        identifier = std::get<0>(m_vreg_vals[identifier]);
      }
      */
      if (m_vreg_vals.find(dest) != m_vreg_vals.end())
      {
        m_vreg_vals[dest] = std::make_tuple(identifier, src_size);
      }
      else
      {
        m_vreg_vals.insert(std::pair(dest, std::make_tuple(identifier, src_size)));
      }
      opt = true;
    }
  }
}

void LocalOptimizationHighLevel::neg_optimize(Instruction *hl_ins, std::shared_ptr<InstructionSequence> return_hl_iseq,
                                              bool &opt, std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals,
                                              std::map<std::tuple<HighLevelOpcode, Operand, Operand>, Operand> &m_vreg_ops)
{
  HighLevelOpcode hl_opcode = static_cast<HighLevelOpcode>(hl_ins->get_opcode());
  Operand dest = hl_ins->get_operand(0);
  Operand src = hl_ins->get_operand(1);
  int src_size = get_src_size(hl_opcode - HINS_neg_b);
  src = get_orig_operand(src, m_vreg_vals, true);
  /*
  while (src.get_kind() == Operand::VREG && m_vreg_vals.find(src) != m_vreg_vals.end())
  {
    src = std::get<0>(m_vreg_vals[src]);
  }
  */
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
    if (similar_op.get_kind() == Operand::NONE)
    {
      m_vreg_ops.insert(std::pair(std::make_tuple(hl_opcode, src, Operand(Operand::LABEL, "0")), dest));
    }
    else
    {
      HighLevelOpcode move_opcode = static_cast<HighLevelOpcode>(HINS_mov_b + hl_opcode - HINS_neg_b);
      return_hl_iseq->append(new Instruction(move_opcode, dest, similar_op));
      opt = true;
      m_is_modified = true;
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
    if (std::get<1>(key).get_kind() == Operand::NONE || std::get<2>(key).get_kind() == Operand::NONE)
    {
      // std::cout << std::get<0>(key) << std::endl;
    }
    m_vreg_ops.erase(key);
  }
}

std::vector<Operand> LocalOptimizationHighLevel::erase_relevant_values(Instruction *hl_ins, Operand dest,
                                                                       std::shared_ptr<InstructionSequence> return_hl_iseq,
                                                                       std::map<Operand, std::tuple<Operand, int>> &m_vreg_vals)
{

  if (dest.get_kind() == Operand::VREG)
  {
    // std::cout << "deleting relevant operations to vreg" << dest.get_base_reg() << std::endl;
  }
  else if (dest.get_kind() == Operand::VREG_MEM)
  {
    // std::cout << "deleting relevant operations to vreg mem ref" << dest.get_base_reg() << std::endl;
  }

  std::vector<Operand> items_to_delete_after_instruction;
  std::vector<Operand> items_to_delete;
  LiveVregsAnalysis::FactType instruction_before_liveness = m_live_vreg.get_fact_before_instruction(m_cur_block, hl_ins);
  LiveVregsAnalysis::FactType instruction_after_liveness = m_live_vreg.get_fact_after_instruction(m_cur_block, hl_ins);
  for (std::pair<Operand, std::tuple<Operand, int>> ele : m_vreg_vals)
  {
    Operand map_dest = ele.first;
    Operand map_src = std::get<0>(ele.second);
    int src_size = std::get<1>(ele.second);
    if ((map_dest.get_kind() != Operand::NONE && map_dest == dest) ||
        (map_src.get_kind() != Operand::NONE && map_src == dest))
    {
      if ((map_src.get_kind() != Operand::NONE && map_src == dest) && !instruction_after_liveness.test(ele.first.get_base_reg()) &&
          instruction_before_liveness.test(ele.first.get_base_reg()))
      {
        items_to_delete_after_instruction.push_back(ele.first);
      }
      else
      {
        items_to_delete.push_back(ele.first);
        if ((map_src.get_kind() != Operand::NONE && map_src == dest) &&
            instruction_after_liveness.test(ele.first.get_base_reg()))
        {
          // // std::cout << "still using vr" << ele.first << " after this instruction" << std::endl;
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
    // std::cout << "checking ending vreg " << dest_vreg << std::endl;
    if (end_of_block_facts.test(dest_vreg.get_base_reg()))
    {
      Operand src = std::get<0>(ele.second);
      int src_size = std::get<1>(ele.second);
      // // std::cout << "checking needed " << dest_vreg << " of size " << src_size << std::endl;
      HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, src_size);
      return_hl_iseq->append(new Instruction(mov_opcode, dest_vreg, src));
    }
    else
    {
      // std::cout << "checking not needed " << dest_vreg << std::endl;
    }
  }
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