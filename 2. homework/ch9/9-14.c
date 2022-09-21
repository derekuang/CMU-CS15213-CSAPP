#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>

int main()
{
    int fd;
    char *bufp; /* ptr to memory-mapped VM area */

    fd = open("hello.txt", O_RDWR, 0);
    bufp = mmap(NULL, 1, PROT_WRITE, MAP_SHARED, fd, 0);
    bufp[0] = 'J';

    return 0;
}