#include <iostream>
#include <fstream>

#define ARR_SIZE 1024*1024*256
//#define ARR_SIZE 1048576

int main(){


    uint64_t *arr = (uint64_t *)malloc(ARR_SIZE * (sizeof(uint64_t)));
    //uint64_t arr[ARR_SIZE];
    for (int i=0; i<ARR_SIZE;i++){
        arr[i]=i;
    }
    int sum=0;
		uint64_t dummyval;
		__asm__ __volatile__(
            "xchg %%rbx, %%rbx;"
            ::"b"((uint64_t)(&dummyval)),"c"((uint64_t)0x5) //clobbered registers
        );

    for (int i=0; i<ARR_SIZE;i++){
        sum+=arr[i];
    }
    std::cout<<"sum: "<<sum<<std::endl;

    return 0;


}
