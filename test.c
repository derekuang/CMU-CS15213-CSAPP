#include <stdio.h>
#include <math.h>
#include <limits.h>

typedef unsigned char* pointer;

void show_bytes(pointer start, size_t len)
{
    size_t i;

    printf("0x");
    for (i = len-1; i < len; i--)
        printf("%.2X", start[i]);
    printf("\n");
}

void show_float(size_t* f)
{
    size_t x = *f;
    size_t s = (x >> 31) & 1;
    size_t e = (x >> 23) & 0xFF;
    size_t m = x & 0x7FFFFF;

    printf("S = 0x%X E = 0x%X M = 0x%X\n", s, e, m);
}

int main() {
    int x = 0xFFFFFFFF;
    float f = (float)x;

    // show_bytes((pointer)&f, sizeof(f));
    show_float((size_t*)&f);

    return 0;
}
