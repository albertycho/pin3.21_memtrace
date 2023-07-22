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
// enum class DState {
//     I, 
//     S,
// 	E,
// 	M,
//     // add more states if required
// };

typedef coh_state_t DState;

struct DirectoryEntry {
    DState state=I;
    uint64_t owner;
    //uint64_t sharers;
	bool sharers[N_SOCKETS];
	//DirectoryEntry(): state(I),owner(0),sharers({0}){}
};
typedef struct sim_stats {
    uint64_t m_reads;                 // read requests
    uint64_t m_writes;                // write requests
	uint64_t m_writes_from_coh;       // write due to M->S
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
std::ofstream output_log("coh_repl12M_log_dbg.txt");


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

void update_directory(uint64_t socket_id, uint64_t lineaddr, uint64_t set_index, bool isW){
	DirectoryEntry &de = CCD[set_index][lineaddr];

	
	if(set_index==5913 && lineaddr==2182637451033){
		cout<<"requestor "<<socket_id<<", owner: "<<de.owner<<", ";
		if(isW){cout<<"Write, ";}
		else{cout<<"Read, ";}
		cout<<"directory state ";
		switch(de.state){
			case I:
				cout<<"I"<<endl;
				break;
			case S:
				cout<<"S"<<endl;
				break;
			case E:
				cout<<"E"<<endl;
				break;
			case M:
				cout<<"M"<<endl;
				break;
			default:
				assert(0); //code should not get here
		}
	}

	// cout<<"directory state ";
	// switch(de.state){
	// 	case I:
	// 		cout<<"I"<<endl;
	// 		break;
	// 	case S:
	// 		cout<<"S"<<endl;
	// 		break;
	// 	case E:
	// 		cout<<"E"<<endl;
	// 		break;
	// 	case M:
	// 		cout<<"M"<<endl;
	// 		break;
	// 	default:
	// 		assert(0); //code should not get here
	// }

	if(!isW){ //READS
		switch(de.state){
			case I:
				//memrd++
				//change state to E
				simstats.m_reads++;
				de.state=E;
				break;
			case S:
				//do nothing (set sharers[socket_id], which is done at the end)
				//if sharers[socket_id] was not set previously, memrd++
				if(de.sharers[socket_id]==false){simstats.m_reads++;}
				break;
			case E:
				// if owner is someone else, change to S, and change state of previous owner to S
				//	TODO: c_blocktransfer ++
				if(de.owner!=socket_id){
					assert(de.sharers[socket_id]==false);
					de.state=S;
					for(int jj=0; jj<NUM_WAYS;jj++){
						if(caches[de.owner].entries[set_index][jj].tag==lineaddr){
							assert(caches[de.owner].entries[set_index][jj].cstate==E);
							assert(caches[de.owner].entries[set_index][jj].dirty==false);
							caches[de.owner].entries[set_index][jj].cstate=S;
							break;
						}
					}

					simstats.c_block_trans++;
				}

				// if owner is self, do nothing
				break;
			case M:
				// if owner is someone else, change to S, 
				//		and change state of previous owner to S(and unset dirtybit). mem_wr++
				//		c_blocktransfer++

				if(de.owner!=socket_id){
					//DBG
					if(set_index==5913 && lineaddr==2182637451033){
						cout<<"downgrading debugging line from M to S"<<endl;
					}
					assert(de.sharers[socket_id]==false);
					de.state=S;
					for(int jj=0; jj<NUM_WAYS;jj++){
						if(caches[de.owner].entries[set_index][jj].tag==lineaddr){
							assert(caches[de.owner].entries[set_index][jj].cstate==M);
							assert(caches[de.owner].entries[set_index][jj].dirty);
							caches[de.owner].entries[set_index][jj].cstate=S;
							caches[de.owner].entries[set_index][jj].dirty=false;
							break;
						}
					}
					simstats.m_writes_from_coh++;
					simstats.c_block_trans++;
				}
				//if onwer is self, do nothing
				break;
			default:
				assert(0); //code should not get here
		}
	}
	else{ // WRITES
		switch(de.state){
			case I:
				//mem_rd++
				simstats.m_reads++;
				break;
			case S:
				// send inval. go and invalidate entries in sharers (unset valid, dirty)
				// 		set directory state to M
				// if cur socket_id is not a sharer, mem_rd++
				simstats.c_invals++;
				if(de.sharers[socket_id]==false){simstats.m_reads++;}
				for(uint64_t jj=0; jj<N_SOCKETS;jj++){
					if(jj!=socket_id){
						if(de.sharers[jj]){
							de.sharers[jj]=false;
							for(int ii=0;ii<NUM_WAYS;ii++){
								if(caches[jj].entries[set_index][ii].tag==lineaddr){
									caches[jj].entries[set_index][ii].cstate=I;
									caches[jj].entries[set_index][ii].valid=false;
									caches[jj].entries[set_index][ii].dirty=false;
									break;
								}							
							}
						}
					}
				}
				break;
			case E:
				// if owner is someone else, and change state of previous owner to I
				//		blocktransfer++
				// if owner is self, just set directory state to M
				if(de.owner!=socket_id){
					assert(de.sharers[de.owner]);
					assert(de.sharers[socket_id]==false);
					de.sharers[de.owner]=false;
					for(int ii=0;ii<NUM_WAYS;ii++){
						if(caches[de.owner].entries[set_index][ii].tag==lineaddr){
							assert(caches[de.owner].entries[set_index][ii].cstate==E);
							assert(caches[de.owner].entries[set_index][ii].dirty==false);
							caches[de.owner].entries[set_index][ii].cstate=I;
							caches[de.owner].entries[set_index][ii].valid=false;
							caches[de.owner].entries[set_index][ii].dirty=false;
							break;
						}							
					}
					simstats.c_block_trans++;					
				}
				break;
			case M:
				// if owner is someone else, change state of previous owner to I. 
				//		block transfer++
				//if onwer is self, do nothing
				if(de.owner!=socket_id){
					assert(de.sharers[de.owner]);
					assert(de.sharers[socket_id]==false);
					de.sharers[de.owner]=false;
					for(int ii=0;ii<NUM_WAYS;ii++){
						if(caches[de.owner].entries[set_index][ii].tag==lineaddr){
							assert(caches[de.owner].entries[set_index][ii].cstate==M);
							assert(caches[de.owner].entries[set_index][ii].dirty);
							caches[de.owner].entries[set_index][ii].cstate=I;
							caches[de.owner].entries[set_index][ii].valid=false;
							caches[de.owner].entries[set_index][ii].dirty=false;
							break;
						}							
					}
					simstats.c_block_trans++;					
				}

				break;
			default:
				assert(0); //code should not get here
		}
		de.state=M;

	}
	//this is done for all cases
	de.sharers[socket_id]=true;
	de.owner=socket_id; //this isn't necessary for all cases, but is also not incorrect, and makes coding easier
	
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
	

    if(ishit){
		if(set_index==5913 && lineaddr==2182637451033){
			cout<<"is hit on debugging line, requestor: "<<socketid<<endl;
		}
		//std::cout<<"ishit"<<endl;
		simstats.llc_hits++;
        if(isW){
            cach.entries[set_index][hit_i].dirty=true;
			cach.entries[set_index][hit_i].cstate=M;			            
        }
        cach.entries[set_index][hit_i].ts=ts;
		update_directory(socketid, lineaddr, set_index, isW);
        return 0;
    }
	if(set_index==5913 && lineaddr==2182637451033){
		cout<<"cache miss on debugging line, requestor: "<<socketid<<endl;
	}

	simstats.llc_misses++;

    if(i_entry_found==false){ //need to evict
        insert_i = lru_i;
        //TODO handle evicted line directory/coherence:
        // if state is E or M --> update directory state to I
        // if state is M --> m_write++;

		uint64_t evicted_lineaddr = cach.entries[set_index][lru_i].tag;
		DirectoryEntry &ede = CCD[set_index][evicted_lineaddr];

		//DBG
		if(set_index==5913 && evicted_lineaddr==2182637451033){
			cout<<"current cache socket: "<<socketid<<endl;
			cout<<"evicted cache way index: "<<lru_i<<endl;
			cout<<"evicted line owner: "<<ede.owner<<endl;
			cout<<"evicted line state: ";
			switch(ede.state){
				case I:
					cout<<"I"<<endl;
					break;
				case S:
					cout<<"S"<<endl;
					break;
				case E:
					cout<<"E"<<endl;
					break;
				case M:
					cout<<"M"<<endl;
					break;
				default:
					assert(0); //code should not get here
			}
		}

		if(cach.entries[set_index][lru_i].dirty==true){
			//incrmenet mem_wr, set directory state to I, unset all sharers
			//dbg
			assert(cach.entries[set_index][lru_i].cstate==M);
			if(ede.state!=M){
				cout<<"\nevicted line Directory state: "<<ede.state<<endl;
				cout<<"evicted line cache cstate: "<<cach.entries[set_index][lru_i].cstate<<endl;
				cout<<"evicted lineaddr: "<<evicted_lineaddr<<", set_index: "<<set_index<<endl;
				cout<<"evicted way index: "<<lru_i<<", access_count: "<<simstats.total_m_insts<<endl;
			}
			assert(ede.state==M);
			assert(ede.owner==socketid);

			simstats.m_writes++;
			ede.sharers[socketid]=false;
			ede.state=I;
		}
		else{
			//if E, and owner is self, set invalid
			if(ede.state==E){
				//dbg
				assert(ede.owner==socketid);
				assert(cach.entries[set_index][lru_i].cstate==E);
				ede.sharers[socketid]=false;
				ede.state=I;
			}
			else{
				//dbg
				assert(ede.state==S);
				ede.sharers[socketid]=false;
				uint64_t numsharers=0;
				for(uint32_t jj=0; jj<N_SOCKETS;jj++){
					if(ede.sharers[jj]){
						numsharers++;
						if(jj!=socketid){ede.owner=jj;}
					}
				}
				assert(ede.owner!=socketid); // should have at least one other node if in S
				if(numsharers==1){
					ede.state=E;
					for(int ii=0; ii<NUM_WAYS;ii++){
						if(caches[ede.owner].entries[set_index][ii].tag==evicted_lineaddr){
							caches[ede.owner].entries[set_index][ii].cstate=E;
							break;
						}
					}					
				}
			}
		}

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
	update_directory(socketid, lineaddr, set_index, isW);
	if(CCD[set_index][lineaddr].state==S){
		assert(!isW);
		cach.entries[set_index][insert_i].cstate=S;
	}

	if(CCD[set_index][lineaddr].state != cach.entries[set_index][insert_i].cstate){
		cout<<"CCD state: "<<CCD[set_index][lineaddr].state<<", cache cstate: "<<cach.entries[set_index][insert_i].cstate<<endl;
		cout<<"AccessID: "<<simstats.total_m_insts<<endl;
		cout<<"set_index: "<<set_index<<", lineaddr: "<<lineaddr<<endl;
		if(isW){cout<<"Write"<<endl;}
		else{cout<<"Read"<<endl;}
	}
	assert(CCD[set_index][lineaddr].state == cach.entries[set_index][insert_i].cstate);

	//dbg
	if(cach.entries[set_index][insert_i].cstate==S) assert(cach.entries[set_index][insert_i].dirty==false);
	if(socketid==10 && set_index==5913 && lineaddr==2182637451033){
		cout<<"at access done for dubbing line, cstate: "<<cach.entries[set_index][insert_i].cstate<<", dirty: "<<cach.entries[set_index][insert_i].dirty;
		cout<<", way index: "<<insert_i<<", access_count"<<simstats.total_m_insts<<endl;
	}
	if(caches[10].entries[5913][22].cstate==M){
		cout<<"WHEN debugging line cstate flipped, accessCount: "<<simstats.total_m_insts<<endl;
		if(caches[10].entries[5913][22].tag==2182637451033) cout<<"tag matches 2182637451033"<<endl;
		else cout<<"tag DOES NOT MATCH 2182637451033"<<endl;
		cout<<"socketid: "<<socketid<<", set_index: "<<set_index<<", lineaddr: "<<lineaddr<<endl;
	}

    return 0;
}


int read_8B_line(uint64_t * buf_val, char* buffer, FILE* fptr){
	size_t readsize = std::fread(buffer, sizeof(char), sizeof(buffer), fptr);
	if(readsize!=8) return -1;
	//dunno why but this step needed for things t work
	std::memcpy(buf_val, buffer, sizeof(U64)); 
	return readsize;
}

void log_cur_stats(uint64_t inscount){
	cout<<"Inst "<<inscount<<endl;
	cout<<"Total Access		:"<<simstats.total_m_insts<<endl;
	cout<<"LLC hits__		:"<<simstats.llc_hits<<endl;
	cout<<"LLC misses		:"<<simstats.llc_misses<<endl;
	cout<<"Mem Reads		:"<<simstats.m_reads<<endl;
	cout<<"Mem Writes		:"<<simstats.m_writes<<endl;
	cout<<"Coherence Invals	:"<<simstats.c_invals<<endl;
	cout<<"C Block Transfer	:"<<simstats.c_block_trans<<endl;
	cout<<endl;

	output_log<<"Inst "<<inscount<<endl;
	output_log<<"Total Access		:"<<simstats.total_m_insts<<endl;
	output_log<<"LLC hits__		:"<<simstats.llc_hits<<endl;
	output_log<<"LLC misses		:"<<simstats.llc_misses<<endl;
	output_log<<"Mem Reads		:"<<simstats.m_reads<<endl;
	output_log<<"Mem Writes		:"<<simstats.m_writes<<endl;
	output_log<<"Coherence Invals	:"<<simstats.c_invals<<endl;
	output_log<<"C Block Transfer	:"<<simstats.c_block_trans<<endl;
	output_log<<endl;
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