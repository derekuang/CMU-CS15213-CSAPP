/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 *
 * @author kuangyaode
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_submit_32(int M, int N, int A[N][M], int B[M][N]);
void transpose_submit_64(int M, int N, int A[N][M], int B[M][N]);
void transpose_submit_61(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    switch (M)
    {
        case 32: // 32*32
            transpose_submit_32(M, N, A, B);
            break;
        case 64: // 64*64
            transpose_submit_64(M, N, A, B);
            break;
        case 61: // 61*67
            transpose_submit_61(M, N, A, B);
            break;
        default:
            break;
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/**
 * transpose_submit_32 - A transpose function for specific matrix 32*32
 */
void transpose_submit_32(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    int n, m;

    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            for (n = i; n < i + 8; n++) {
                for (m = j; m < j + 8; m++) {
                    B[m][n] = A[n][m];
                }
            }
        }
    }
}

/**
 * transpose_submit_64 - A transpose function for specific matrix 64*64
 */
void transpose_submit_64(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    int n, m;
    // int t1, t2, t3, t4, t5, t6, t7, t8;

    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            // top-left 4*4 sub block of the 8*8 block
            for (n = i; n < i + 4; n++) {
                for (m = j; m < j + 4; m++) {
                    B[m][n] = A[n][m];
                }
            }

            // top-right 4*4 sub block of the 8*8 block
            for (n = i + 4; n < i + 8; n++) {
                for (m = j; m < j + 4; m++) {
                    B[m][n] = A[n][m];
                }
            }

            // bottom-right 4*4 sub block of the 8*8 block
            for (n = i + 4; n < i + 8; n++) {
                for (m = j + 4; m < j + 8; m++) {
                    B[m][n] = A[n][m];
                }
            }

            // bottom-left 4*4 sub block of the 8*8 block
            for (n = i; n < i + 4; n++) {
                for (m = j + 4; m < j + 8; m++) {
                    B[m][n] = A[n][m];
                }
            }
        }
    }
}
/**
 * transpose_submit_61 - A transpose function for specific matrix 61*67
 */
void transpose_submit_61(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    int n, m;

    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            for (n = i; n < i + 8 && n < N; n++) {
                for (m = j; m < j + 8 && m < M; m++) {
                    B[m][n] = A[n][m];
                }
            }
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
