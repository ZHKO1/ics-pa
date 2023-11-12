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

#include <common.h>

extern uint64_t g_nr_guest_inst;

static struct IringBufs iringbufs = {.start = 0, .end = 0, .str = {}};

#ifndef CONFIG_TARGET_AM
FILE *log_fp = NULL;

void init_log(const char *log_file) {
  log_fp = stdout;
  if (log_file != NULL) {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    log_fp = fp;
  }
  Log("Log is written to %s", log_file ? log_file : "stdout");
}

bool log_enable() {
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) &&
         (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}
#endif

void update_iringbufs(char *str) {
  iringbufs.end = (iringbufs.end + 1) % IringBufNum;
  char *buf = iringbufs.str[iringbufs.end];
  memset(buf, 0, IringBufLen);
  strncpy(buf, str, IringBufLen);
  buf[IringBufLen - 1] = '\0';
  if (iringbufs.end == iringbufs.start) {
    iringbufs.start = (iringbufs.start + 1) % IringBufNum;
  }
}

void printf_iringbufs() {
  printf("last %d instructions:\n", IringBufNum);
  size_t i = iringbufs.start;
  while (i != iringbufs.end) {
    printf("     %s\n", iringbufs.str[i]);
    i = (i + 1) % IringBufNum;
  }
  printf("---> %s\n", iringbufs.str[i]);
}
