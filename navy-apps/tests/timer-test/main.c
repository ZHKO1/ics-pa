#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <assert.h>

int main()
{
  struct timeval tv = {};
  struct timezone tz = {};
  int result = gettimeofday(&tv, &tz);
  uint64_t us = tv.tv_sec * 1000000 + tv.tv_usec;
  while (1)
  {
    result = gettimeofday(&tv, &tz);    
    assert(result == 0);
    uint64_t new_us = tv.tv_sec * 1000000 + tv.tv_usec;
    if(new_us - us >= 500000) {
      printf("sec = %ds\n", (int)tv.tv_sec);
      us = new_us;
    }
  }
  return 0;
}
