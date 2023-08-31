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
#include <chrono>

#include <cmath>
#include <queue>
#include <stack>
#include <set>
#include <unordered_map>
#include <omp.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include "page_map_generator.hpp"

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
//#define MIGRATION_LIMIT 12288


using namespace std;

FILE * trace[N_THR];

omp_lock_t page_W_lock;
omp_lock_t page_R_lock;
omp_lock_t page_owner_lock;
omp_lock_t page_owner_CI_lock;

vector<unordered_map<uint64_t, uint64_t>> page_access_counts_dummy;
// vector<unordered_map<uint64_t, uint64_t>> page_access_counts_R(N_THR);
// vector<unordered_map<uint64_t, uint64_t>> page_access_counts_W(N_THR);
//unordered_map<uint64_t, uint64_t> page_sharers;
//unordered_map<uint64_t, uint64_t> page_Rs;
//unordered_map<uint64_t, uint64_t> page_Ws;
unordered_map<uint64_t, uint64_t> page_owner;
unordered_map<uint64_t, uint64_t> page_owner_CI;
uint64_t pages_in_pool = 0;

unordered_map<uint64_t, uint64_t> migration_per_page;
unordered_map<uint64_t, uint64_t> migration_per_page_CI;

//// for tracking access stats from past 1 billion insts
vector<vector<unordered_map<uint64_t, uint64_t>>> page_access_counts_history={};
vector<vector<unordered_map<uint64_t, uint64_t>>> page_access_counts_R_history={};
vector<vector<unordered_map<uint64_t, uint64_t>>> page_access_counts_W_history={};
vector<unordered_map<uint64_t, uint64_t>> page_Rs_history={};
vector<unordered_map<uint64_t, uint64_t>> page_Ws_history={};



//U64 curphase=0;
U64 phase_end_cycle=0;
bool any_trace_done=false;
std::ofstream misc_log_full("misc_log_full.txt");

uint64_t skipinsts=5000000000;
uint64_t phase_to_dump_pagemapping=0;
uint64_t num_phases_to_dump_pagemapping=20;

