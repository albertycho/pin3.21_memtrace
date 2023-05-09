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

#define PAGESIZE 4096
#define PAGEBITS 12
#define N_THR 16
#define THR_OFFSET_VAL 2
#define N_THR_OFFSET (N_THR+1)
#define U64 uint64_t

using namespace std;

FILE * trace[N_THR];

vector<unordered_map<uint64_t, uint64_t>> page_access_counts_dummy;
// vector<unordered_map<uint64_t, uint64_t>> page_access_counts_R(N_THR);
// vector<unordered_map<uint64_t, uint64_t>> page_access_counts_W(N_THR);
//unordered_map<uint64_t, uint64_t> page_sharers;
//unordered_map<uint64_t, uint64_t> page_Rs;
//unordered_map<uint64_t, uint64_t> page_Ws;

int addMap(std::vector<std::unordered_map<uint64_t, uint64_t>>& inunordered_map) {
    // Create a unordered_map
    std::unordered_map<uint64_t, uint64_t> new_unordered_map;
    
    // Add some data to the unordered_map
    new_unordered_map[1] = 2;
	new_unordered_map[5] = 999;

    // Add the unordered_map to the vector
    inunordered_map.push_back(new_unordered_map);

	return 0;
}
int addMap2(){
    // Create a unordered_map
    std::unordered_map<uint64_t, uint64_t> new_unordered_map;
    
    // Add some data to the unordered_map
    new_unordered_map[1] = 2;
	new_unordered_map[5] = 999;

    // Add the unordered_map to the vector
    page_access_counts_dummy.push_back(new_unordered_map);

	return 0;
}

int read_8B_line(uint64_t * buf_val, char* buffer, FILE* fptr){
	size_t readsize = std::fread(buffer, sizeof(char), sizeof(buffer), fptr);
	if(readsize!=8) return -1;
	//dunno why but this step needed for things t work
	std::memcpy(buf_val, buffer, sizeof(U64)); 
	return readsize;
}

int savearray(U64 * arr, U64 arrsize, string savefilename){
	std::cout<<savefilename<<endl;
	ofstream outf(savefilename);

	for(U64 i=0; i<arrsize;i++){
		//std::cout<<arr[i]<<endl;
		outf<<arr[i]<<endl;
	}
	outf.close();
	return 0;
}

