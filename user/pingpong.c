#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int ptc[2];
    int ctp[2];

    char cr;
    char pr;

    pipe(ptc);
    pipe(ctp);

    if (fork() == 0)
    {
        close(ptc[1]);
        read(ptc[0], &cr, 1);
        fprintf(1, "%d: received ping\n", getpid());
        close(ptc[0]);

        close(ctp[0]);
        write(ctp[1], &cr, 1);
        close(ctp[1]);
        exit(0);
    }
    else
    {
        close(ptc[0]);
        write(ptc[1], "s", 1);
        close(ptc[1]);

        close(ctp[1]);
        read(ctp[0], &pr, 1);
        fprintf(1, "%d: received pong\n", getpid());
        close(ctp[0]);
        exit(0);
    }

  exit(0);
}