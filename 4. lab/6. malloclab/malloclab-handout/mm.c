/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * v0.1: In this naive approach, a block is allocated by simply incrementing
 *       the brk pointer.  A block is pure payload. There are no headers or
 *       footers.  Blocks are never coalesced or reused. Realloc is
 *       implemented directly using mm_malloc and mm_free.
 *
 * v1.0: In this approach, the allocator based on implicit free lists,
 *       first-fit placement, and boundary tag coalescing.
 *
 * v1.1: Compatible with next-fit placement.
 *
 * v1.2: Remove footer from allocated block to improve space utilization.
 *
 * v1.3: Compatible with best-fit placement.
 *
 * v2.0: In this approach, the allocator based on explicit free lists,
 *       first-fit placement, freeing with LIFO(last-in-first-out) policy,
 *       and boundary tag coalescing.
 *
 * v2.1: Compatible with next-fit placement and best-fit placement.
 *
 * v2.2: Freeing with address-ordered policy compatibly.
 *
 * v3.0: In this approach, the allocator based on segregated free lists,
 *       first-fit placement, freeing with LIFO(last-in-first-out) policy,
 *       and boundary tag coalescing.
 *
 * v3.1: Freeing with address-ordered policy compatibly.
 *
 * v3.2: Optimize mm_realloc.
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

/* $begin mallocmacros */

/*
 * Choose the allocator is based on which type of free list,
 * which type to find free block and which insertion policy
 * used by explicit free list.
 */

/*
 * If EXPT_LIST defined, the allocator based on explicit free lists,
 * else based on implicit free lists.
 */
#define EXPT_LIST
#ifdef EXPT_LIST
/*
 * If SEG_LIST defined, the allocator based on segregated explicit
 * free lists.
 */
#define SEG_LIST
#endif

/*
 * If ADDR_ORDERED defined, the free blocks insertion use
 * address-ordered policy, else use LIFO(last-in-first-out)
 * policy(based on explicit free lists or segregated free lists).
 */
#define ADDR_ORDERED

/*
 * Use first-fit search by default.
 * Segregated free list cannot use next-fit search.
 */
#ifndef SEG_LIST
/* If NEXT_FIT defined, use next fit search */
#define NEXT_FITx
#endif
/* If BEST_FIT defined, use best fit search */
#define BEST_FITx

/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */
#define DSIZE       8       /* Double word size (bytes) */
#define ALIGNMENT   DSIZE   /* Single word (4) or double word (8) alignment */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (4KB) */
#define MINBLOCK    2*DSIZE /* Minimum size of a block(except for prologue and epilogue) */

/* Block alloc state */
#define BLK_FREE    0b00    /* The block is free */
#define BLK_ALLOC   0b01    /* The block is allocated */
#define BLK_PALLOC  0b10    /* The previous block is allocated(only stored in header) */

/* Rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* Adjust block size to include overhead and alignment reqs */
#define ADJUST(size) ((size < 3*WSIZE) ? MINBLOCK : ALIGN(size+WSIZE))

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (unsigned int)(val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)   (GET(p) & ~0b111)
#define GET_ALLOC(p)  (GET(p) & BLK_ALLOC)
#define GET_PALLOC(p) (GET(p) & BLK_PALLOC)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) /* Only works for the block is free */

/* Given block ptr bp, compute address of previous and next blocks */
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) /* Only works for the previous block is free */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))

/* Given free block ptr bp, compute address of pointer of its predecessor and successor */
#define PRED_BLKPP(bp) ((char **)(bp))
#define SUCC_BLKPP(bp) ((char **)((char *)(bp) + WSIZE))

/* Given free block ptr bp, compute address of its predecessor and successor */
#define PRED_BLKP(bp)  (*PRED_BLKPP(bp))
#define SUCC_BLKP(bp)  (*SUCC_BLKPP(bp))
/* $end mallocmacros */

/* Global variables */
static char *heap_listp = 0;   /* Pointer to first block */
#ifdef SEG_LIST
#define CLASS_SIZE 5           /* Class size of segregated free lists */
static char **class_listp = 0; /* Pointer to size classes list */
#endif
#ifdef NEXT_FIT
static char *rover;           /* Next fit rover */
#endif

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t size);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
#ifdef EXPT_LIST
static void unjoin(void *bp);
static void join(void *pred, void *bp, void *succ);
static char *find_pred(void *bp);
#endif
#ifdef SEG_LIST
static char **match_heap(size_t asize);
#endif

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

#ifdef EXPT_LIST
    heap_listp = NULL; /* Initial free list is empty */
