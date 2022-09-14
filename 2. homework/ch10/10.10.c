#include "csapp.h"

int main(int argc, char **argv)
{
    int n;
    rio_t rio;
    char buf[MAXLINE];

    char *infile;
    int fd;
    if (argc > 1) {
        infile = argv[1];
        if ((fd = Open(infile, O_RDONLY, 0777)) > 0) {
            Dup2(fd, STDIN_FILENO);
        }
    }

    Rio_readinitb(&rio, STDIN_FILENO);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
        Rio_writen(STDOUT_FILENO, buf, n);
}