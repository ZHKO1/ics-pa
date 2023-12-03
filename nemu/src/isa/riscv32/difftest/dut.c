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

#include <isa.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"

static void isa_difftest_checkregs_log(const char *reg_name, word_t dut_value, word_t ref_value, bool *is_diff) {
  if(!*is_diff) {
    printf("Difftest:\n");
  }
  printf("%-15s" ANSI_FMT(FMT_REG_DISPLAY, ANSI_FG_RED) ANSI_FMT(FMT_REG_DISPLAY, ANSI_FG_GREEN) "\n", reg_name, dut_value, ref_value);
  *is_diff = true;
}


bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  size_t reg_len = MUXDEF(CONFIG_RVE, 16, 32);
  bool is_diff = false;
  for(size_t i = 0; i < reg_len; i++){
    word_t dut_reg_value = cpu.gpr[i];
    word_t ref_reg_value = ref_r->gpr[i];
    if(dut_reg_value != ref_reg_value){
      isa_difftest_checkregs_log(reg_name(i), dut_reg_value, ref_reg_value, &is_diff);
    }
  }
  vaddr_t dut_pc = cpu.pc;
  vaddr_t ref_pc = ref_r->pc;
  if(dut_pc != ref_pc){
    isa_difftest_checkregs_log("$pc", dut_pc, ref_pc, &is_diff);
  }

  return !is_diff;
}

void isa_difftest_attach() {
}
