#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <stdint.h>
#include <limits.h>
#include <cstdio>
#include <cassert>

#include <cmath>
#include <queue>
#include <stack>
#include <set>
#include <unordered_map>
#include <omp.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include "cache.hpp" 

#define PAGESIZE 4096
#define PAGEBITS 12
#define N_THR 64
#define N_SOCKETS 16

#define THR_OFFSET_VAL 0
#define N_THR_OFFSET (N_THR+1)
#define N_SOCKETS_OFFSET (N_SOCKETS+1)
#define U64 uint64_t

#define INVAL_OWNER 9999
#define NBILLION 1000000000
#define CXO 100

using namespace std;




/// structs for DIRECTORY
enum class DState {
    I, 
    S,
	E,
	M,
    // add more states if required
};

struct DirectoryEntry {
    DState state;
    uint64_t owner;
    uint64_t sharers;
};
typedef struct sim_stats {
    uint64_t m_reads;                 // read requests
    uint64_t m_writes;                // write requests
	uint64_t c_invals;
	uint64_t c_block_trans;
	uint64_t total_m_insts;
	uint64_t llc_hits;
	uint64_t llc_misses;
} sim_stats_t;


uint64_t syscycle;
//cache coherence directory
//using CCD = std::unordered_map<uint64_t, DirectoryEntry>;
unordered_map<uint64_t, DirectoryEntry> CCD[NUM_SETS]={}; //split into sets for quicker indexing?
cache_t caches[N_SOCKETS];
sim_stats_t simstats={0};
std::ofstream output_log("coh_repl_log.txt");


///cache functions

void init_caches(){
	for(int i=0; i<N_SOCKETS;i++){
		caches[i].socket_id=i; //maybe pointless. wtv
		for(int j=0; j<NUM_SETS;j++){
			for(int k=0; k<NUM_WAYS;k++){
				caches[i].entries[j][k].valid=false;
				caches[i].entries[j][k].dirty=false;
				caches[i].entries[j][k].cstate=coh_state_t::I;
				caches[i].entries[j][k].ts=0;

				//caches[i].lru_list[j].push_back(k); //initialize lrulist with default order
			}
		}
	}
	simstats.c_block_trans=0;
	simstats.c_invals=0;
	simstats.m_reads=0;
	simstats.m_writes=0;
	simstats.total_m_insts=0;
	simstats.llc_hits=0;
	simstats.llc_misses=0;
}

void update_directory(uint64_t socket_id, uint64_t lineaddr){

}


uint64_t access_cache(cache_t& cach, uint64_t lineaddr, bool isW, uint64_t ts, uint32_t socketid){
    //uint64_t lineaddr = addr>>LINEBITS;
	uint64_t set_index = lineaddr & (NUM_SETS-1); //only works if NUM_SETS is power of 2
    //uint64_t set_index_dbg = lineaddr % NUM_SETS;
    //std::cout<<"set_i    :"<<set_index<<std::endl;
    //std::cout<<"set_i_dbg:"<<set_index_dbg<<std::endl;

	simstats.total_m_insts++;
    //find hit
    uint64_t hit_i=0;
    bool ishit = false;
    //find inval
    uint64_t insert_i=0;
    bool i_entry_found = false;
    //find lru
    uint64_t lru_ts = ~0;
    uint64_t lru_i = 0;
    //assert(999999999999<lru_ts); //dbg - TODO - remove

    //TODO check for hit first!
    for(int j=0; j<NUM_WAYS;j++){
        if(cach.entries[set_index][j].valid==false){
            insert_i=j;
            i_entry_found=true;
            //break;
        }
        else if(cach.entries[set_index][j].tag==lineaddr){
            hit_i=j;
            ishit=true;
            break;            
        }
        //finding lru
        if(cach.entries[set_index][j].ts<lru_ts){
            lru_ts = cach.entries[set_index][j].ts;
            lru_i=j;
        }
    }

	//update coherence directory here?
	update_directory(socketid, lineaddr);

    if(ishit){
		//std::cout<<"ishit"<<endl;
		simstats.llc_hits++;
        if(isW){
            cach.entries[set_index][hit_i].dirty=true;            
        }
        cach.entries[set_index][hit_i].ts=ts;

        return 0;
    }

	simstats.llc_misses++;

    if(i_entry_found==false){
        insert_i = lru_i;
        //TODO handle evicted line directory/coherence:
        // if state is E or M --> update directory state to I
        // if state is M --> m_write++;

    }

    cach.entries[set_index][insert_i].tag=lineaddr;
    cach.entries[set_index][insert_i].valid=true;
    cach.entries[set_index][insert_i].ts = ts;
    cach.entries[set_index][insert_i].dirty=false;
	cach.entries[set_index][insert_i].cstate=coh_state_t::E;
    if(isW){ 
		cach.entries[set_index][insert_i].dirty=true;
		cach.entries[set_index][insert_i].cstate=coh_state_t::M;
	}
    //TODO handle directory/coherence
    
	//std::cout<<"inserted index: "<<insert_i<<endl;

    return 0;
}


