#ifndef POLICY_H
#define POLICY_H

#include <stdint.h>
#include "config.h"

/*
 * A CacheLine as seen by the policy module.
 * The full definition lives in cache.h; we forward-declare here so
 * policy.h can be included before cache.h if needed.
 */
typedef struct CacheLine CacheLine;

/*
 * Select the index (0..assoc-1) of the line to evict from a set.
 *
 * Rules (same for both policies):
 *   1. If any line is invalid (valid == 0), return its index immediately
 *      — no eviction of a real block is needed.
 *   2. Among valid lines:
 *      LRU  → pick the line with the smallest last_used timestamp.
 *      FIFO → pick the line with the smallest inserted_at timestamp.
 *
 * lines : pointer to the first CacheLine in the set (array of `assoc` lines)
 * assoc : number of ways in the set
 * policy: LRU or FIFO
 */
int find_victim(CacheLine *lines, int assoc, ReplacementPolicy policy);

#endif /* POLICY_H */