#ifdef SEG_LIST
    /*
     * Create the initial size class list.
     * There are CLASS_SIZE separate classes for each size block of free lists
     */
    if ((class_listp = mem_sbrk((CLASS_SIZE+1)*WSIZE)) == (void *)-1)
        return -1;
    PUT(HDRP(class_listp), PACK((CLASS_SIZE+1)*WSIZE, (BLK_ALLOC | BLK_PALLOC)));
    memset((char *)class_listp, 0, CLASS_SIZE*WSIZE);
    PUT(class_listp + CLASS_SIZE, PACK(0, (BLK_ALLOC | BLK_PALLOC)));
#endif
#endif

    /* Extend the empty heap with a free block of CHUNKSIZE */
    if (extend_heap(CHUNKSIZE) == NULL)
        return -1;

#ifdef NEXT_FIT
    rover = heap_listp;
#endif

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

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs */
    asize = ADJUST(size);

    /* Search the free list for a fit */
#ifdef SEG_LIST
    char **last_listpp = class_listp + (CLASS_SIZE-1);
    char **heap_listpp = match_heap(asize);

    while (heap_listpp <= last_listpp) {
        heap_listp = *heap_listpp;
        if (heap_listp && (bp = find_fit(asize))) {
            place(bp, asize);
            return bp;
        }
        heap_listpp++; /* Move to next size heap list */
    }
#else
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }
#endif

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if (!(bp = extend_heap(extendsize)))
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
    PUT(HDRP(np), PACK(nsize, nalloc)); /* Clear the palloc state */

    coalesce(bp);
}

/*
 * mm_realloc - Reallocates the given area of memory.
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize, asize;
    void *newptr, *bp;

    /* If size == 0, free the block and return NULL */
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    /* If ptr is NULL, malloc a block */
    if (ptr == NULL) {
        return mm_malloc(size);
    }

    /* Adjust block size to include overhead and alignment reqs */
    asize = ADJUST(size);

    oldsize = GET_SIZE(HDRP(ptr));
    if (asize <= oldsize) {
        if ((oldsize - asize) >= MINBLOCK) {
            /* Split a free block from old allocated area */
            PUT(HDRP(ptr), PACK(asize, (BLK_ALLOC | GET_PALLOC(HDRP(ptr)))));
            bp = NEXT_BLKP(ptr);
            PUT(HDRP(bp), PACK((oldsize-asize), (BLK_FREE | BLK_PALLOC)));
            PUT(FTRP(bp), PACK((oldsize-asize), BLK_FREE));
            mm_free(bp);
        }
        return ptr;
    }
    else {
        bp = NEXT_BLKP(ptr);
        if (!GET_ALLOC(HDRP(bp)) && (oldsize+GET_SIZE(HDRP(bp))) >= asize) {
            /* The next block is free and combine size bigger than asize */
            place(bp, (asize-oldsize));
            PUT(HDRP(ptr), PACK((oldsize+GET_SIZE(HDRP(bp))), (BLK_ALLOC | GET_PALLOC(HDRP(ptr)))));
            return ptr;
        }
        else {
            newptr = mm_malloc(size);
            if (!newptr) {
                return NULL;
            }

            /* Copy the old data */
            memcpy(newptr, ptr, oldsize);

            /* Free the old block */
            mm_free(ptr);
        }
    }


    return newptr;
}

/*
 * Internal helper routines
 */

/*
 * extend_heap - Extend heap with free block and return its block pointer.
 */
static void *extend_heap(size_t size)
{
    char *bp;
    size_t asize;
    size_t palloc;

    /* Allocate an multiple of ALIGNMENT number bytes to maintain alignment */
    asize = ALIGN(size);
    if ((long)(bp = mem_sbrk(asize)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    palloc = GET_PALLOC(HDRP(bp));
    PUT(HDRP(bp), PACK(asize, (BLK_FREE | palloc)));
    PUT(FTRP(bp), PACK(asize, BLK_FREE));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, BLK_ALLOC));

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least MINBLOCK.
 * Note: If based on explicit free lists, the predecessor and
 *       successor of old block might be NULL.
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    char *old_bp = bp;

    if ((csize - asize) >= MINBLOCK) {
    #ifdef SEG_LIST
        /*
         * The size in the header is precise for unjoin now(csize),
         * later will change to asize.
         */
        unjoin(old_bp);
    #endif
        PUT(HDRP(bp), PACK(asize, (BLK_ALLOC | BLK_PALLOC)));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK((csize-asize), (BLK_FREE | BLK_PALLOC)));
        PUT(FTRP(bp), PACK((csize-asize), BLK_FREE));

    #ifdef EXPT_LIST

    #ifdef SEG_LIST
        coalesce(bp);
    #else
        join(PRED_BLKP(old_bp), bp, SUCC_BLKP(old_bp));
    #endif

    #ifdef NEXT_FIT
        rover = bp;
    #endif

    #endif
    }
    else {
        PUT(HDRP(bp), PACK(csize, (BLK_ALLOC | BLK_PALLOC)));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(GET_SIZE(HDRP(bp)), (BLK_ALLOC | BLK_PALLOC)));

    #ifdef EXPT_LIST
        unjoin(old_bp);

    #ifdef NEXT_FIT
        rover = SUCC_BLKP(rover);
    #endif

    #endif
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

