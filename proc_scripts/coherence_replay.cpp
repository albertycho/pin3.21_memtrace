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
#include "omp_pp_trace.hpp"


using namespace std;

FILE * trace[N_THR];



int main(){

    //open traces
    for(int i=0; i<N_THR;i++){
		std::ostringstream tfname;
		tfname << "memtrace_t" << i << ".out";
		std::cout<<tfname.str()<<std::endl;
    	trace[i] = fopen(tfname.str().c_str(), "rb");
        if(trace[i]==NULL){
			cout<<"could not open "<<tfname.str()<<endl;
		}
	}

    for(int i=0; i<10;i++){
        for(int i=0; i<N_THR;i++){
            char buffer[8];
		    uint64_t buf_val;
            size_t readsize = read_8B_line(&buf_val, buffer, trace[i]);
            if(buf_val==0xc0ffee){ // 1B inst phase done
				read_8B_line(&buf_val, buffer, trace[i]);
                cout<<"timestampline: "<<buf_val<<endl;
            }
        }

    }
}