int process_phase(){
	cout<<"starting phase "<<curphase<<endl;
	misc_log_full<<"starting phase "<<curphase<<endl;
	phase_end_cycle=phase_end_cycle+PHASE_CYCLES;
	//page accesses
	//vector<unordered_map<uint64_t, uint64_t>> page_access_counts={};
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts(N_SOCKETS);
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts_R(N_SOCKETS);
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts_W(N_SOCKETS);
	
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts_per_thread(N_THR);
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts_R_per_thread(N_THR);
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts_W_per_thread(N_THR);

	//list of all unique pages and their sharer count
	unordered_map<uint64_t, uint64_t> page_sharers={};
	//list of all unique pags and their R/W count
	unordered_map<uint64_t, uint64_t> page_Rs={};
	unordered_map<uint64_t, uint64_t> page_Ws={};

	std::multiset<std::pair<uint64_t, uint64_t>, migration_compare> sorted_candidates;
	
	U64 migrated_pages=0;
	U64 migrated_pages_CI=0;
	U64 pages_to_CI=0;


	U64 total_num_accs[N_THR]={0};

	auto read_start = std::chrono::high_resolution_clock::now();

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
			U64 icount_val=0;
			read_8B_line(&icount_val, buffer, trace[i]);
			//cout<<"page: "<<page<<" icount: "<<icount_val<<endl;
			if(icount_val>=phase_end_cycle){
				//TODO set flag to break while loop
				//let's just break... missing one access is fine
				break;
			}


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
			page_access_counts_per_thread[i]=pa_count;
			page_access_counts_W_per_thread[i]=(pa_count_W);
			page_access_counts_R_per_thread[i]=(pa_count_R);
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
	
	auto read_end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> elapsed1 = read_end - read_start;
    std::cout << "Time taken for trace read: " << elapsed1.count() << " seconds\n";

	//consoliate page_access_counts per thread into per socket
	for(uint64_t ii=0; ii<N_THR; ii++){
		uint64_t socketid = ii>>2; //4 cores per socket
		for(const auto& pt : page_access_counts_per_thread[ii]){
			page_access_counts[socketid][pt.first]=page_access_counts[socketid][pt.first]+pt.second;
		}
		for(const auto& pt : page_access_counts_W_per_thread[ii]){
			page_access_counts_W[socketid][pt.first]=page_access_counts_W[socketid][pt.first]+pt.second;
		}
		for(const auto& pt : page_access_counts_R_per_thread[ii]){
			page_access_counts_R[socketid][pt.first]=page_access_counts_R[socketid][pt.first]+pt.second;
		}
	}

	//update access data from past 1 billion instructions
	page_access_counts_history.push_back(page_access_counts);
	page_access_counts_R_history.push_back(page_access_counts_R);
	page_access_counts_W_history.push_back(page_access_counts_W);
	page_Rs_history.push_back(page_Rs);
	page_Ws_history.push_back(page_Ws);
	if(page_access_counts_history.size()>HISTORY_LEN){ 
		//assume all the history vectors will have same length. as they should
		page_access_counts_history.erase(page_access_counts_history.begin());
		page_access_counts_R_history.erase(page_access_counts_R_history.begin());
		page_access_counts_W_history.erase(page_access_counts_W_history.begin());
		page_Rs_history.erase(page_Rs_history.begin());
		page_Ws_history.erase(page_Ws_history.begin());
	}

	auto consol_end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> elapsed2 = consol_end - read_end;
    std::cout << "Time taken for trace consol: " << elapsed2.count() << " seconds\n";

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// log sharers for each page for the past 1 Billion insts
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// build accesses per thread 
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts_consol(N_SOCKETS);
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts_R_consol(N_SOCKETS);
	vector<unordered_map<uint64_t, uint64_t>> page_access_counts_W_consol(N_SOCKETS);
	unordered_map<uint64_t, uint64_t> page_Rs_consol={};
	unordered_map<uint64_t, uint64_t> page_Ws_consol={};
	
	#pragma omp parallel for
	for(uint64_t ii=0;ii<N_SOCKETS;ii++){
		for(uint64_t jj=0;jj<page_access_counts_history.size();jj++){
			for (const auto& ppair : page_access_counts_history[jj][ii]){
				U64 page = ppair.first;
				page_access_counts_consol[ii][page]=page_access_counts_consol[ii][page]+ppair.second;
			}
			for (const auto& ppair : page_access_counts_R_history[jj][ii]){
				U64 page = ppair.first;
				page_access_counts_R_consol[ii][page]=page_access_counts_R_consol[ii][page]+ppair.second;
			}
			for (const auto& ppair : page_access_counts_W_history[jj][ii]){
				U64 page = ppair.first;
				page_access_counts_W_consol[ii][page]=page_access_counts_W_consol[ii][page]+ppair.second;
			}
		}
	}
	for(uint64_t jj=0;jj<page_Rs_history.size();jj++){
		for (const auto& ppair : page_Rs_history[jj]){
			U64 page = ppair.first;
			page_Rs_consol[page]=page_Rs_consol[page]+ppair.second;
		}
		for (const auto& ppair : page_Ws_history[jj]){
			U64 page = ppair.first;
			page_Ws_consol[page]=page_Ws_consol[page]+ppair.second;
		}
	}
	
	unordered_map<uint64_t, uint64_t> page_sharers_long={};
	for (const auto& pa_c : page_access_counts_consol) {
		for (const auto& ppair : pa_c) {
			U64 page = ppair.first;
			page_sharers_long[page]=page_sharers_long[page]+1;
			assert(page_sharers_long[page] < N_SOCKETS+1);
		}
	}
	
	auto sharer_consol_end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed3 = sharer_consol_end - consol_end;
    std::cout << "Time taken for sharer consol: " << elapsed3.count() << " seconds\n";


	// //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// // log sharers for each page for this phase
	// //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// for (const auto& pa_c : page_access_counts) {
	// 	for (const auto& ppair : pa_c) {
	// 		U64 page = ppair.first;
	// 		page_sharers[page]=page_sharers[page]+1;
	// 		assert(page_sharers[page] < N_SOCKETS+1);
	// 	}
	// }
	
	
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// sort pages in order of accesses in sorted_candidates
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//cout<<"page_Rs size: "<<page_Rs.size()<<endl;
	for (const auto& ppair : page_Rs_consol){
		U64 page = ppair.first;
		U64 accs = ppair.second + page_Ws_consol[page];
		//TODO can probably skip pages that have low accs, like < 100
		//if(accs>100){
		//dumping everything - for evictino from pool
			sorted_candidates.insert({page, accs});
		//}
	}
	cout<<"sorted candidates size: "<<sorted_candidates.size()<<endl;

	auto sort_end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed4 = sort_end - sharer_consol_end;
    std::cout << "Time taken for sort: " << elapsed4.count() << " seconds\n";



	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// Populate owners for new pages
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//dbg
	U64 pac_size = page_access_counts_consol.size();
	assert(pac_size==N_SOCKETS);
	
	uint64_t new_pages=0;
	for (const auto& ppair : page_sharers_long){
		auto page = ppair.first;
		auto pp_it = page_owner.find(page);
		uint64_t owner=0;
		if(pp_it==page_owner.end()){ //new page first touch
			new_pages++;
			// this will favor thread 0. 
			// should be ok after the first phase..
			unordered_map<uint64_t, uint64_t> pa_count;
			vector<uint64_t> sharers = {};
			uint64_t jj = 0;
			for (const auto& pa_c : page_access_counts) {
				if (pa_c.find(page) != pa_c.end()) {
					sharers.push_back(jj);
				}
				jj++;
			}
			uint64_t ri = rand() % sharers.size();
			owner = sharers[ri];
			page_owner[page]=owner;
			// if this is new page, wouldn't bei n page_owner_CI either
			page_owner_CI[page]=owner;
			
		}
	}
	
	std::cout<<"new pages: "<<new_pages<<endl;
	auto t_newassign = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed_newa = t_newassign - sort_end;
    std::cout << "Time taken for to assign owners to new pages: " << elapsed_newa.count() << " seconds\n";




	//save vpage to owner mapping, before reassigning for next phase
	// Record Stat Data from this phase
	string dir_name = generate_phasedirname();
	if(mkdir(dir_name.c_str(),0777)==-1){
		perror(("Error creating directory " + dir_name).c_str());
    } else {
        std::cout << "Created directory " << dir_name << std::endl;
    }

	//if(curphase>=phase_to_dump_pagemapping && curphase<(phase_to_dump_pagemapping+num_phases_to_dump_pagemapping)){
		// cout<<"dumping page to socket mapping"<<std::endl;
		// save_uo_map(page_owner,"page_owner.txt\0");
		// save_uo_map(page_owner_CI,"page_owner_CI.txt\0");
	//}

	

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// Reassign owners
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	if(LIMIT_MIGRATION){
		uint64_t pingpong_lim = curphase / 4;
		uint64_t pool_cap = (page_owner_CI.size())/POOL_FRACTION;
		auto it_migration = sorted_candidates.begin();
		auto eviction_it = sorted_candidates.rbegin();

		uint64_t i=0;
		uint64_t i_cxi=0;
		bool eviction_candidate_was_hot=false;
		while((i<MIGRATION_LIMIT) || (i_cxi<MIGRATION_LIMIT)){
			auto reassign_1_page_start = std::chrono::high_resolution_clock::now();
			// if(i<3){
			// 	cout<<"i: "<<i<<", page: "<<it_migration->first<<endl;
			// }
			//cout<<"it->first: "<<it_migration->first<<", it->second: "<<it_migration->second<<endl;
			//if(it_migration->second < )
			// if(it_migration->second < 100){
			// 	//pages are not hot anymore, stop migrating
			// 	i=MIGRATION_LIMIT+1;
			// 	i_cxi=MIGRATION_LIMIT+1;
			// }
			U64 page  = it_migration->first;
			U64 most_acc=0;
			U64 new_owner=INVAL_OWNER;
			U64 sharers = page_sharers_long[page];
			//TODO assign random owner among sharers for pages with +8 sharers
			if(sharers>=8){
				// uint64_t tmp_i = rand()%sharers;
				// //dbg
				// //cout<<"sharers: "<<sharers <<", tmp_i: "<<tmp_i<<endl;
				// ////
				// U64 tmp_j=0;
				// new_owner=0;
				// for(U64 j=0; j<N_SOCKETS;j++){
				// 	if(page_access_counts_consol[j][page]>0){
				// 		if(tmp_j==tmp_i){
				// 			new_owner = j;
				// 			break;
				// 		}
				// 		tmp_j++;
				// 	}
				// }
				//screw this, too slow
				new_owner=rand()%N_SOCKETS;

				//cout<<"new owner: "<<new_owner<<endl;
			}
			else{
				for(U64 j=0; j<N_SOCKETS;j++){
					if(page_access_counts_consol[j][page]>most_acc){
						most_acc = page_access_counts[j][page];
						new_owner=j;
					}
				}
			}
			// auto t_new_owner = std::chrono::high_resolution_clock::now();
			// std::chrono::duration<double> elapsed_new_owner = t_new_owner - reassign_1_page_start;
			// std::cout << "Time taken for finding new owner : " << elapsed_new_owner.count() << " seconds\n";

			if(i<MIGRATION_LIMIT){ // baseline
				//for baseline
				//assert(page_owner.find(page)!=page_owner.end()); //remove this after sanity check
				
				U64 old_owner=0;
				// auto pp_it = page_owner.find(page);
				// if(pp_it==page_owner.end()){ //new page first touch
				// 	old_owner=new_owner;
				// }
				// else{
				// 	old_owner = pp_it->second;//page_owner[page];
				// }
				old_owner=page_owner[page];
				if (migration_per_page[page] <= (pingpong_lim)) {
					if (old_owner != new_owner) {
						//if migrated. if no migration, don't increment
						i++;
						migrated_pages++;
						page_owner[page] = new_owner;
						migration_per_page[page] = migration_per_page[page] + 1;
					}
				}
			}
			auto t_baseline = std::chrono::high_resolution_clock::now();
			// std::chrono::duration<double> elapsed_baseline = t_baseline - t_new_owner;
			// std::cout << "Time taken for assigning baseline : " << elapsed_baseline.count() << " seconds\n";
			if(i_cxi<MIGRATION_LIMIT){ // cxl-island
				//U64 sharers = page_sharers_long[page];
				//U64 old_owner = page_owner_CI[page];
				if (migration_per_page_CI[page] <= (pingpong_lim)) {
					U64 old_owner=page_owner_CI[page];
					// auto pp_it = page_owner.find(page);
					// if(pp_it==page_owner_CI.end()){ //new page first touch
					// 	old_owner=new_owner;
					// }
					// else{
					// 	old_owner = page_owner_CI[page];
					// }
					if(!eviction_candidate_was_hot && (sharers >= SHARER_THRESHOLD)){
						//if (sharers >= SHARER_THRESHOLD) {
							// with replication allowed, would have to check RW ratio and take appropraite step
							//if (page_owner_CI[page] != CXO) {
						if (old_owner != CXO) {
							pages_to_CI++;
							pages_in_pool++;
							i_cxi++;
							page_owner_CI[page] = CXO;
							migration_per_page_CI[page] = migration_per_page_CI[page] + 1;
							//deal eviction if full capacity
							if(pool_cap <= pages_in_pool){
								//std::cout<<"DBG: is pool_cap being hit?"<<endl;
								//find eviction candidate
								uint64_t evicted_page=0;
								//auto it = sorted_candidates.rbegin();
								for(; eviction_it != sorted_candidates.rend(); ++eviction_it) {
									if(page_owner_CI[eviction_it->first] == CXO) {
										evicted_page = eviction_it->first;
										break;
									}
								}
								//std::cout<<"eviction_candidate access count: "<<eviction_it->second<<endl;
								if(eviction_it==sorted_candidates.rend()) {
									cout<<"WARNING: needed to find eviction candidate from pool but didn't find"<<endl;
									i_cxi=MIGRATION_LIMIT+1;
								}
								//U64 ev_most_acc=0;
								if((eviction_it->second)>=(it_migration->second)){
									//i_cxi=MIGRATION_LIMIT;
									cout<<"eviction candidate is hotter than new element into CXL island"<<endl;
									eviction_candidate_was_hot=true;
								}
								U64 ev_most_acc=0;
								U64 ev_new_owner=rand() % N_SOCKETS;								
								U64 ev_sharers = page_sharers_long[evicted_page];
								if(ev_sharers>=8){
									// uint64_t tmp_i = rand()%sharers;
									// //dbg
									// //std::cout<<"ev_sharers: "<<ev_sharers <<", tmp_i: "<<tmp_i<<endl;
									// ////
									// U64 tmp_j=0;
									// for(U64 j=0; j<N_SOCKETS;j++){
									// 	if(page_access_counts_consol[j][evicted_page]>1){
									// 		if(tmp_j==tmp_i){
									// 			ev_new_owner = j;
									// 			break;
									// 		}
									// 		tmp_j++;
									// 	}
									// }
									//cout<<"ev_new_owner: "<<ev_new_owner<<endl;									
								}
								else{
									for(U64 j=0; j<N_SOCKETS;j++){
										if(page_access_counts_consol[j][evicted_page]>ev_most_acc){
											ev_most_acc = page_access_counts[j][evicted_page];
											ev_new_owner=j;
										}
									}
								}
								// screw this, takes too long. if u were on the island, u had a lot of sharers.

								page_owner_CI[evicted_page] = ev_new_owner;
								pages_in_pool--;
								i_cxi++;

							}
						}
						//}
					}
					else {
						//U64 old_owner=0;
						//auto pp_it = page_owner.find(page);
						// if(pp_it==page_owner_CI.end()){ //new page first touch
						// 	old_owner=new_owner;
						// }
						// else{
						// 	old_owner = page_owner_CI[page];
						// }
						if (old_owner != new_owner) {
							migrated_pages_CI++;
							page_owner_CI[page] = new_owner;
							i_cxi++;
							migration_per_page_CI[page] = migration_per_page_CI[page] + 1;
						}
					}
				}
				
			}
			auto t_cxi = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed_cxi = t_cxi - t_baseline;
			//std::cout << "Time taken for assigning cxi : " << elapsed_cxi.count() << " seconds\n";

			it_migration++;
			if(it_migration==sorted_candidates.end()){//reached end of pages
				cout<<"reached end of sorted_candidates"<<endl;
				cout<<"sorted candidates size: "<<sorted_candidates.size()<<endl;
				break;
			}
			auto reassign_1_page_end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed5 = reassign_1_page_end - reassign_1_page_start;
			//std::cout << "Time taken for reassigning page "<< i<< ": " << elapsed5.count() << " seconds\n";


			
		}

	}
	else{
		std::cout<<"ONLY SUPPORTS SIMULATION WITH MIGARTION LIMIT!"<<endl;
	}

	std::cout<<"reassign owners done"<<std::endl;
	auto reassign_end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed6 = reassign_end - sort_end;
    std::cout << "Time taken for reassign owners: " << elapsed6.count() << " seconds\n";

	cout<<"dumping page to socket mapping"<<std::endl;
	save_uo_map(page_owner,"page_owner.txt\0");
	save_uo_map(page_owner_CI,"page_owner_CI.txt\0");

	//U64 total_pages = page_sharers.size();
	U64 total_pages = page_sharers_long.size();
	U64 memory_touched = total_pages*PAGESIZE;
	U64 memory_touched_inMB = memory_touched>>20;
	U64 sumallacc=0;
	for(U64 i=0;i<N_SOCKETS;i++){
		sumallacc+=total_num_accs[i];
	}
	/// find all pages whose owner is CXI, to get memory size on CXI
	uint64_t CXI_count = std::count_if(page_owner_CI.begin(), page_owner_CI.end(), [](const std::pair<uint64_t, uint64_t>& pair) {
        return pair.second == CXO;
    });


	log_misc_stats(memory_touched_inMB,total_num_accs[0]
	,sumallacc,migrated_pages,migrated_pages_CI,pages_to_CI, CXI_count, misc_log_full);


	curphase=curphase+1;
	return 0;
}