#ifdef EXPT_LIST
    /* Search from the rover to the end of list */
    for ( ; rover != NULL; rover = SUCC_BLKP(rover)) {
        if (GET_SIZE(HDRP(rover)) >= asize) {
            return rover;
        }
    }

    /* Search from start of list to old rover */
    for (rover = heap_listp; rover != oldrover; rover = SUCC_BLKP(rover)) {
        if (GET_SIZE(HDRP(rover)) >= asize) {
            return rover;
        }
    }
#else
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
#endif

#elif defined(BEST_FIT)
    /* Best fit search */
    void *bp1, *bp2 = NULL;

#ifdef EXPT_LIST
    for (bp1 = heap_listp; bp1 != NULL; bp1 = SUCC_BLKP(bp1)) {
        if (GET_SIZE(HDRP(bp1)) >= asize && (!bp2 || (GET_SIZE(HDRP(bp1)) < GET_SIZE(HDRP(bp2))))) {
            bp2 = bp1;
        }
    }
#else
    for (bp1 = heap_listp; GET_SIZE(HDRP(bp1)) > 0; bp1 = NEXT_BLKP(bp1)) {
        if (!GET_ALLOC(HDRP(bp1)) && (GET_SIZE(HDRP(bp1)) >= asize) &&
           (!bp2 || (GET_SIZE(HDRP(bp1)) < GET_SIZE(HDRP(bp2))))) {
            bp2 = bp1;
        }
    }
#endif
    return bp2;

#else
    /* First fit search */
    void *bp;

#ifdef EXPT_LIST
    for (bp = heap_listp; bp != NULL; bp = SUCC_BLKP(bp)) {
        if (GET_SIZE(HDRP(bp)) >= asize) {
            return bp;
        }
    }
#else
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (GET_SIZE(HDRP(bp)) >= asize)) {
            return bp;
        }
    }
#endif

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
    char *bq;

    /* Case 1 */
    if (palloc && nalloc) {
    #ifdef EXPT_LIST

    #ifdef ADDR_ORDERED
        /* Address-ordered policy */
        bq = find_pred(bp);
        bq ? join(bq, bp, SUCC_BLKP(bq)) : join(NULL, bp, heap_listp);
    #else
        /* LIFO policy */
        join(NULL, bp, heap_listp);
    #endif

    #endif
    }

    /* Case 2 */
    else if (palloc && !nalloc) {
    #ifdef SEG_LIST
        unjoin(NEXT_BLKP(bp));
    #endif
        bq = NEXT_BLKP(bp);
        size += GET_SIZE(HDRP(bq));
        PUT(HDRP(bp), PACK(size, (BLK_FREE | palloc)));
        PUT(FTRP(bp), PACK(size, BLK_FREE));

    #ifdef EXPT_LIST

    #ifdef ADDR_ORDERED
        /* Address-ordered policy */
    #ifdef SEG_LIST
        bq = find_pred(bp);
        bq ? join(bq, bp, SUCC_BLKP(bq)) : join(NULL, bp, heap_listp);
    #else
        join(PRED_BLKP(bq), bp, SUCC_BLKP(bq));
    #endif

    #else
        /* LIFO policy */
        unjoin(bq);
        join(NULL, bp, heap_listp);
    #endif

    #endif
    }

    /* Case 3 */
    else if (!palloc && nalloc) {
    #ifdef SEG_LIST
        unjoin(PREV_BLKP(bp));
    #endif

        bq = bp;
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, (BLK_FREE | BLK_PALLOC)));
        PUT(FTRP(bp), PACK(size, BLK_FREE));
        bp = PREV_BLKP(bp);

    #ifdef EXPT_LIST

    #ifdef ADDR_ORDERED
        /* Address-ordered policy */
    #ifdef SEG_LIST
        bq = find_pred(bp);
        bq ? join(bq, bp, SUCC_BLKP(bq)) : join(NULL, bp, heap_listp);
    #else
        ; /* Don't need to do sth else in this case */
    #endif
    #else
        /* LIFO policy */
    #ifdef SEG_LIST
        join(NULL, bp, heap_listp);
    #else
        if (bp == heap_listp) { /* The first free block is coalesced */
            ;
        }
        else {
            unjoin(bp);
            join(NULL, bp, heap_listp);
        }
    #endif

    #endif

    #endif
    }

    /* Case 4 */
    else {
    #ifdef SEG_LIST
        unjoin(PREV_BLKP(bp));
        unjoin(NEXT_BLKP(bp));
    #endif

        bq = NEXT_BLKP(bp);
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
            GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, (BLK_FREE | BLK_PALLOC)));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, BLK_FREE));
        bp = PREV_BLKP(bp);

    #ifdef EXPT_LIST

    #ifdef ADDR_ORDERED
        /* Address-ordered policy */
    #ifdef SEG_LIST
        bq = find_pred(bp);
        bq ? join(bq, bp, SUCC_BLKP(bq)) : join(NULL, bp, heap_listp);
    #else
        join(PRED_BLKP(bp), bp, SUCC_BLKP(bq));
    #endif
    #else
        /* LIFO policy */
    #ifdef SEG_LIST
        join(NULL, bp, heap_listp);
    #else
        if (bp == heap_listp) {
            unjoin(bq);
        }
        else {
            unjoin(bq);
            unjoin(bp);
            join(NULL, bp, heap_listp);
        }
    #endif

    #endif

    #endif
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

