#include <iostream>
#include <fstream>

#define ARR_SIZE 1024

int main(){


    uint32_t arr[ARR_SIZE];
    for (int i=0; i<ARR_SIZE;i++){
        arr[i]=i;
    }
    int sum=0;
    for (int i=0; i<ARR_SIZE;i++){
        sum+=arr[i];
    }
    std::cout<<"sum: "<<sum<<std::endl;

    return 0;


}
