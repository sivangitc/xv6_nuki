#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc <= 1){
    fprintf(2, "sleep: not enough arguments\n");
    exit(1);
  }

    int n = atoi(argv[1]);
    fprintf(1, "sleeping for %d ticks\n", n);

    if (sleep(n) != 0)
        fprintf(2, "error when sleeping\n");

  exit(0);
}