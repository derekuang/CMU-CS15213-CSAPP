#include <stdio.h>
#include <math.h>

typedef unsigned float_bits;

float_bits float_half(float_bits f)
{
    size_t s = f & 0x80000000;
    size_t e = f & 0x7F800000;
    size_t m = f & 0x007FFFFF;

    // 规格化的值
    if (e != 0 && e != 0x7F800000) {
        if (e == 0x00800000) {
            size_t em = e | m;
            em = ((em & 2) == 2) ? em+1 >> 1 : em >> 1;
            return s | em;
        }
        e -= 0x00800000;
        return s | e | m;
    }
    // NaN
    else if(e == 0x7F800000 && m != 0) {
        return f;
    }
    // 非规格化的值
    else {
        m = ((m & 2) == 2) ? m+1 >> 1 : m >> 1;
        return s | e | m;
    }
}

typedef unsigned char* pointer;

void copy_bytes(pointer from, pointer to, size_t len)
{
    size_t i;
    for (i = len-1; i < len; i--)
        to[i] = from[i];
}

void show_bytes(pointer start, size_t len)
{
    size_t i;

    printf("0x");
    for (i = len-1; i < len; i--)
        printf("%.2X", start[i]);
    printf("\n");
}

void test_float_half(size_t from, size_t to)
{
    unsigned long long i;
    float x, y;
    float_bits f;

    printf("testing...\n");
    for (i = from; i <= to; i++) {
        f = i;

        copy_bytes((pointer)&f, (pointer)&x, sizeof(float_bits));
        x *= 0.5;

        f = float_half(f);
        copy_bytes((pointer)&f, (pointer)&y, sizeof(float_bits));

        if((isnan(x) && isnan(y)) || x == y)
            ;
        else {
            printf("ERROR!\ni = 0x%08X \nx = ", i), show_bytes((pointer)&x, sizeof(x));
            printf("y = "), show_bytes((pointer)&y, sizeof(y));
            return;
        }

        if (i > 0 && (i & 0xF0000000) != (i-1 & 0xF0000000)) {
            float per = (float)(i - from) / (to - from) * 100;
            printf("%5.2f%%\n", per);
        }
    }
    printf("  100%%\n");
    printf("CORRECT!\n");
}

int main()
{
    test_float_half(0x0FFFFFFF, 0xFFFFFFFF);
    // size_t test = 0x01000001;
    // test_float_half(test, test);

    return 0;
}