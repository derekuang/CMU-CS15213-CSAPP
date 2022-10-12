/*
 * cache.h - prototypes and definitions for proxy cache helper
 */
#ifndef __CACHE_H__
#define __CACHE_H__

#include <stdio.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* Structure of an cache object */
typedef struct object_t {
    int size;                       /* Size of the object */
    char url[1024];                 /* URL of the object */
    char *obj_bufptr;               /* Next unwritten byte in internal buf */
    char obj_buf[MAX_OBJECT_SIZE];  /* Internal object buffer */
    struct object_t *next;          /* Pointer to the next object */
} object_t;

/*
 * State structure of the cache, maintain a cache object list
 */
typedef struct {
    int size;           /* Total size of cache object */
    object_t *root;     /* Root pointer of the object list */
} cache_t;

/* cache related function */
void init_object(object_t *objp);
void init_cache(cache_t *cachep);
object_t *fetch_object(cache_t *cachep, char *host, char *port, char *uri);
void receive_object(object_t *objp, char *bufp, ssize_t size);
void insert_object(cache_t *cachep, object_t *objp);
void remove_object(cache_t *cachep);

#endif /* __CACHE_H__ */