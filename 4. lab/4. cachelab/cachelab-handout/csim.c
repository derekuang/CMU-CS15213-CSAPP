/**
 * @author kuangyaode <kuangyaode@gmail.com>
 */

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cachelab.h"

typedef struct block{
    size_t tag;
    char valid;
} block, *set;

void updateSet(set pset, size_t tag, int E, char *line);
int getBlock(set pset, size_t tag, int E);
void hitBlock(set pset, int E, int index);
int missBlock(set pset, size_t tag, int E);
void sortSet(set pset, int E, int index);
void printHelp();

int hit_count = 0, miss_count = 0, eviction_count = 0;

int main(int argc, char** argv) {
    int opt;
    int v = 0, s = 0, E = 0, b = 0;
    char *t;

    // loop over arguments
    while (-1 != (opt = getopt(argc, argv, "s:E:b:t:hv"))) {
        switch (opt) {
            case 'h':
                printHelp();
                return 0;
            case 'v': // verbose mode
                v = 1;
                break;
            case 's': // number of set index bits(S = 2^s is the number of sets)
                s = atoi(optarg);
                break;
            case 'E': // associativity(number of lines per set)
                E = atoi(optarg);
                break;
            case 'b': // number of block bits(B = 2^b is the block size)
                b = atoi(optarg);
                break;
            case 't': // name of the valgrind trace to replay
                t = optarg;
                break;
            default:
                printf("wrong argument %c\n", opt);
                break;
        }
    }

    // Initialize the cache
    int i, setNum = pow(2, s);
    set *cache = malloc(setNum*sizeof(set));
    for (i = 0; i < setNum; i++) {
        cache[i] = calloc(E, sizeof(block));
    }

    FILE *fp = fopen(t, "r");
    char line[50], res[20];
    int op, size;
    size_t addr, tag;
    int iset;
    set pset;

    // loop over operations
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] == 'I') continue;

        op = line[1];
        addr = strtoll(strtok(&line[3], ","), NULL, 16);
        size = atoi(strtok(NULL, ","));
        res[0] = '\0';

        tag = addr >> (b + s);
        iset = (addr >> b) & ((1 << s) - 1);
        pset = cache[iset];
        switch (op) {
            case 'L': // load data
                updateSet(pset, tag, E, res);
                break;
            case 'S': // store data
                updateSet(pset, tag, E, res);
                break;
            case 'M': // modify data(load followed by store)
                updateSet(pset, tag, E, res);
                updateSet(pset, tag, E, res);
                break;
            default:
                break;
        }

        if (v) {
            printf("%c %lx,%d\tsindex: %d\tresult:%s\n",
                    op, addr, size, iset, res);
        }
    }

    printSummary(hit_count, miss_count, eviction_count);
    for (i = 0; i < setNum; i++) free(cache[i]);
    free(cache);
    fclose(fp);
    return 0;
}

/**
 * updateSet - Update the set by tag
 */
void updateSet(set pset, size_t tag, int E, char *res) {
    int index = getBlock(pset, tag, E);
    if (index >= 0) {
        hit_count++;
        strcat(res, " hit");
        hitBlock(pset, E, index);
    } else {
        miss_count++;
        strcat(res, " miss");
        if (missBlock(pset, tag, E)) {
            eviction_count++;
            strcat(res, " eviction");
        }
    }
}

/**
 * getBlock - Return index of the block with tag in the set, return -1 if the block is not exist
 */
int getBlock(set pset, size_t tag, int E) {
    int i = E-1;
    while (i >= 0) {
        if (!pset[i].valid) {
            return -1;
        } else if (pset[i].tag == tag) {
            return i;
        }
        i--;
    }
    return -1;
}

/**
 * hitBlock - The block with tag hit in the set
 */
void hitBlock(set pset, int E, int index) {
    sortSet(pset, E, index);
}

/**
 * missBlock - The block with tag miss in the set, return 1 if evict another block, otherwise return 0
 */
int missBlock(set pset, size_t tag, int E) {
    int index = E-1, evict = 0;
    block b = {tag, 1}; // new block
    if (pset[0].valid) {
        pset[E-1] = b; // evict the block by LRU
        evict = 1;
    } else {
        while (index >= 0) {
            if (!pset[index].valid) break;
            index--;
        }
        pset[index] = b;
    }

    sortSet(pset, E, index);
    return evict;
}

/**
 * sortSet - Sort the set by valid sign asec, count desc
 */
void sortSet(set pset, int E, int index) {
    int i = index - 1;
    block t;

    while (i >= 0 && pset[i].valid) {
        t = pset[i];
        pset[i] = pset[i+1];
        pset[i+1] = t;
        i--;
    }
}

/**
 * printHelp - Print the csim program help infomation
 */
void printHelp() {
    printf(
    "Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n"
    "Options:\n"
    "  -h         Print this help message.\n"
    "  -v         Optional verbose flag.\n"
    "  -s <num>   Number of set index bits.\n"
    "  -E <num>   Number of lines per set.\n"
    "  -b <num>   Number of block offset bits.\n"
    "  -t <file>  Trace file.\n"
    "\n"
    "Examples:\n"
    "  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n"
    "  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n"
    );
}