/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __RISCV_REG_H__
#define __RISCV_REG_H__

#include <common.h>

static inline int check_reg_idx(int idx) {
  IFDEF(CONFIG_RT_CHECK, assert(idx >= 0 && idx < MUXDEF(CONFIG_RVE, 16, 32)));
  return idx;
}

#define gpr(idx) (cpu.gpr[check_reg_idx(idx)])

static inline const char* reg_name(int idx) {
  extern const char* regs[];
  return regs[check_reg_idx(idx)];
}

#define CSR_SATP 0x180
#define CSR_MSTATUS 0x300
#define CSR_MTVEC 0x305
#define CSR_MSCRATCH 0x340
#define CSR_MEPC 0x341
#define CSR_MCAUSE 0x342

word_t get_csr(word_t key);
void set_csr(word_t key, word_t value);

#define get_mcause(inter, exccode) ((inter ? (~((word_t)(-1) >> 1)) : 0) | exccode)
#define inter_from_mcause(mcause) ((mcause & (~((word_t)(-1) >> 1))) ? 1 : 0)
#define exccode_from_mcause(mcause) ((mcause << 1) >> 1)

#define MCAUSE_CASE(name, inter, exccode) \
  enum { MCAUSE_##name = get_mcause(inter, exccode) }; 

MCAUSE_CASE(ENVIRONMENTCALL, 0, 11)
MCAUSE_CASE(MACHINETIMERINTR, 1, 7)

#endif
