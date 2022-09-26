/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * v1.0: In this approach, the allocator based on implicit free lists,
 *       first-fit placement, and boundary tag coalescing.
 *
 * v1.1: Compatible with next-fit placement.
 *
 * v1.2: Remove footer from allocated block to improve space utilization.
 *
 * v1.3: Compatible with best-fit placement.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "kuang",
    /* First member's full name */
    "Yaode Kuang",
    /* First member's email address */
    "kuangyaode@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/*
 * If NEXT_FIT defined, use next fit search, else use first-fit search
 */
#define NEXT_FIT

/*
 * If NEXT_FIT not defined and BEST_FIT defined, use best fit search,
 * if BEST_FIT not defined either, use first-fit search
 */
#ifndef NEXT_FIT
#define BEST_FIT
#endif

/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */
#define DSIZE       8       /* Double word size (bytes) */
#define ALIGNMENT   DSIZE   /* Single word (4) or double word (8) alignment */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (4KB) */
#define MINBLOCK    2*DSIZE /* Minimum size of a block */

/* Block alloc state */
#define BLK_FREE    0b00    /* The block is free */
#define BLK_ALLOC   0b01    /* The block is allocated */
#define BLK_PALLOC  0b10    /* The previous block is allocated(only stored in header) */

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)   (GET(p) & ~0b111)
#define GET_ALLOC(p)  (GET(p) & BLK_ALLOC)
#define GET_PALLOC(p) (GET(p) & BLK_PALLOC)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) /* Only works for the block is free */

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) /* Only works for the previous block is free */
/* $end mallocmacros */

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */
#ifdef NEXT_FIT
static char *rover;           /* Next fit rover */
#endif

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);

/*
 * mm_init - Initialize the memory manager.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                                             /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, BLK_ALLOC));            /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, BLK_ALLOC));            /* Prologue footer(unnecessary) */
    PUT(heap_listp + (3*WSIZE), PACK(0, (BLK_ALLOC | BLK_PALLOC))); /* Epilogue header */
    heap_listp += (2*WSIZE);                                        /* Payload(0) start of prologue */

#ifdef NEXT_FIT
    rover = heap_listp;
#endif

    /* Extend the empty heap with a free block of CHUNKSIZE */
    if (extend_heap(CHUNKSIZE) == NULL)
        return -1;
    return 0;
}

/*
 * mm_malloc - Allocate a block with at least size bytes of payload.
 */
void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;

    if (!heap_listp)
        mm_init();

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs */
    if (size <= 3*WSIZE) {
        asize = MINBLOCK;
    }
    else {
        asize = ALIGN(size+WSIZE);
    }

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Free a block.
 */
void mm_free(void *bp)
{
    if (!bp) {
        return;
    }

    if (!heap_listp) {
        mm_init();
    }

    size_t size, palloc, nsize, nalloc;
    char *np;

    /* Update alloc state of the block */
    size = GET_SIZE(HDRP(bp));
    palloc = GET_PALLOC(HDRP(bp));
    PUT(HDRP(bp), PACK(size, (BLK_FREE | palloc)));
    PUT(FTRP(bp), PACK(size, BLK_FREE));

    /* Update alloc state of the next block */
    np = NEXT_BLKP(bp);
    nsize = GET_SIZE(HDRP(np));
    nalloc = GET_ALLOC(HDRP(np));
    PUT(HDRP(np), PACK(nsize, nalloc));

    coalesce(bp);
}

/*
 * mm_realloc - Naive implementation of realloc.
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void *newptr;

    /* If size == 0, free the block and return NULL */
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    /* If ptr is NULL, malloc a block */
    if (ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);
    if (!newptr) {
        return NULL;
    }

    /* Copy the old data */
    oldsize = MIN(GET_SIZE(HDRP(ptr)), size);
    memcpy(newptr, ptr, oldsize);

    /* Free the old block */
    mm_free(ptr);

    return newptr;
}

/*
 * Internal helper routines
 */

/*
 * extend_heap - Extend heap with free block and return its block pointer.
 */
static void *extend_heap(size_t bytes)
{
    char *bp;
    size_t size;
    size_t palloc;

    /* Allocate an multiple of ALIGNMENT number bytes to maintain alignment */
    size = ALIGN(bytes);
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    palloc = GET_PALLOC(HDRP(bp));
    PUT(HDRP(bp), PACK(size, (BLK_FREE | palloc)));
    PUT(FTRP(bp), PACK(size, BLK_FREE));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, BLK_ALLOC));

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least MINBLOCK.
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= MINBLOCK) {
        PUT(HDRP(bp), PACK(asize, (BLK_ALLOC | BLK_PALLOC)));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK((csize-asize), (BLK_FREE | BLK_PALLOC)));
        PUT(FTRP(bp), PACK((csize-asize), BLK_FREE));
    }
    else {
        PUT(HDRP(bp), PACK(csize, (BLK_ALLOC | BLK_PALLOC)));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(GET_SIZE(HDRP(bp)), (BLK_ALLOC | BLK_PALLOC)));
    }
}

/*
 * find_fit - Find a fit for a block with asize bytes.
 */
static void *find_fit(size_t asize)
{
#ifdef NEXT_FIT
    /* Next fit search */
    void *oldrover = rover;

    /* Search from the rover to the end of list */
    for( ; GET_SIZE(HDRP(rover)) > 0; rover = NEXT_BLKP(rover)) {
        if (!GET_ALLOC(HDRP(rover)) && (GET_SIZE(HDRP(rover)) >= asize)) {
            return rover;
        }
    }

    /* Search from start of list to old rover */
    for (rover = heap_listp; rover < oldrover; rover = NEXT_BLKP(rover)) {
        if (!GET_ALLOC(HDRP(rover)) && (GET_SIZE(HDRP(rover)) >= asize)) {
            return rover;
        }
    }

#elif defined(BEST_FIT)
    /* Best fit search */
    void *bp1, *bp2 = NULL;
    size_t size;

    for (bp1 = heap_listp; GET_SIZE(HDRP(bp1)) > 0; bp1 = NEXT_BLKP(bp1)) {
        if (!GET_ALLOC(HDRP(bp1)) && (GET_SIZE(HDRP(bp1)) >= asize) &&
           (!bp2 || (GET_SIZE(HDRP(bp1)) < GET_SIZE(HDRP(bp2))))) {
            bp2 = bp1;
        }
    }
    return bp2;

#else
    /* First fit search */
    void *bp;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (GET_SIZE(HDRP(bp)) >= asize)) {
            return bp;
        }
    }
#endif

    return NULL; /* No fit */
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block.
 */
static void *coalesce(void *bp)
{
    size_t palloc = GET_PALLOC(HDRP(bp));
    size_t nalloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /* Case 1 */
    if (palloc && nalloc) {
        return bp;
    }

    /* Case 2 */
    else if (palloc && !nalloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, (BLK_FREE | palloc)));
        PUT(FTRP(bp), PACK(size, BLK_FREE));
    }

    /* Case 3 */
    else if (!palloc && nalloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, (BLK_FREE | BLK_PALLOC)));
        PUT(FTRP(bp), PACK(size, BLK_FREE));
        bp = PREV_BLKP(bp);
    }

    /* Case 4 */
    else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
            GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, (BLK_FREE | BLK_PALLOC)));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, BLK_FREE));
        bp = PREV_BLKP(bp);
    }

#ifdef NEXT_FIT
    /* Make sure the rove isn't pointing into the free block */
    /* that we just coalesced */
    if ((rover > (char *)bp) && (rover < NEXT_BLKP(bp))) {
        rover = bp;
    }
#endif

    return bp;
}