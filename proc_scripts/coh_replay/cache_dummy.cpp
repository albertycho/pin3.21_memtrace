
#include <cassert>
#include <cstring>
#include <cmath>
#include <iostream>
#include <vector>

#include "cachesim.hpp"

std::vector<uint64_t> lv_addr_offset;
uint64_t total_levels;

/**
 * @brief Subroutine for initializing the cache simulator. You many add and initialize any global or heap
 * variables as needed.
 * 
 * @param config Simulation config
 */

void sim_setup(cache_t *cache_core, sim_config_t *config) {

    for (int i=0; i<NUM_NODES; i++){
        cache_core[i].c = config->c;
        cache_core[i].b = 6;
        cache_core[i].s = config->s;
        cache_core[i].idx = config->c - config->s - cache_core[i].b;
        cache_core[i].cache = new std::map<uint64_t, cache_entry_t>[1 << cache_core[i].idx];
        cache_core[i].lruQ = new std::list<uint64_t>[1 << cache_core[i].idx];
        cache_core[i].set_entries = new uint64_t[1 << cache_core[i].idx]();
        cache_core[i].tag_compare_time = L1_TAG_COMPARE_TIME_CONST + L1_TAG_COMPARE_TIME_PER_S * (cache_core[i].s);
    }
}

/**
 * @brief Access Metadata Cache
 * 
 * @param addr Address to access
 * @param rw 0 for Read or 1 for Write
 * @param stats Simulation stats
 */
bool sim_access_cache(cache_t *cache, uint64_t pfn, bool rw, sim_stats_t* stats) {
    bool res = true;
    uint64_t idx = pfn % (1 << cache->idx);
    uint64_t tag = pfn >> cache->idx;
    stats->accesses_l1++;
    if (rw == READ) {
        stats->eff_reads++;
    } else {
        stats->eff_writes++;
    }
    if (cache->cache[idx][tag].valid) {
        // hit
        stats->hits_l1++;
        if (rw == WRITE)
            cache->cache[idx][tag].dirty = true;
        auto it = std::find(cache->lruQ[idx].begin(), cache->lruQ[idx].end(), tag);
        if (it != cache->lruQ[idx].end())
            cache->lruQ[idx].remove(tag);
        cache->lruQ[idx].push_back(tag);
        return res;
    }
    // miss
    res = false;
    stats->misses_l1++;
    if (cache->set_entries[idx] == (uint64_t)(1 << cache->s)) {
        // victim needed if set is full
        uint64_t victimTag = cache->lruQ[idx].front();
        if (cache->cache[idx][victimTag].dirty) {
            ++stats->num_dram_accesses;
            ++stats->num_dram_writes;
            stats->writebacks_l1++;
        }
        // Remove the victim
        cache->cache[idx][victimTag].valid = false;
        cache->cache[idx][victimTag].dirty = false;
        cache->set_entries[idx]--;
        cache->lruQ[idx].pop_front();
    }
    cache->set_entries[idx]++;
    cache->lruQ[idx].push_back(tag);
    cache->cache[idx][tag].valid = true;
    if (rw == WRITE) {
        cache->cache[idx][tag].dirty = true;
    }
    if (rw == READ) {
        ++stats->num_dram_accesses;
        ++stats->num_dram_reads;
    }
    return res;
}

static int64_t sim_verify_access(cache_t *cache, uint32_t level, uint64_t pfn, sim_stats_t *stats, bool eager,
                                 bool rw) {
    if (level == total_levels - 1) {
    #ifdef DEBUG
        std::cout << "VERIFY: Received hit at root" << std::endl;
    #endif
        return level;
    }
    pfn = pfn % (MAX_MEM_SIZE / (1ULL << CPU_CACHE_BLOCK_SIZE));
    uint64_t metadata_offset = pfn >> ((level + 1) * BLOCKS_PER_TOC_NODE);
    uint64_t metadata_pfn = lv_addr_offset[level] + metadata_offset;
    //std::cout << /*std::hex << */pfn << " " << act_addr << " " << level << std::endl;
    //uint64_t act_addr = pfn >> ((level + 1) * 1);
#ifdef DEBUG
    std::cout << "VERIFY: Generated address " << std::hex << metadata_pfn << " for level " << std::dec << level
              << ", pfn " << std::hex << pfn << std::endl;
#endif
    bool hit = sim_access_cache(cache, metadata_pfn, READ, stats);
    if (rw == WRITE || !hit) {
    #ifdef DEBUG
        std::cout << "VERIFY: Received miss at level " << level << std::endl;
    #endif
        return sim_verify_access(cache, level + 1, pfn, stats, eager, rw);
    }
#ifdef DEBUG
    std::cout << "VERIFY: Received hit at level " << level << std::endl;
#endif
    return level;
}

