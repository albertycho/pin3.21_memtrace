#include <iostream>
#include <fstream>
#include <cstdint>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "memhog_mt.hpp"
//#include "zsim_nic_defines.hpp"
#include <sys/syscall.h>                                                        
#include <getopt.h>       

#define ARR_SIZE 1024*1024

using namespace std;

void* memhog_thread(void* inarg) {

	thread_params* casted_inarg = (thread_params*) inarg;
	int tid = casted_inarg->core_id;
	uint64_t ws_size = 8388608;
	//core_id = casted_inarg->core_id;
	ws_size = casted_inarg->ws_size;

	//if(argc>1){
	//	core_id=atoi(argv[1]);
	//}
	//else{
	//	std::cout<<"qp_test: no core_id specified"<<std::endl;
	//}

	//pid_t pid = getpid();

	//cpu_set_t cpuset;
	//CPU_ZERO(&cpuset);

	//CPU_SET(core_id, &cpuset);

	//int setresult = sched_setaffinity(pid, sizeof(cpuset), &cpuset);

	//if(setresult!=0){
	//	std::cout<<"qp_test - sched_setaffinity failed"<<std::endl;
	//}

	std::cout << "memhogTid: " << tid << std::endl;

	uint64_t array_size = ws_size / sizeof(uint64_t);
	std::cout<<"memhog: array_size: "<<array_size<<std::endl;
	//array_size = array_size / 2;

	//uint64_t * hog_arr = (uint64_t*)malloc(array_size*sizeof(uint64_t));
	//uint64_t * hog_arr = casted_inarg->marr;
	//uint64_t * hog_arr2 = casted_inarg->marr2;

        uint64_t * arr=casted_inarg->marr;
        //uint64_t arr[ARR_SIZE];
        for (int i=0; i<array_size;i++){
            arr[i]=i;
        }
        int sum=0;
        for (int i=0; i<array_size;i++){
            sum+=arr[i];
        }
        std::cout<<"sum: "<<sum<<std::endl;

        return 0;



}


