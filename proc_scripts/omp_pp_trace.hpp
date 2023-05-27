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

#define PAGESIZE 4096
#define PAGEBITS 12
#define N_THR 16
//#define THR_OFFSET_VAL 2
#define THR_OFFSET_VAL 0
#define N_THR_OFFSET (N_THR+1)
#define U64 uint64_t
#define CXO 100 //CXL Island is owner
#define SHARER_THRESHOLD 8
#define INVAL_OWNER 9999
#define PHASE_CYCLES 1000000000

using namespace std;

U64 curphase;

struct migration_compare{
	bool operator()(const std::pair<uint64_t, uint64_t>& a, const std::pair<uint64_t, uint64_t>& b) const {
        return a.second > b.second;
	}
};

int read_8B_line(uint64_t * buf_val, char* buffer, FILE* fptr){
	size_t readsize = std::fread(buffer, sizeof(char), sizeof(buffer), fptr);
	if(readsize!=8) return -1;
	//dunno why but this step needed for things t work
	std::memcpy(buf_val, buffer, sizeof(U64)); 
	return readsize;
}

string generate_phasedirname(){
	stringstream ss;
	ss<<"1BPhase"<<curphase;
	string dir_name=ss.str();
	return dir_name;
}
string generate_full_savefilename(string savefilename){
	string dir_name = generate_phasedirname();
	stringstream ss;
	ss<<dir_name<<"/"<<savefilename;
	string savefilename_full=ss.str();
	return savefilename_full;
}

int log_misc_stats(U64 memory_touched_inMB, U64 total_num_accs_0, U64 sumallacc, U64 migrated_pages,
	U64 migrated_pages_CI, U64 pages_to_CI, ofstream& misc_log_full){
	cout<<"total memory touched in this phase: "<<(memory_touched_inMB)<<"MB"<<endl;
	cout<<"total number of accesses in  this phase by t0: "<<total_num_accs_0<<endl;
	cout<<"total number of accesses in  this phase by all t: "<<sumallacc<<endl;
	
	cout<<"Baseline   migrations: "<<migrated_pages<<endl;
	cout<<"CXLisland  migrations: "<<(migrated_pages_CI+pages_to_CI)<<"("<<migrated_pages_CI<<" not counting pages to CI)"<<endl;
	cout<<"Migration to CXL_node: "<<pages_to_CI<<endl;	

	misc_log_full<<"total memory touched in this phase: "<<(memory_touched_inMB)<<"MB"<<endl;
	misc_log_full<<"total number of accesses in  this phase by t0: "<<total_num_accs_0<<endl;
	misc_log_full<<"total number of accesses in  this phase by all t: "<<sumallacc<<endl;
	
	misc_log_full<<"Baseline   migrations: "<<migrated_pages<<endl;
	misc_log_full<<"CXLisland  migrations: "<<(migrated_pages_CI+pages_to_CI)<<"("<<migrated_pages_CI<<" not counting pages to CI)"<<endl;
	misc_log_full<<"Migration to CXL_node: "<<pages_to_CI<<endl;
	

	string phase_misc_log_name=generate_full_savefilename("phase_misc_log.txt");
	std::ofstream phase_misc_log(phase_misc_log_name);
	phase_misc_log<<"total memory touched in this phase: "<<(memory_touched_inMB)<<"MB"<<endl;
	phase_misc_log<<"total number of accesses in  this phase by t0: "<<total_num_accs_0<<endl;
	phase_misc_log<<"total number of accesses in  this phase by all t: "<<sumallacc<<endl;
	
	phase_misc_log<<"Baseline   migrations: "<<migrated_pages<<endl;
	phase_misc_log<<"CXLisland  migrations: "<<(migrated_pages_CI+pages_to_CI)<<"("<<migrated_pages_CI<<" not counting pages to CI)"<<endl;
	phase_misc_log<<"Migration to CXL_node: "<<pages_to_CI<<endl;

	phase_misc_log.close();


	return 0;

}

int savearray(U64 * arr, U64 arrsize, string savefilename){
	// std::cout<<savefilename<<endl;
	// stringstream ss;
	// ss<<"1BPhase"<<curphase<<"/"<<savefilename;
	//string savefilename_full=ss.str();
	string savefilename_full=generate_full_savefilename(savefilename);
	ofstream outf(savefilename_full);

	for(U64 i=0; i<arrsize;i++){
		//std::cout<<arr[i]<<endl;
		outf<<arr[i]<<endl;
	}
	outf.close();
	return 0;
}

int save2Darr(U64 arr[][N_THR_OFFSET], U64 arrsize, string savefilename){
	string savefilename_full=generate_full_savefilename(savefilename);
	std::ofstream file(savefilename_full);
	//cout<<"does code get here 88"<<endl;
    if (file.is_open()) {
        for (size_t j=0; j<10;j++) {
			//cout<<"j: "<<j<<endl;
            for (size_t i = 0; i < arrsize; ++i) {
				//cout<<"i: "<<i<<endl;
                file << arr[j][i];
                if (i != arrsize - 1)
                    file << ",";
            }
            file << "\n";
        }
    }

    file.close();
	return 0;
}

int save_hophist(U64 arr[][4], U64 arrsize, string savefilename){
	//std::ofstream file(savefilename);
	string savefilename_full=generate_full_savefilename(savefilename);
	std::ofstream file(savefilename_full);
	//cout<<"does code get here 88"<<endl;
    if (file.is_open()) {
        for (size_t j=0; j<arrsize;j++) {
			//cout<<"j: "<<j<<endl;
            for (size_t i = 0; i < 4; ++i) {
				//cout<<"i: "<<i<<endl;
                file << arr[j][i];
                if (i != 4 - 1)
                    file << ",";
            }
            file << "\n";
        }
    }

    file.close();
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
