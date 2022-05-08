#include <stdio.h>
#include <math.h>

typedef unsigned float_bits;

float_bits float_half(float_bits f)
{
    size_t s = (f >> 31) & 1;
    size_t e = (f >> 23) & 0xFF;
    size_t m = f & 0x7FFFFF;

    printf("%.2X %.2X %.2X", s, e, m);

    return f;
}

int main()
{
    float_bits neg_inf = 0xFF800000;

    float_half(neg_inf);

    return 0;
}