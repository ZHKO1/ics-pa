#include <amtest.h>
#include <limits.h>

#define MAXLINE 1024

int nums[] = {0, INT_MAX / 17, INT_MAX, INT_MIN, INT_MIN + 1,
              UINT_MAX / 17, INT_MAX / 17, UINT_MAX};

void test_sprintf()
{
  char str[MAXLINE] = "";

  memset(str, 0, MAXLINE);
  int done = sprintf(str, "");
  assert(done == 0);
  assert(strcmp(str, "") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, "波動拳");
  assert(done == 9);
  assert(strcmp(str, "波動拳") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, "昇龍拳 %d", 0);
  assert(done == 11);
  assert(strcmp(str, "昇龍拳 0") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, " %d 竜巻旋風脚", nums[1]);
  assert(done == 26);
  assert(strcmp(str, " 126322567 竜巻旋風脚") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, "迅%d雷脚", nums[2]);
  assert(done == 19);
  assert(strcmp(str, "迅2147483647雷脚") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, "風鎌%d蹴り", nums[3]);
  assert(done == 23);
  assert(strcmp(str, "風鎌-2147483648蹴り") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, "轟雷落%dとし", nums[4]);
  assert(done == 26);
  assert(strcmp(str, "轟雷落-2147483647とし") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, "閃火脚%d", nums[5]);
  assert(done == 18);
  assert(strcmp(str, "閃火脚252645135") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, "火砕蹴 %d", nums[6]);
  assert(done == 19);
  assert(strcmp(str, "火砕蹴 126322567") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, "龍尾脚 %d", nums[7]);
  assert(done == 12);
  assert(strcmp(str, "龍尾脚 -1") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, "神龍烈破->%d->%d->%d->%d->%d->%d->%d", nums[1], nums[2], nums[3], nums[4], nums[5], nums[6], nums[7]);
  assert(done == 87);
  assert(strcmp(str, "神龍烈破->126322567->2147483647->-2147483648->-2147483647->252645135->126322567->-1") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, "%s龍%s尾烈脚 %s", "", "", "");
  assert(done == 13);
  assert(strcmp(str, "龍尾烈脚 ") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, "疾風迅雷脚->%s", "神龍烈破");
  assert(done == 29);
  assert(strcmp(str, "疾風迅雷脚->神龍烈破") == 0);

  memset(str, 0, MAXLINE);
  done = sprintf(str, "%s地%s獄%s車%s", "奮迅脚", "急停止", "紫電カカト落とし", "踏み込み前蹴り");
  assert(done == 72);
  assert(strcmp(str, "奮迅脚地急停止獄紫電カカト落とし車踏み込み前蹴り") == 0);
}

void test_format()
{
  test_sprintf();
  printf("sprintf pass\n");
}