int main(){

	omp_init_lock(&page_W_lock);
	omp_init_lock(&page_R_lock);
	omp_init_lock(&page_owner_lock);
	omp_init_lock(&page_owner_CI_lock);
	
	assert(HISTORY_LEN >=1);
	
	curphase=0;
	
	phase_to_dump_pagemapping=(skipinsts/PHASE_CYCLES); 
	//for initial check
	//phase_to_dump_pagemapping=1;
	assert(phase_to_dump_pagemapping>=0);
	cout<<"phase to dump pagemapping "<<phase_to_dump_pagemapping<<std::endl;
	//-1 because mapping is deteremined at the end of previus phase

	for(int i=0; i<N_THR;i++){
		std::ostringstream tfname;
		tfname << "memtrace_t" << (i+THR_OFFSET_VAL) << ".out";
		//std::cout<<tfname.str()<<std::endl;
    	trace[i] = fopen(tfname.str().c_str(), "rb");
		if(trace[i]==NULL){
			std::cerr<<tfname.str()<<" failed to open"<<std::endl;
		}
	}

	process_phase(); //do a single phase for warmup(page allocation)
	//while(!any_trace_done){
	for(int i=0;i<10000;i++){ //putting a bound for now
		if(any_trace_done) break;
		process_phase();
	}
	cout<<"processed "<<curphase<<" phases"<<endl;
	for(int i=0; i<N_THR;i++){
		fclose(trace[i]);
	}
	return 0;

}