int read_8B_line(uint64_t * buf_val, char* buffer, FILE* fptr){
	size_t readsize = std::fread(buffer, sizeof(char), sizeof(buffer), fptr);
	if(readsize!=8) return -1;
	//dunno why but this step needed for things t work
	std::memcpy(buf_val, buffer, sizeof(U64)); 
	return readsize;
}

void log_cur_stats(){
	cout<<"Total Accesses	:"<<simstats.total_m_insts<<endl;
	cout<<"LLC hits			:"<<simstats.llc_hits<<endl;
	cout<<"LLC misses		:"<<simstats.llc_misses<<endl;
	cout<<"Mem Reads		:"<<simstats.m_reads<<endl;
	cout<<"Mem Writes		:"<<simstats.m_writes<<endl;
	cout<<"Coherence Invals	:"<<simstats.c_invals<<endl;
	cout<<"C Block Transfer	:"<<simstats.c_block_trans<<endl;
	cout<<endl;

	output_log<<"Total Accesses	:"<<simstats.total_m_insts<<endl;
	output_log<<"LLC hits			:"<<simstats.llc_hits<<endl;
	output_log<<"LLC misses		:"<<simstats.llc_misses<<endl;
	output_log<<"Mem Reads		:"<<simstats.m_reads<<endl;
	output_log<<"Mem Writes		:"<<simstats.m_writes<<endl;
	output_log<<"Coherence Invals	:"<<simstats.c_invals<<endl;
	output_log<<"C Block Transfer	:"<<simstats.c_block_trans<<endl;
	output_log<<endl;
}

int log_misc_stats(U64 memory_touched_inMB, U64 total_num_accs_0, U64 sumallacc, U64 migrated_pages,
	U64 migrated_pages_CI, U64 pages_to_CI, U64 CXI_count, ofstream& misc_log_full){
	// cout<<"total memory touched in this phase: "<<(memory_touched_inMB)<<"MB"<<endl;
	// cout<<"total number of accesses in  this phase by t0: "<<total_num_accs_0<<endl;
	// cout<<"total number of accesses in  this phase by all t: "<<sumallacc<<endl;
	
	// cout<<"Baseline   migrations: "<<migrated_pages<<endl;
	// cout<<"CXLisland  migrations: "<<(migrated_pages_CI+pages_to_CI)<<"("<<migrated_pages_CI<<" not counting pages to CI)"<<endl;
	// cout<<"Migration to CXL_node: "<<pages_to_CI<<endl;	
	// cout<<"Number of pages in CXL_node: "<<CXI_count<<endl;
	// cout<<"size of data on CXL_node: "<<((CXI_count*4096)/1000000)<<"MB"<<endl;

	// misc_log_full<<"total memory touched in this phase: "<<(memory_touched_inMB)<<"MB"<<endl;
	// misc_log_full<<"total number of accesses in  this phase by t0: "<<total_num_accs_0<<endl;
	// misc_log_full<<"total number of accesses in  this phase by all t: "<<sumallacc<<endl;
	
	// misc_log_full<<"Baseline   migrations: "<<migrated_pages<<endl;
	// misc_log_full<<"CXLisland  migrations: "<<(migrated_pages_CI+pages_to_CI)<<"("<<migrated_pages_CI<<" not counting pages to CI)"<<endl;
	// misc_log_full<<"Migration to CXL_node: "<<pages_to_CI<<endl;
	// misc_log_full<<"Number of pages in CXL_node: "<<CXI_count<<endl;
	// misc_log_full<<"size of data on CXL_node: "<<((CXI_count*4096)/1000000)<<"MB"<<endl;

	// string phase_misc_log_name=generate_full_savefilename("phase_misc_log.txt");
	// std::ofstream phase_misc_log(phase_misc_log_name);
	// phase_misc_log<<"total memory touched in this phase: "<<(memory_touched_inMB)<<"MB"<<endl;
	// phase_misc_log<<"total number of accesses in  this phase by t0: "<<total_num_accs_0<<endl;
	// phase_misc_log<<"total number of accesses in  this phase by all t: "<<sumallacc<<endl;
	
	// phase_misc_log<<"Baseline   migrations: "<<migrated_pages<<endl;
	// phase_misc_log<<"CXLisland  migrations: "<<(migrated_pages_CI+pages_to_CI)<<"("<<migrated_pages_CI<<" not counting pages to CI)"<<endl;
	// phase_misc_log<<"Migration to CXL_node: "<<pages_to_CI<<endl;
	// phase_misc_log<<"Number of pages in CXL_node: "<<CXI_count<<endl;
	// phase_misc_log<<"size of data on CXL_node: "<<((CXI_count*4096)/1000000)<<"MB"<<endl;

	// phase_misc_log.close();


	return 0;

}

U64 gethop(U64 a, U64 b){
	if(b==CXO){
		return 3;
	}
	if(a==b){
		return 0;
	}
	U64 agrp=(a>>2);
	U64 bgrp=(b>>2);
	if(agrp==bgrp){
		return 1;
	}
	return 2;
}

bool isPowerOfTwo(unsigned int x) {
    return x && (!(x & (x - 1)));
}