#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

void print_a(char* a[], int len)
{
    // for (int i = 0; i < len; i++)
    //     fprintf(1, "%s ", a[i]);
    int i = 0;
    while (a[i] != 0)
        fprintf(1, "%s ", a[i++]);
    fprintf(1, "\n");
}

int xarg_line(char* nargv[], char buf[], int nargc)
{
    int i = 0;
    buf[0] = 0;
    while (read(0, &buf[i], 1) == 1 && buf[i++] != '\n' && i < 512)
        ;
    buf[i - 1] = 0;
    i = 0;

    int argi = 0;
    while (buf[i])
    {
        if (buf[i] == ' ')
        {
            buf[i] = 0;
            nargv[nargc++] = buf + argi;
            argi = i + 1;
        }
        i++;
    }
    if (i == 0)
        return nargc;
    nargv[nargc++] = buf + argi;
    return nargc;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(1, "not enough arguments\n");
        exit(0);
    }

    char buf[512];
    char* nargv[MAXARG];

    for (int i = 1; i < argc; i++)
        nargv[i - 1] = argv[i];
    
    int added_args_c = argc - 1;
    int nargc = added_args_c;
    while ((nargc = xarg_line(nargv, buf, added_args_c)) != added_args_c)
    {
        if (fork() == 0)
        {
            exec(argv[1], nargv);
            exit(0);
        }
        else
        {
            wait(0);
        }

        for (int i = added_args_c; i < MAXARG; i++)
            nargv[i] = 0;
    }

    exit(0);
}
