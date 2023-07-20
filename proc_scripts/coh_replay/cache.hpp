#ifndef CACHESIM_HPP
#define CACHESIM_HPP

#include <map>
#include <list>
#include <algorithm>
#include <stdint.h>
#include <stdbool.h>
#include <iostream>

#define CPU_CACHE_BLOCK_SIZE 6
#define LINEBITS 6
#define ULL unsigned long long
//#define NUM_WAYS 16
#define NUM_WAYS 24 //for including l2 - 1MB per core, 4MB per socket
//#define NUM_SETS 32768 //32MB cache size
#define NUM_SETS 8192 //8MB cache size 

//#define NUM_NODES 1

typedef enum {
    READ,
    WRITE,
} access_types_t;

typedef enum {
    I, 
    S,
	E,
	M,
} coh_state_t;

typedef struct cache_entry {
    uint64_t tag;   //addr
    bool valid;                 // valid bit
    bool dirty;                 // dirty bit
    coh_state_t cstate;      // coherence state
    uint64_t ts; //timestamp
} cache_entry_t;

// typedef struct cache_set {
//     cache_entry_t entries[NUM_WAYS];
//     std::list<uint8_t> lru_list;
// } cache_set_t;

typedef struct cache {
    cache_entry_t entries[NUM_SETS][NUM_WAYS];
    //std::list<uint32_t> lru_list[NUM_SETS];
    uint32_t socket_id;
} cache_t;



#endif /* CACHESIM_HPP */
