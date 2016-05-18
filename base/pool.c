#include "pool.h"

#include <assert.h>
#include <stdint.h>


int chunk_init(chunk_t* chunk, int width, int size)
{
    assert(width != 0 && size != 0);
    chunk->data = malloc(width * size);
    if (chunk->data == NULL)
        return ERROR;
    
    uint8_t* ele = chunk->data;
    for (int i = 0; i < size - 1; i++) {
        ((chunk_slot_t*)ele)->next = ele + width;
        ele += width;
    }
    ((chunk_slot_t*)ele)->next = NULL;
    
    chunk->width = width;
    return OK;
}

static inline void chunk_clear(chunk_t* chunk)
{
    free(chunk->data);
    chunk->data = NULL;
}

int pool_init(pool_t* pool, int width, int chunk_size, int nchunks)
{
    pool->width = max(width, sizeof(chunk_slot_t));
    pool->chunk_size = chunk_size;
    pool->nallocated = 0;
    int err = vector_init(&pool->chunks, sizeof(chunk_t), nchunks);
    pool->cur = vector_at(&pool->chunks, 0);
    return err;
}

void* pool_alloc(pool_t* pool)
{
    if (pool->cur == NULL) {
        // The chunk is full
        chunk_t* new_chunk = vector_alloc(&pool->chunks);
        if (new_chunk == NULL)
            return NULL;
        if (chunk_init(new_chunk, pool->width, pool->chunk_size) != OK)
            return NULL;
        pool->cur = new_chunk->data;
    }
    
    void* ret = pool->cur;
    if (pool->cur != NULL) {
        ++pool->nallocated;
        pool->cur = ((chunk_slot_t*)pool->cur)->next;
    }
    return ret;
}

void pool_clear(pool_t* pool)
{
    for (int i = 0; i < pool->chunks.size; i++)
        chunk_clear(vector_at(&pool->chunks, i));
    vector_clear(&pool->chunks);
    
    pool->cur = NULL;
    pool->nallocated = 0;
}
