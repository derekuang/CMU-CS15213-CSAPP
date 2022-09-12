#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

sigjmp_buf buf;

char *tfgets(char *str, int n, FILE *stream);

int main()
{
    char s[100];

    if (tfgets(s, 100, stdin)) {
        printf("return val %s\n", s);
    } else {
        printf("return val NULL\n");
    }

    return 0;
}

void sigalrm_handler(int signum) {
    siglongjmp(buf, 1);
}

char *tfgets(char *str, int n, FILE *stream)
{
    if (!sigsetjmp(buf, 1)) {
        signal(SIGALRM, sigalrm_handler);
        alarm(5);
        fgets(str, n, stream);
    } else {
        str = NULL;
        return NULL;
    }
}