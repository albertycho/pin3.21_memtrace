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
#include "omp_pp_trace.hpp"

// #define PAGESIZE 4096
// #define PAGEBITS 12
// #define N_THR 16
// //#define THR_OFFSET_VAL 2
// #define THR_OFFSET_VAL 0
// #define N_THR_OFFSET (N_THR+1)
// #define U64 uint64_t
// #define CXO 100 //CXL Island is owner
// #define SHARER_THRESHOLD 8
// #define INVAL_OWNER 9999
// #define PHASE_CYCLES 1000000000

#define LIMIT_MIGRATION 1
#define MIGRATION_LIMIT 32768

using namespace std;

FILE * trace[N_THR];

omp_lock_t page_W_lock;
omp_lock_t page_R_lock;
omp_lock_t page_owner_lock;
omp_lock_t page_owner_CI_lock;
omp_lock_t hop_hist_lock;
omp_lock_t hop_hist_CI_lock;

vector<unordered_map<uint64_t, uint64_t>> page_access_counts_dummy;
// vector<unordered_map<uint64_t, uint64_t>> page_access_counts_R(N_THR);
// vector<unordered_map<uint64_t, uint64_t>> page_access_counts_W(N_THR);
//unordered_map<uint64_t, uint64_t> page_sharers;
//unordered_map<uint64_t, uint64_t> page_Rs;
//unordered_map<uint64_t, uint64_t> page_Ws;
unordered_map<uint64_t, uint64_t> page_owner;
unordered_map<uint64_t, uint64_t> page_owner_CI;

//U64 curphase=0;
U64 phase_end_cycle=0;
bool any_trace_done=false;
std::ofstream misc_log_full("misc_log_full.txt");

uint64_t skipinsts=5000000000;
uint64_t phase_to_dump_pagemapping=0;
uint64_t num_phases_to_dump_pagemapping=2;

