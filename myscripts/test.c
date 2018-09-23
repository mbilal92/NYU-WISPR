#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>

int main(int argc, char *argv[])
{

  int b = 0x0111;
  int mask = 0x1100;
  int mask2 = 0x0001;

  printf("%d %d %d %d %d\n",b, (b & mask), (b & mask2), (b & mask) >> 4, (b & mask2) );


  
  return 0;
} 