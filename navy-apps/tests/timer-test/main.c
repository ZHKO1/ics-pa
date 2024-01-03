#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <NDL.h>

int main()
{
  uint32_t us = NDL_GetTicks();
  while (1)
  {
    uint32_t new_us = NDL_GetTicks();
    if (new_us - us >= 500000)
    {
      printf("sec = %ds\n", (int)new_us / 1000000);
      us = new_us;
    }
  }
  return 0;
}
