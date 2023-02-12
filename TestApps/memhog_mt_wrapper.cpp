#include <iostream>
#include <cstdint>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "memhog_mt.hpp"
#include <getopt.h>
#include <random>
//#include "zsim_nic_defines.hpp"



int main(int argc, char* argv[]) {

	int numthreads = 1;
	uint64_t start_core = 0;
	//uint64_t ws_size=8388608;
	uint64_t ws_size=1024*1024*sizeof(uint64_t);

	static const struct option opts[] = {                                       
		{.name = "num-threads", .has_arg = 1, .flag = NULL, .val = 't'},        
		{.name = "ws_size", .has_arg = 1,.flag = NULL, .val = 'd'} };           

	int c;

	while (1) {                                                                 
		c = getopt_long(argc, argv, "M:t:b:N:n:c:u:m:C:q:O:K:B:L:D:r:d:p", opts, NULL);
		if (c == -1) {                                                          
			printf("GETOPT FAILED (1 FAIL expected)\n");                        
			break;                                                              
		}                                                                       
		switch (c) {                                                            
			case 't':                                                           
				numthreads = atol(optarg);    
				printf("memhog threads %d\n", numthreads);          
				break;                                                          
			case 'd':                                                           
				ws_size = atol(optarg);                      
				break; 

			default:                                                            
				printf("Invalid argument %d\n", c);                             
				//assert(false);                                                  
		}                                                                       
	}  

	
	pthread_t *thread_arr = (pthread_t*)malloc(numthreads * sizeof(pthread_t));

	struct thread_params* tpa;
	tpa = (struct thread_params*)malloc(numthreads * sizeof(struct thread_params));

	int i;

	std::mt19937 mt(1234);	
	
	uint64_t arrsize = ws_size / (sizeof(uint64_t));
	//arrsize=arrsize/2; //2arrays
	//for(i=0;i<numthreads;i++){
	//	uint64_t * marr = (uint64_t *)malloc(arrsize * (sizeof(uint64_t)));
	//	uint64_t * marr2 = (uint64_t *)malloc(arrsize * (sizeof(uint64_t)));
	//	for(int ii=0;ii<arrsize;ii++){
	//		uint64_t kk = mt();
	//		marr[ii]=i*arrsize+ii+kk;
	//	}
	//	tpa[i].marr=marr;
	//	tpa[i].marr2=marr2;
	//}


        uint64_t * marr = (uint64_t *)malloc(arrsize * (sizeof(uint64_t)));


	for (i = 0; i < numthreads; i++) {
		tpa[i].ws_size = ws_size;
		tpa[i].core_id = i;//using it as tid
                tpa[i].marr = marr;
		int err = pthread_create(&thread_arr[i], NULL, memhog_thread, (void*)&(tpa[i]));
		if (err != 0) std::cout << "pthread_create failed" << std::endl;
	}

	for (i = 0; i < numthreads; i++) {
		pthread_join(thread_arr[i], NULL);
	}


	free(thread_arr);


	std::cout << "memhog_mt wrapper done" << std::endl;
	return 0;
}