#ifdef EXPT_LIST

/*
 * unjoin - Given a block ptr bp, unjoin it from block list.
 *          The size info in the header of block should be precise.
 */
static void unjoin(void *bp) {
#ifdef SEG_LIST
    char **heap_listpp;
    heap_listp = *(heap_listpp = match_heap(GET_SIZE(HDRP(bp))));
#endif

    if (SUCC_BLKP(bp)) PUT(PRED_BLKPP(SUCC_BLKP(bp)), PRED_BLKP(bp));
    if (PRED_BLKP(bp)) PUT(SUCC_BLKPP(PRED_BLKP(bp)), SUCC_BLKP(bp));
    if (bp == heap_listp) {
        /* The block is the first block of the list */
        heap_listp = SUCC_BLKP(bp);
    }

#ifdef SEG_LIST
    *heap_listpp = heap_listp;
#endif
}

/*
 * join - Given a block ptr bp, join it between pred and succ,
 *        if pred is NULL, join it as the first block.
 *        The size info in the header of block should be precise.
 */
static void join(void *pred, void *bp, void *succ) {
#ifdef SEG_LIST
    char **heap_listpp;
    heap_listp = *(heap_listpp = match_heap(GET_SIZE(HDRP(bp))));
    if (!pred) succ = heap_listp; /* Redirect the heap list */
#endif

    PUT(PRED_BLKPP(bp), pred);
    PUT(SUCC_BLKPP(bp), succ);
    if (succ) PUT(PRED_BLKPP(succ), bp);
    if (pred) PUT(SUCC_BLKPP(pred), bp); else (heap_listp = bp);

#ifdef SEG_LIST
    *heap_listpp = heap_listp;
#endif
}

/*
 * find_pred - Given a block pointer bp, find its predcessor address-ordered.
 */
static char *find_pred(void *bp) {
#ifdef SEG_LIST
    heap_listp = *(match_heap(GET_SIZE(HDRP(bp))));
#endif

    char *bq = heap_listp;

    if (!heap_listp || (bp < heap_listp)) { /* Empty free list or addr(bp) < addr(heap_listp) */
        return NULL;
    }

    while (SUCC_BLKP(bq) && (SUCC_BLKP(bq) < bp)) {
        bq = SUCC_BLKP(bq);
    }
    return bq;
}

#endif

#ifdef SEG_LIST

/*
 * match_heap - Given the asize, return pointer of heap matches the class of size.
 */
static char **match_heap(size_t asize) {
    if (asize <= 2*MINBLOCK) {      /* 1-2 block-size */
        return class_listp;
    }
    else if (asize <= 3*MINBLOCK) { /* 3 block-size */
        return class_listp + 1;
    }
    else if (asize <= 4*MINBLOCK) { /* 4 block-size */
        return class_listp + 2;
    }
    else if (asize <= 8*MINBLOCK) { /* 5-8 block-size */
        return class_listp + 3;
    }
    else {                          /* 9-inf block-size */
        return class_listp + 4;
    }

    return NULL;
}

#endif