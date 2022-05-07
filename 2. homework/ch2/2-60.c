#include <stdio.h>

unsigned replace_byte(unsigned x, int i, unsigned char b)
{
    unsigned char* p = (unsigned char*)&x;
    p += i;
    *p = b;

    return x;
}

int main()
{
    printf("0x%X\n", replace_byte(0x12345678, 2, 0xAB));
    printf("0x%X\n", replace_byte(0x12345678, 0, 0xAB));

    return 0;
}