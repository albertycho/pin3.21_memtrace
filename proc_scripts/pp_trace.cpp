#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <stdint.h>
#include <limits.h>
#include <cstdio>

#include <queue>
#include <stack>
#include <set>
#include <unordered_map>

#define PAGESIZE 4096
#define PAGEBITS 12
#define N_THR 16
#define THR_OFFSET_VAL 2
#define N_THR_OFFSET (N_THR+1)

vector<map<uint64_t, uint64_t>> page_access_counts(n_thr);
vector<map<uint64_t, uint64_t>> page_access_counts_R(n_thr);
vector<map<uint64_t, uint64_t>> page_access_counts_W(n_thr);
map<uint64_t, uint64_t> page_sharers;
map<uint64_t, uint64_t> page_Rs;
map<uint64_t, uint64_t> page_Ws;

int main(){




	return 0;


}
