#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void concat(int p[][2], int i);

int
main(int argc, char* argv[])
{
    int pipes[35][2];
    int i;

    pipe(pipes[0]);

    for (i = 2; i <= 35; i++)
    {
        write(pipes[0][1], &i, sizeof(i));
    }
    
    if (fork() == 0)
    {
        concat(pipes, 0);
        exit(0);
    }
    else
    {
        close(pipes[0][1]);
        close(pipes[0][0]);
        wait(0);
    }

    exit(0);
}

void concat(int p[][2], int i)
{
    pipe(p[i+1]);
    close(p[i][1]);
    int pr; // read from left
    read(p[i][0], &pr, sizeof(pr));
    fprintf(1, "prime %d\n", pr);

    int rcvd = 0;
    int n;
    while (read(p[i][0], &n, sizeof(n)))
    {
        rcvd = 1;
        if (n % pr)
        {
            write(p[i+1][1], &n, sizeof(n));
        }
    }
    close(p[i][0]);

    if (!rcvd)
    {
        close(p[i+1][0]);
        return;
    }

    if (fork() == 0)
    {
        concat(p, i + 1);
        close(p[i+1][0]);
        exit(0);
    }
    else
    {
        close(p[i+1][0]);
        close(p[i+1][1]);
        wait(0);
    }
}
