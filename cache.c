#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "policy.h"

CacheLevel *cache_level_init(const char *name, int cache_size,
                             int block_size, int associativity, int latency) {
    CacheLevel *lvl = malloc(sizeof(CacheLevel));
    if (!lvl) { perror("malloc"); exit(EXIT_FAILURE); }

    strncpy(lvl->name, name, sizeof(lvl->name) - 1);
    lvl->name[sizeof(lvl->name) - 1] = '\0';
    lvl->cache_size   = cache_size;
    lvl->block_size   = block_size;
    lvl->associativity= associativity;
    lvl->num_lines    = cache_size / block_size;
    lvl->num_sets     = lvl->num_lines / associativity;
    lvl->offset_bits  = log2_int(block_size);
    lvl->index_bits   = log2_int(lvl->num_sets);
    lvl->latency      = latency;

    /* Allocate sets */
    lvl->sets = malloc(sizeof(CacheSet) * lvl->num_sets);
    if (!lvl->sets) { perror("malloc"); exit(EXIT_FAILURE); }

    /* Allocate lines within each set; zero-init means valid=0, dirty=0 */
    for (int s = 0; s < lvl->num_sets; s++) {
        lvl->sets[s].lines = calloc(associativity, sizeof(CacheLine));
        if (!lvl->sets[s].lines) { perror("calloc"); exit(EXIT_FAILURE); }
    }

    memset(&lvl->stats, 0, sizeof(Stats));
    return lvl;
}

int cache_lookup(CacheLevel *level, uint64_t address, DecodedAddr *out) {
    *out = decode_address(address, level->offset_bits, level->index_bits);
    CacheSet *set = &level->sets[out->index];

    for (int w = 0; w < level->associativity; w++) {
        CacheLine *line = &set->lines[w];
        if (line->valid && line->tag == out->tag)
            return w;   /* hit — return way index */
    }
    return -1;          /* miss */
}

int cache_insert(CacheLevel *level, uint64_t address,
                 ReplacementPolicy policy, uint64_t timestamp,
                 uint64_t *writeback_addr) {
    DecodedAddr d = decode_address(address, level->offset_bits, level->index_bits);
    CacheSet   *set = &level->sets[d.index];

    int victim = find_victim(set->lines, level->associativity, policy);
    CacheLine *vline = &set->lines[victim];

    int dirty_eviction = 0;

    if (vline->valid) {
        level->stats.evictions++;

        if (vline->dirty) {
            level->stats.dirty_writebacks++;
            dirty_eviction = 1;
            /*
             * Reconstruct the byte address of the evicted block so the
             * caller can write it to the next level.
             * block_address = (tag << index_bits) | index
             * byte_address  = block_address << offset_bits
             */
            uint64_t evicted_block = (vline->tag << level->index_bits) | d.index;
            *writeback_addr = evicted_block << level->offset_bits;
        }
    }

    /* Install the new block */
    vline->tag         = d.tag;
    vline->valid       = 1;
    vline->dirty       = 0;
    vline->last_used   = timestamp;
    vline->inserted_at = timestamp;

    return dirty_eviction;
}

void cache_mark_dirty(CacheLevel *level, uint64_t address, int way) {
    DecodedAddr d = decode_address(address, level->offset_bits, level->index_bits);
    level->sets[d.index].lines[way].dirty = 1;
}

void cache_update_lru(CacheLevel *level, uint64_t address,
                      int way, uint64_t timestamp) {
    DecodedAddr d = decode_address(address, level->offset_bits, level->index_bits);
    level->sets[d.index].lines[way].last_used = timestamp;
}

void cache_free(CacheLevel *level) {
    if (!level) return;
    for (int s = 0; s < level->num_sets; s++)
        free(level->sets[s].lines);
    free(level->sets);
    free(level);
}
