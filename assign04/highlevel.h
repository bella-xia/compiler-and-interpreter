#ifndef HIGHLEVEL_H
#define HIGHLEVEL_H

#include "formatter.h"

// Generated high-level opcodes and support functions/classes
// Do not edit this file!

enum HighLevelOpcode {
  HINS_nop,
  HINS_add_b,
  HINS_add_w,
  HINS_add_l,
  HINS_add_q,
  HINS_sub_b,
  HINS_sub_w,
  HINS_sub_l,
  HINS_sub_q,
  HINS_mul_b,
  HINS_mul_w,
  HINS_mul_l,
  HINS_mul_q,
  HINS_div_b,
  HINS_div_w,
  HINS_div_l,
  HINS_div_q,
  HINS_mod_b,
  HINS_mod_w,
  HINS_mod_l,
  HINS_mod_q,
  HINS_lshift_b,
  HINS_lshift_w,
  HINS_lshift_l,
  HINS_lshift_q,
  HINS_rshift_b,
  HINS_rshift_w,
  HINS_rshift_l,
  HINS_rshift_q,
  HINS_cmplt_b,
  HINS_cmplt_w,
  HINS_cmplt_l,
  HINS_cmplt_q,
  HINS_cmplte_b,
  HINS_cmplte_w,
  HINS_cmplte_l,
  HINS_cmplte_q,
  HINS_cmpgt_b,
  HINS_cmpgt_w,
  HINS_cmpgt_l,
  HINS_cmpgt_q,
  HINS_cmpgte_b,
  HINS_cmpgte_w,
  HINS_cmpgte_l,
  HINS_cmpgte_q,
  HINS_cmpeq_b,
  HINS_cmpeq_w,
  HINS_cmpeq_l,
  HINS_cmpeq_q,
  HINS_cmpneq_b,
  HINS_cmpneq_w,
  HINS_cmpneq_l,
  HINS_cmpneq_q,
  HINS_and_b,
  HINS_and_w,
  HINS_and_l,
  HINS_and_q,
  HINS_or_b,
  HINS_or_w,
  HINS_or_l,
  HINS_or_q,
  HINS_xor_b,
  HINS_xor_w,
  HINS_xor_l,
  HINS_xor_q,
  HINS_neg_b,
  HINS_neg_w,
  HINS_neg_l,
  HINS_neg_q,
  HINS_not_b,
  HINS_not_w,
  HINS_not_l,
  HINS_not_q,
  HINS_compl_b,
  HINS_compl_w,
  HINS_compl_l,
  HINS_compl_q,
  HINS_inc_b,
  HINS_inc_w,
  HINS_inc_l,
  HINS_inc_q,
  HINS_dec_b,
  HINS_dec_w,
  HINS_dec_l,
  HINS_dec_q,
  HINS_mov_b,
  HINS_mov_w,
  HINS_mov_l,
  HINS_mov_q,
  HINS_spill_b,
  HINS_spill_w,
  HINS_spill_l,
  HINS_spill_q,
  HINS_restore_b,
  HINS_restore_w,
  HINS_restore_l,
  HINS_restore_q,
  HINS_sconv_bw,
  HINS_sconv_bl,
  HINS_sconv_bq,
  HINS_sconv_wl,
  HINS_sconv_wq,
  HINS_sconv_lq,
  HINS_uconv_bw,
  HINS_uconv_bl,
  HINS_uconv_bq,
  HINS_uconv_wl,
  HINS_uconv_wq,
  HINS_uconv_lq,
  HINS_ret,
  HINS_jmp,
  HINS_call,
  HINS_enter,
  HINS_leave,
  HINS_localaddr,
  HINS_cjmp_t,
  HINS_cjmp_f,
}; // HighLevelOpcode enumeration

// Translate a high-level opcode to its assembler mnemonic.
// Returns nullptr if the opcode is unknown.
const char *highlevel_opcode_to_str(HighLevelOpcode opcode);

// Determine the source operand size (int bytes) implied by a specified
// opcode. If the opcode doesn't have a source operand conveying data,
// 0 is returned.
int highlevel_opcode_get_source_operand_size(HighLevelOpcode opcode);

// Determine the destination operand size (int bytes) implied by a specified
// opcode. If the opcode doesn't have a destination operand,
// 0 is returned.
int highlevel_opcode_get_dest_operand_size(HighLevelOpcode opcode);

#endif // HIGHLEVEL_H
