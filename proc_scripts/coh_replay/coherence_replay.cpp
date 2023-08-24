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
#include <cstdlib>
#include <cassert>

#include <cmath>
#include <queue>
#include <stack>
#include <set>
#include <unordered_map>
#include <omp.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include "coherence_replay.hpp"

#define TRACESTARTINST 5000000000

using namespace std;

FILE * trace[N_THR];
//auto CCD = 
//unordered_map<uint64_t, DirectoryEntry> CCD={};


int main(){
    assert(isPowerOfTwo(NUM_SETS));
    syscycle=0;

    init_caches();

    //return 0; //for dbg
    //open traces
    for(int i=0; i<N_THR;i++){
		std::ostringstream tfname;
		tfname << "memtrace_t" << i << ".out";
		//std::cout<<tfname.str()<<std::endl;
    	trace[i] = fopen(tfname.str().c_str(), "rb");
        if(trace[i]==NULL){
			cout<<"could not open "<<tfname.str()<<endl;
		}
	}
    // print alignmnet DBG
    // log_cur_stats(0);
    // return 0;

    bool ts_6Bhit=false;
    //for(int j=0; j<100;j++){
    while(1){
        for(int i=0; i<N_THR;i++){
            uint64_t socket_index = i>>2;
            char buffer[8];
		    uint64_t buf_val=0;
            size_t readsize = read_8B_line(&buf_val, buffer, trace[i]);
            if(readsize!=8){
                cout<<"Trace Readline Failed!"<<endl;
            }
            if(buf_val==0xc0ffee){ // 1B inst phase done
				read_8B_line(&buf_val, buffer, trace[i]);
                if((buf_val>=TRACESTARTINST) && (i==2)){
                    //cout<<"timestampline : "<<buf_val<<endl;
                    log_cur_stats(buf_val);
                }
                if(buf_val==6000000000){
                    ts_6Bhit=true;
                }
            }
            else{
                U64 addr = buf_val;
			    U64 lineaddr = addr >> LINEBITS;
			    U64 rwbit = addr & 1;
			    bool isW = rwbit==1;
                U64 icount_val=0;
			    read_8B_line(&icount_val, buffer, trace[i]);
                //cout<<"ADDR     : "<<hex<<buf_val<<dec<<endl;
                //cout<<"INSCOUNT : "<<icount_val<<endl;
                access_cache(caches[socket_index], lineaddr, isW, icount_val, socket_index);
            }
        }
        if(ts_6Bhit) break;

    }
    //print touched memory:
    uint64_t touched_mem = 0;
    for(int jj=0;jj<N_SOCKETS;jj++){
        touched_mem+=CCD[jj].size();
    }
    touched_mem = touched_mem * 64;
    cout<<"Touched Mem: ";
    if(touched_mem > 1000000000){cout<<touched_mem/1000000000<<"GB"<<endl;}
    else{cout<<touched_mem/1000000<<"MB"<<endl;}
}



/*
    access_cache(caches[4], 0x7f0124124, true, 8274,0);
    std::cout<<caches[4].entries[292][23].ts<<endl;
    access_cache(caches[4], 0x7f0124124, true, 8278,1);
    access_cache(caches[4], 0x7f0124124+(NUM_SETS*64), true, 8362,2);
    std::cout<<caches[4].entries[292][23].ts<<endl;
    std::cout<<caches[4].entries[292][22].ts<<endl;
    std::cout<<"direcotry[292] size: "<<CCD[292].size()<<endl;
    std::cout<<"direcotry[295] size: "<<CCD[295].size()<<endl;

    log_cur_stats();
*/