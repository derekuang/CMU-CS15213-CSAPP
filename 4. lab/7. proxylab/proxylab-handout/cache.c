/*
 * cache.c - Functions for cache operations
 */

#include "cache.h"
#include "csapp.h"

static sem_t mutex; /* Mutex protect from race of cache */

/*
 * init_object - Initialize the cache object
 */
void init_object(object_t *objp)
{
    objp->size = 0;
    memset(objp->obj_buf, '\0', MAX_OBJECT_SIZE);
    objp->obj_bufptr = objp->obj_buf;
    objp->next = NULL;
}

/*
 * init_cache - Initialize the cache state
 */
void init_cache(cache_t *cachep)
{
    Sem_init(&mutex, 0, 1);
    cachep->size = 0;
    cachep->root = Malloc(sizeof(object_t));
    init_object(cachep->root);
}

/*
 * fetch_object - Fetch object from cache by URL
 */
object_t *fetch_object(cache_t *cachep, char *host, char *port, char *uri)
{
    object_t *p, *q;
    char url[1024];

    sprintf(url, "%s:%s%s", host, port, uri);

    P(&mutex);
    p = cachep->root;
    q = p->next;

    while (q) {
        if (!strcmp(q->url, url)) {
            /* Find the object successful, put it on the head of the list */
            p->next = q->next;
            q->next = cachep->root->next;
            cachep->root->next = q;
            V(&mutex);
            return q;
        }
        p = q;
        q = q->next;
    }
    V(&mutex);

    return NULL;
}

/*
 * receive_object - Receive bytes for the current object
 */
void receive_object(object_t *objp, char *bufp, ssize_t size)
{
    if (!objp->obj_bufptr) {
        /* Current object's size has exceeded MAX_OBJECT_SIZE */
        return;
    }

    if ((objp->size+size) > MAX_OBJECT_SIZE) {
        /* Current object's size exceed MAX_OBJECT_SIZE, set obj_bufptr to NULL */
        objp->obj_bufptr = NULL;
        return;
    }

    objp->size += size;
    memcpy(objp->obj_bufptr, bufp, size);
    objp->obj_bufptr += size;
}

/*
 * insert_object - Insert an object to cache
 */
void insert_object(cache_t *cachep, object_t *objp)
{
    object_t *root = cachep->root;

    if (!objp->obj_bufptr) {
        return;
    }

    P(&mutex);
    while ((cachep->size+objp->size) > MAX_CACHE_SIZE) {
        remove_object(cachep);
    }

    objp->next = root->next;
    root->next = objp;
    cachep->size += objp->size;
    V(&mutex);
}

/*
 * remove_object - Remove the last object of the cache(LRU policy)
 */
void remove_object(cache_t *cachep)
{
    object_t *p, *q;

    p = cachep->root;
    q = p->next;

    if (!q) {
        /* Empty object list */
        return;
    }

    while (q->next) {
        p = q;
        q = p->next;
    }
    p->next = NULL;
    cachep->size -= q->size;
    Free(q);
}