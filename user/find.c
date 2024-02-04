#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char* path, char* filename)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0){
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type){
    case T_DEVICE:
    case T_FILE:
        fprintf(1, "file instead of directory\n");
        break;

    case T_DIR:
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){
            printf("find: cannot stat %s\n", buf);
            continue;
        }
        if (st.type == T_DIR && strcmp(de.name, ".") && strcmp(de.name, ".."))
        {
            find(buf, filename);
            continue;
        }
        if (!strcmp(de.name, filename))
        {
            fprintf(1, "%s\n", buf);
            continue;
        }
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if (argc < 2)
  {
    fprintf(1, "not enough arguments\n");
    exit(0);
  }

  if(argc < 3){
    find(".", argv[1]);
    exit(0);
  }
  for(i=2; i<argc; i++)
    find(argv[1], argv[i]);
  exit(0);
}