static void sim_write_access(cache_t *cache, uint32_t level, uint64_t pfn, sim_stats_t *stats, bool eager) {
    assert(eager);
    // TODO: Lazy update

    if (level == total_levels - 1) {
        return;     // Stop recursion at root
    }
    // Somehow force to <16GB??
    pfn = pfn % (MAX_MEM_SIZE / (1ULL << CPU_CACHE_BLOCK_SIZE));
    uint64_t metadata_offset = pfn >> ((level + 1) * BLOCKS_PER_TOC_NODE);
    uint64_t metadata_pfn = lv_addr_offset[level] + metadata_offset;
#ifdef DEBUG
    std::cout << "WRITE: Writing to address " << std::hex << metadata_pfn << " for level " << std::dec << level
              << ", pfn " << std::hex << pfn << std::endl;
#endif
    sim_access_cache(cache, metadata_pfn, WRITE, stats);   // Need to stop somewhere for lazy
    ++stats->num_dram_accesses;
    ++stats->num_dram_writes;
    sim_write_access(cache, level + 1, pfn, stats, eager);
}

/**
 * @brief Subroutine that simulates the cache one trace event at a time.
 * 
 *  @param rw 0 for Read or 1 for Write
 *  @param addr Address being accessed
 *  @param stats Simulation stats
 */
void sim_access(cache_t *cache, bool rw, uint64_t addr, sim_stats_t* stats) {
    // 64 bytes --> 1 CPU block
    // 8 blocks --> 1 entry
    uint64_t addr_pfn = addr >> CPU_CACHE_BLOCK_SIZE;
    int lv_hit = 0;
    bool eager = true;
    if (rw == READ) {
    #ifdef DEBUG
        std::cout << "ACCESS: Sending pfn " << std::hex << addr_pfn << " for addr " << addr << " to verify\n";
    #endif
        stats->reads++;
        lv_hit = sim_verify_access(cache, 0, addr_pfn, stats, eager,  READ);
        stats->total_levels += lv_hit;
    #ifdef DEBUG
        std::cout << "ACCESS: Verified pfn " << std::hex << addr_pfn << std::dec << " at level " << lv_hit << std::endl;
    #endif
    } else {
#ifdef DEBUG
        std::cout << "ACCESS: Sending pfn " << std::hex << addr_pfn << " for addr " << addr << " to verify\n";
#endif
        stats->writes++;
        // Go till root
        lv_hit = sim_verify_access(cache, 0, addr_pfn, stats, eager, WRITE);
        stats->total_levels += lv_hit;
    #ifdef DEBUG
        std::cout << "Verified pfn " << std::hex << addr_pfn << std::dec << " at level " << lv_hit << std::endl;
    #endif
        // Set dirty bits
        sim_write_access(cache, 0, addr_pfn, stats, eager);
    }
    // Generate eq metadata cache address
    // Issue a cache access and see if hit
    // If not go to next level, place it in cache
    // Keep going till hit
}

void compute_stats(cache_t *cache, sim_stats_t *stats) {
    //double tag_compare_time = L1_TAG_COMPARE_TIME_CONST + L1_TAG_COMPARE_TIME_PER_S * (cache->s);
    double tag_compare_time = cache->tag_compare_time;
    stats->hit_ratio_l1 = (stats->hits_l1 * 1.0/stats->accesses_l1);
    stats->miss_ratio_l1 = (stats->misses_l1 * 1.0/stats->accesses_l1);
    stats->avg_access_time = ((L1_ARRAY_LOOKUP_TIME_CONST + tag_compare_time) *
                                stats->accesses_l1 + DRAM_ACCESS_PENALTY * stats->misses_l1 * 1.0)/
                                stats->accesses_l1;
    stats->avg_level = stats->total_levels * 1.0/(stats->reads + stats->writes);
}

/**
 * @brief Subroutine for cleaning up any outstanding memory operations and calculating overall statistics
 * such as miss rate or average access time.
 * 
 * @param stats Simulation stats
 */
void sim_finish(cache_t *cache, sim_stats_t *stats) {
    for(int i=0;i<NUM_NODES;i++){
    compute_stats(&(cache[i]), &(stats[i]));
    delete[] cache[i].cache;
    delete[] cache[i].lruQ;
    delete[] cache[i].set_entries;
    }
}