int process_phase(){
	cout<<"starting phase "<<curphase<<endl;
	misc_log_full<<"starting phase "<<curphase<<endl;
	phase_end_cycle=phase_end_cycle+PHASE_CYCLES;
	//page accesses
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts={};
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts_R={};
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts_W={};
	
	//list of all unique pages and their sharer count
	unordered_map<uint64_t, uint64_t> page_sharers={};
	//list of all unique pags and their R/W count
	unordered_map<uint64_t, uint64_t> page_Rs={};
	unordered_map<uint64_t, uint64_t> page_Ws={};

	std::multiset<std::pair<uint64_t, uint64_t>, migration_compare> sorted_candidates;

	// list of links (track traffic per link)
	U64 link_traffic_R[N_THR][N_THR]={0};
	U64 link_traffic_W[N_THR][N_THR]={0};
	U64 link_traffic_R_CI[N_THR][N_THR]={0};
	U64 link_traffic_W_CI[N_THR][N_THR]={0};
	U64	CI_traffic_R[N_THR]={0};
	U64	CI_traffic_W[N_THR]={0};

	// traffic at memory controller on each ndoe
	U64 mem_traffic[N_THR]={0};
	U64 mem_traffic_CI[N_THR]={0};
	
	U64 migrated_pages=0;
	U64 migrated_pages_CI=0;
	U64 pages_to_CI=0;

	//Histograms
	uint64_t hist_access_sharers[N_THR_OFFSET]={0};
	uint64_t hist_access_sharers_R[N_THR_OFFSET]={0};
	uint64_t hist_access_sharers_R_to_RWP[N_THR_OFFSET]={0};
	uint64_t hist_access_sharers_W[N_THR_OFFSET]={0};
	uint64_t hist_page_sharers[N_THR_OFFSET]={0};
	uint64_t hist_page_sharers_R[N_THR_OFFSET]={0};
	uint64_t hist_page_sharers_W[N_THR_OFFSET]={0};
	uint64_t hist_page_shareres_nacc[10][N_THR_OFFSET]={0};

	uint64_t hop_hist_W[N_THR_OFFSET][4]={0};
	uint64_t hop_hist_RtoRW[N_THR_OFFSET][4]={0};
	uint64_t hop_hist_RO[N_THR_OFFSET][4]={0};
	uint64_t hop_hist_W_CI[N_THR_OFFSET][4]={0};
	uint64_t hop_hist_RtoRW_CI[N_THR_OFFSET][4]={0};
	uint64_t hop_hist_RO_CI[N_THR_OFFSET][4]={0};

	U64 total_num_accs[N_THR]={0};

	#pragma omp parallel for
	for (int i=0; i<N_THR;i++){
		//U64 nompt=omp_get_num_threads();
		//cout<<"omp threads: "<<nompt<<endl;
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		// Read Trace and get page access counts
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		U64 tmp_numacc=0;

		unordered_map<uint64_t, uint64_t> pa_count;
		unordered_map<uint64_t, uint64_t> pa_count_R;
		unordered_map<uint64_t, uint64_t> pa_count_W;
		unordered_map<uint64_t, uint64_t> page_Rs_tmp;
		unordered_map<uint64_t, uint64_t> page_Ws_tmp;

		char buffer[8];
		uint64_t buf_val;
		size_t readsize = read_8B_line(&buf_val, buffer, trace[i]);
		while(readsize==8){
			if(buf_val==0xc0ffee){ // 1B inst phase done
				read_8B_line(&buf_val, buffer, trace[i]);
				if(buf_val>=phase_end_cycle){
					//cout<<"inst count: "<<buf_val<<endl;
					break;
				}
				else{
					readsize = read_8B_line(&buf_val, buffer, trace[i]);
					continue;
				}
			}
			tmp_numacc++;

			//parse page and RW
			U64 addr = buf_val;
			U64 page = addr >> PAGEBITS;
			U64 rwbit = addr & 1;
			bool isW = rwbit==1;

			//not doing anything with ins count in this script for now
			//just read and discard
			U64 icount_val;
			read_8B_line(&icount_val, buffer, trace[i]);
			//cout<<"page: "<<page<<" icount: "<<icount_val<<endl;


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


			//increment R and Ws
			if(isW){
				//omp_set_lock(&page_W_lock);
				page_Ws_tmp[page]=page_Ws_tmp[page]+1;
				//omp_unset_lock(&page_W_lock);
			}
			else{
				//omp_set_lock(&page_R_lock);
				page_Rs_tmp[page]=page_Rs_tmp[page]+1;
				//omp_unset_lock(&page_R_lock);
			}

			
			readsize = read_8B_line(&buf_val, buffer, trace[i]);
		}
		if(readsize!=8){
			any_trace_done=true;
		}

		/////////coalescing local stats into one
		total_num_accs[i]+=tmp_numacc;
		#pragma omp critical
		{
			page_access_counts.push_back(pa_count);
			page_access_counts_W.push_back(pa_count_W);
			page_access_counts_R.push_back(pa_count_R);
			misc_log_full<<"t_"<<i<<" accesses this phase: "<<tmp_numacc<<endl;
			
		}
		for (const auto& pw : page_Ws_tmp) {
			U64 page = pw.first;
			U64 tmp_accs = pw.second;
			omp_set_lock(&page_W_lock);
			page_Ws[page]=page_Ws[page]+tmp_accs;
			omp_unset_lock(&page_W_lock);
		}
		for (const auto& pw : page_Rs_tmp) {
			U64 page = pw.first;
			U64 tmp_accs = pw.second;
			omp_set_lock(&page_R_lock);
			page_Rs[page]=page_Rs[page]+tmp_accs;
			omp_unset_lock(&page_R_lock);
		}

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
	// sort pages in order of accesses in sorted_candidates
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//cout<<"page_Rs size: "<<page_Rs.size()<<endl;
	for (const auto& ppair : page_Rs){
		U64 page = ppair.first;
		U64 accs = ppair.second + page_Ws[page];
		//TODO can probably skip pages that have low accs, like < 100
		if(accs>100){
			sorted_candidates.insert({page, accs});
		}
	}
	//cout<<"sorted candidates size: "<<sorted_candidates.size()<<endl;


	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// Populate access sharer histogram
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	U64 pac_size = page_access_counts.size();
	assert(pac_size==N_THR);
	for(U64 i=0; i<pac_size;i++){
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
	
			//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			// Populate hop histogram
			//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			U64 owner=i;
			auto pp_it = page_owner.find(page);
			if(pp_it==page_owner.end()){ //new page first touch
				// this will favor thread 0. 
				// should be ok after the first phase..
				page_owner[page]=i; 
				owner=i;
			}
			else{
				owner=pp_it->second;
			}
			U64 hop = gethop(i,owner);
			assert(hop<3);
			//U64 sharers = page_sharers[page];
			hop_hist_W[sharers][hop]=hop_hist_W[sharers][hop]+wrs;
			if(page_Ws[page]!=0){
				hop_hist_RtoRW[sharers][hop]=hop_hist_RtoRW[sharers][hop]+rds;
			}
			else{
				hop_hist_RO[sharers][hop]=hop_hist_RO[sharers][hop]+rds;
			}
			link_traffic_R[i][owner]=link_traffic_R[i][owner]+rds;
			link_traffic_W[i][owner]=link_traffic_W[i][owner]+wrs;
			mem_traffic[i]=mem_traffic[i]+rds+wrs;
			
			////@@@ repeat for CXL island @@@@@
			U64 owner_CI=i;
			auto pp_it_CI = page_owner_CI.find(page);
			if(pp_it_CI==page_owner_CI.end()){
				// this will favor thread 0. 
				// should be ok after the first phase..
				page_owner_CI[page]=i; 
				owner_CI=i;
			}
			else{
				owner_CI=pp_it_CI->second;
			}
			U64 hop_CI = gethop(i,owner_CI);
			//U64 sharers_CI = page_sharers[page];
			hop_hist_W_CI[sharers][hop_CI]=hop_hist_W_CI[sharers][hop_CI]+wrs;
			if(page_Ws[page]!=0){
				hop_hist_RtoRW_CI[sharers][hop_CI]=hop_hist_RtoRW_CI[sharers][hop_CI]+rds;
			}
			else{
				hop_hist_RO_CI[sharers][hop_CI]=hop_hist_RO_CI[sharers][hop_CI]+rds;
			}
			if(owner_CI==CXO){
				CI_traffic_R[i]=CI_traffic_R[i]+rds;
				CI_traffic_W[i]=CI_traffic_W[i]+wrs;
				
			}
			else{
				link_traffic_R_CI[i][owner]=link_traffic_R_CI[i][owner]+rds;
				link_traffic_W_CI[i][owner]=link_traffic_W_CI[i][owner]+wrs;
				mem_traffic_CI[i]=mem_traffic_CI[i]+rds+wrs;
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
	// Reassign owners
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	if(LIMIT_MIGRATION){
		auto it_migration = sorted_candidates.begin();
		//for(int i=0;i<MIGRATION_LIMIT;i++){
		uint64_t i=0;
		uint64_t i_cxi=0;
		while((i<MIGRATION_LIMIT) || (i_cxi<MIGRATION_LIMIT)){
			// if(i<3){
			// 	cout<<"i: "<<i<<", page: "<<it_migration->first<<endl;
			// }
			U64 page  = it_migration->first;
			U64 most_acc=0;
			U64 new_owner=INVAL_OWNER;
			for(U64 j=0; j<pac_size;j++){
				if(page_access_counts[j][page]>most_acc){
					most_acc = page_access_counts[j][page];
					new_owner=j;
				}
			}
			if(i<MIGRATION_LIMIT){ // baseline
				//for baseline
				assert(page_owner.find(page)!=page_owner.end()); //remove this after sanity check
				
				U64 old_owner = page_owner[page];
				if(old_owner!=new_owner){
					//if migrated. if no migration, don't increment
					i++;
					migrated_pages++;
					page_owner[page]=new_owner;
				}				
			}
			if(i_cxi<MIGRATION_LIMIT){ // cxl-island
				U64 sharers = page_sharers[page];
				U64 old_owner = page_owner_CI[page];
				if(sharers>=SHARER_THRESHOLD){
					// with replication allowed, would have to check RW ratio and take appropraite step
					if(page_owner_CI[page]!=CXO){
						pages_to_CI++;
						i_cxi++;
						page_owner_CI[page]=CXO;
					}
				}
				else{
					if(page_owner_CI[page]!=new_owner){
						migrated_pages_CI++;
						page_owner_CI[page]=new_owner;
						i_cxi++;
					}
				}
				
			}

			it_migration++;
			if(it_migration==sorted_candidates.end()){//reached end of pages
				cout<<"reached end of sorted_candidates"<<endl;
				cout<<"sorted candidates size: "<<sorted_candidates.size()<<endl;
				break;
			}
			
		}

	}
	else{
		for (const auto& po : page_owner) {
			U64 page = po.first;
			U64 sharers = page_sharers[page];
			if(sharers==0){
				//no one accssed this page, no owner change
				continue;
			}
			U64 most_acc=0;
			U64 new_owner=INVAL_OWNER;

			for(U64 i=0; i<pac_size;i++){
				if(page_access_counts[i][page]>most_acc){
					most_acc = page_access_counts[i][page];
					new_owner=i;
				}
			}
			assert(new_owner!=INVAL_OWNER);
			if(page_owner.find(page)!=page_owner.end()){
				if(page_owner[page]!=new_owner){
					migrated_pages++;			
				}
			}
			page_owner[page]=new_owner;
			//page_owner_CI[page]=new_owner;
			if(sharers>=SHARER_THRESHOLD){
				if(page_owner_CI[page]!=CXO){
					pages_to_CI++;
				}
				page_owner_CI[page]=CXO;
			}
			else{
				if(page_owner_CI[page]!=new_owner){
					migrated_pages_CI++;
				}
				page_owner_CI[page]=new_owner;
			}
		}
	}

	// Record Stat Data from this phase
	string dir_name = generate_phasedirname();
	if(mkdir(dir_name.c_str(),0777)==-1){
		perror(("Error creating directory " + dir_name).c_str());
    } else {
        std::cout << "Created directory " << dir_name << std::endl;
    }
	
	U64 total_pages = page_sharers.size();
	U64 memory_touched = total_pages*PAGESIZE;
	U64 memory_touched_inMB = memory_touched>>20;
	U64 sumallacc=0;
	for(U64 i=0;i<N_THR;i++){
		sumallacc+=total_num_accs[i];
	}


	log_misc_stats(memory_touched_inMB,total_num_accs[0]
	,sumallacc,migrated_pages,migrated_pages_CI,pages_to_CI,misc_log_full);

	cout<<"baseline"<<endl;
	cout<<"memtraffic on node 5: "<<mem_traffic[5]<<endl;
	U64 linktraffic_5_12=link_traffic_R[5][12]+link_traffic_R[12][5]+link_traffic_W[5][12]+link_traffic_W[12][5];
	cout<<"link traffic between 5 and 12: "<< linktraffic_5_12<<endl;
	
	cout<<"CXL Island"<<endl;
	cout<<"memtraffic on node 5: "<<mem_traffic_CI[5]<<endl;
	U64 linktraffic_5_12_CI=link_traffic_R_CI[5][12]+link_traffic_R_CI[12][5]+link_traffic_W_CI[5][12]+link_traffic_W_CI[12][5];
	cout<<"link traffic between 5 and 12: "<< linktraffic_5_12_CI<<endl;
	cout<<"traffic to CI from a single node(5): "<<CI_traffic_R[5]+CI_traffic_W[5]<<endl;

	savearray(hist_access_sharers, N_THR_OFFSET,"access_hist.txt\0");
	savearray(hist_access_sharers_W, N_THR_OFFSET,"access_hist_W.txt\0");
	savearray(hist_access_sharers_R_to_RWP, N_THR_OFFSET,"access_hist_R_to_RWP.txt\0");
	savearray(hist_access_sharers_R, N_THR_OFFSET,"access_hist_R.txt\0");
	savearray(hist_page_sharers, N_THR_OFFSET,"page_hist.txt\0");
	savearray(hist_page_sharers_W, N_THR_OFFSET,"page_hist_W.txt\0");
	savearray(hist_page_sharers_R, N_THR_OFFSET,"page_hist_R.txt\0");
	save2Darr(hist_page_shareres_nacc, N_THR_OFFSET, "page_hist_nacc.txt\0");

	save_hophist(hop_hist_W,N_THR_OFFSET, "hop_hist_W.txt\0");
	save_hophist(hop_hist_RO,N_THR_OFFSET, "hop_hist_RO.txt\0");
	save_hophist(hop_hist_RtoRW,N_THR_OFFSET, "hop_hist_RtoRW.txt\0");

	save_hophist(hop_hist_W_CI,N_THR_OFFSET, "hop_hist_W_CI.txt\0");
	save_hophist(hop_hist_RO_CI,N_THR_OFFSET, "hop_hist_RO_CI.txt\0");
	save_hophist(hop_hist_RtoRW_CI,N_THR_OFFSET, "hop_hist_RtoRW_CI.txt\0");

	if(curphase>=phase_to_dump_pagemapping && curphase<(phase_to_dump_pagemapping+num_phases_to_dump_pagemapping)){
		save_uo_map(page_owner,"page_owner.txt\0");
		save_uo_map(page_owner_CI,"page_owner_CI.txt\0");
	}

	
	curphase=curphase+1;
	return 0;
}

int main(){

	omp_init_lock(&page_W_lock);
	omp_init_lock(&page_R_lock);
	omp_init_lock(&page_owner_lock);
	omp_init_lock(&page_owner_CI_lock);
	omp_init_lock(&hop_hist_lock);
	omp_init_lock(&hop_hist_CI_lock);
	curphase=0;
	
	phase_to_dump_pagemapping=(skipinsts/PHASE_CYCLES)-1; 
	//for initial check
	//phase_to_dump_pagemapping=1;
	assert(phase_to_dump_pagemapping>=0);
	//-1 because mapping is deteremined at the end of previus phase

	for(int i=0; i<N_THR;i++){
		std::ostringstream tfname;
		tfname << "memtrace_t" << (i+THR_OFFSET_VAL) << ".out";
		std::cout<<tfname.str()<<std::endl;
    	trace[i] = fopen(tfname.str().c_str(), "rb");
	}

	process_phase(); //do a single phase for warmup(page allocation)
	//while(!any_trace_done){
	for(int i=0;i<100;i++){ //putting a bound for now
		if(any_trace_done) break;
		process_phase();
	}
	cout<<"processed "<<curphase<<" phases"<<endl;
	for(int i=0; i<N_THR;i++){
		fclose(trace[i]);
	}
	return 0;

}