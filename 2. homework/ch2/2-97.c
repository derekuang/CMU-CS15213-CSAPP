#include <stdio.h>
#include <limits.h>

typedef unsigned float_bits;

/* Compute (float) i */
float_bits float_i2f(int i)
{
    if (i == 0) {
        return 0;
    }

    size_t s, e, m, b;

    s = (i >> 31) & 1;
    i = (s == 0) ? i : (~i)+1; // 转换成非负数操作

    e = m = 0;
    while (i != 0) {
        b = i & 1; // 取lsb(最低位)
        m = (b << 31) | (m >> 1); // 放在msb(最高位)
        e++;
        i = (unsigned)i >> 1; // 确保最高位用0补充
    }

    // normalize m
    m <<= 1;
    e--;

    // round
    size_t lsb = m & 0x200;
    size_t t = m & 0x1FF;
    if (t > 0x100 || (t == 0x100 && lsb == 0x200)) {
        m += 0x200;
        e = (m >> 9 == 0) ? e+1 : e;
    }

    s <<= 31;
    e = (e+127) << 23; // 加上bias
    m >>= 9; // 舍掉
    return s | e | m;
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

void test_float_i2f(int from, int to)
{
    long long n;
    float f1, f2;
    float_bits f;

    printf("testing...\n");
    for (n = from; n <= to; n++) {
        f1 = (float)(int)n;
        f = float_i2f(n);
        copy_bytes((pointer)&f, (pointer)&f2, sizeof(float_bits));

        if (f1 == f2)
            continue;
        else {
            printf("ERROR!\nn = 0x%08X \nf1 = ", (int)n), show_bytes((pointer)&f1, sizeof(f1));
            printf("f2 = "), show_bytes((pointer)&f2, sizeof(f2));
            return;
        }
    }
    printf("CORRECT!\n");
}

int main()
{
    test_float_i2f(INT_MIN, INT_MAX);
    // test_float_i2f(0x80000021, 0x80000021);

    return 0;
}