int save2Darr(U64 arr[][N_THR_OFFSET], U64 arrsize, string savefilename){
	std::ofstream file(savefilename);
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

int process_phase(){
	//page accesses
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts;
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts_R;
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts_W;
	
	//list of all unique pages and their sharer count
	unordered_map<uint64_t, uint64_t> page_sharers;
	//list of all unique pags and their R/W count
	unordered_map<uint64_t, uint64_t> page_Rs;
	unordered_map<uint64_t, uint64_t> page_Ws;

	//Histograms
	uint64_t hist_access_sharers[N_THR_OFFSET]={0};
	uint64_t hist_access_sharers_R[N_THR_OFFSET]={0};
	uint64_t hist_access_sharers_R_to_RWP[N_THR_OFFSET]={0};
	uint64_t hist_access_sharers_W[N_THR_OFFSET]={0};
	uint64_t hist_page_sharers[N_THR_OFFSET]={0};
	uint64_t hist_page_sharers_R[N_THR_OFFSET]={0};
	uint64_t hist_page_sharers_W[N_THR_OFFSET]={0};
	uint64_t hist_page_shareres_nacc[10][N_THR_OFFSET]={0};


	char buffer[8];
	uint64_t buf_val;

	U64 total_num_accs=0;
	for (int i=0; i<N_THR;i++){
	//for (int i=0; i<1;i++){ // for unit testing
		cout<<"processing thread "<<i<<endl;
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		// Read Trace and get page access counts
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		unordered_map<uint64_t, uint64_t> pa_count;
		unordered_map<uint64_t, uint64_t> pa_count_R;
		unordered_map<uint64_t, uint64_t> pa_count_W;
		//trace[i]->read(buffer, sizeof(buffer));
		//size_t readsize = std::fread(buffer, sizeof(char), sizeof(buffer), trace[i]);
		//std::memcpy(&buf_val, buffer, sizeof(buf_val));

		size_t readsize = read_8B_line(&buf_val, buffer, trace[i]);
		while(readsize==8){
			if(buf_val==0xc0ffee){ // 1B inst phase done
				cout<<"coffee read in buf_val"<<endl;
				read_8B_line(&buf_val, buffer, trace[i]);
				cout<<"inst count: "<<buf_val<<endl;
				//TODO - check(assertion) if end of phase inst  count checks out

				break;
			}
			total_num_accs++;

			//parse page and RW
			U64 addr = buf_val;
			U64 page = addr >> PAGEBITS;
			U64 rwbit = addr & 1;
			bool isW = rwbit==1;

			//dbg during dev. TODO remove
			// cout<<page<<" "<<isW<<endl;
			// iiii++;
			// if(iiii==100) return 0;

			// add to page access count
			auto pa_it = pa_count.find(page);
			if(pa_it==pa_count.end()){ //didn't  find
				pa_count.insert({page,1});
				if(isW) {
					pa_count_W.insert({page,1});
					pa_count_R.insert({page,0});
				}
				else{
					pa_count_R.insert({page,1});
					pa_count_W.insert({page,0});
				}
			}
			else{
				pa_it->second=(pa_it->second)+1;
				if(isW) pa_count_W[page]=pa_count_W[page]+1;
				else pa_count_R[page]=pa_count_R[page]+1;
				//pa_count[page]=pa_count[page+1];
			}

			//update page sharers if necessary
			auto ps_it = page_sharers.find(page);
			if(ps_it==page_sharers.end()){ 
				// idk if this whole thing is necessary. will see if code works wihtout
				page_sharers[page]=0; 
				page_Rs[page]=0;
				page_Ws[page]=0;
			}

			//increment R and Ws
			if(isW){
				page_Ws[page]=page_Ws[page]+1;
			}
			else{
				page_Rs[page]=page_Rs[page]+1;
			}
			
			readsize = read_8B_line(&buf_val, buffer, trace[i]);
		}
		page_access_counts.push_back(pa_count);
		page_access_counts_W.push_back(pa_count_W);
		page_access_counts_R.push_back(pa_count_R);
	}
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// log sharers for each page  
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	for (const auto& pa_c : page_access_counts) {
		for (const auto& ppair : pa_c) {
			U64 page = ppair.first;
			page_sharers[page]=page_sharers[page]+1;
			assert(page_sharers[page] < N_THR+1);
		}
	}

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// Populate access sharer histogram
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	U64 pac_size = page_access_counts.size();
	//for (const auto& pa_c : page_access_counts) {
	for(U64 i=0; i<pac_size;i++){
		//unordered_map<uint64_t, uint64_t> pa_c = page_access_counts[i];
		auto pac   = page_access_counts[i];
		auto pac_W = page_access_counts_W[i];
		auto pac_R = page_access_counts_R[i];
		for (const auto& ppair : pac) {
			U64 page = ppair.first;
			U64 accs = ppair.second;
			U64 rds= pac_R[page];
			U64 wrs= pac_W[page];
			U64 sharers = page_sharers[page];
			hist_access_sharers[sharers]=hist_access_sharers[sharers]+accs;
			hist_access_sharers_W[sharers]=hist_access_sharers_W[sharers]+wrs;
			if(page_Ws[page]!=0){
				hist_access_sharers_R_to_RWP[sharers]=hist_access_sharers_R_to_RWP[sharers]+rds;
			}
			else{
				hist_access_sharers_R[sharers]=hist_access_sharers_R[sharers]+rds;
			}
		}
	}	


	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// Populate page sharer histogram
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	for (const auto& pss : page_sharers) {
		U64 sharers = pss.second;
		U64 page = pss.first;
		hist_page_sharers[sharers]=hist_page_sharers[sharers]+1;
		if(page_Ws[page]!=0){
			hist_page_sharers_W[sharers]=hist_page_sharers_W[sharers]+1;
		}
		else{
			hist_page_sharers_R[sharers]=hist_page_sharers_R[sharers]+1;
		}
		U64 access_count = page_Rs[page] + page_Ws[page];
		U64 log_ac = static_cast<uint64_t>(round(log2(access_count)));
		if(log_ac>9) log_ac=9;
		hist_page_shareres_nacc[log_ac][sharers]=hist_page_shareres_nacc[log_ac][sharers]+1;
	}

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// Populate hop histogram
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// Reassign owners
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


	U64 total_pages = page_sharers.size();
	U64 memory_touched = total_pages*PAGESIZE;
	cout<<"total memory touched in this phase: "<<memory_touched<<endl;
	cout<<"total number of accesses in  this phase: "<<total_num_accs<<endl;

	char namebuf[50];
	//namebuf="myarray\0";
	savearray(hist_access_sharers, N_THR_OFFSET,"access_hist.txt\0");
	savearray(hist_access_sharers_W, N_THR_OFFSET,"access_hist_W.txt\0");
	savearray(hist_access_sharers_R_to_RWP, N_THR_OFFSET,"access_hist_R_to_RWP.txt\0");
	savearray(hist_access_sharers_R, N_THR_OFFSET,"access_hist_R.txt\0");
	savearray(hist_page_sharers, N_THR_OFFSET,"page_hist.txt\0");
	savearray(hist_page_sharers_W, N_THR_OFFSET,"page_hist_W.txt\0");
	savearray(hist_page_sharers_R, N_THR_OFFSET,"page_hist_R.txt\0");
	save2Darr(hist_page_shareres_nacc, N_THR_OFFSET, "page_hist_nacc.txt\0");

	return 0;
}

int expmain(){
//int main(){
	cout<<"calling addunordered_map"<<endl;
	//addMap(page_access_counts);
	addMap2();
	cout<<"size of page_access_counts: "<<page_access_counts_dummy.size()<<endl;

	int i=0;
    for(const auto &unordered_map : page_access_counts_dummy) {
		cout<<"unordered_map"<<i<<endl;
		i++;
        for(const auto &pair : unordered_map) {
            std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
        }
    }

	return 0;
}


int main(){

	for(int i=0; i<N_THR;i++){
		std::ostringstream tfname;
		tfname << "memtrace_t" << (i+THR_OFFSET_VAL) << ".out";
		std::cout<<tfname.str()<<std::endl;
    	trace[i] = fopen(tfname.str().c_str(), "rb");
	}

	//return expmain();
	process_phase();
	for(int i=0; i<N_THR;i++){
		fclose(trace[i]);
	}
	return 0;

}