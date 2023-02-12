/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains an ISA-portable PIN tool for functional simulation of
 *  instruction+data TLB+cache hierarchies
 */

#include <iostream>

#include "pin.H"

typedef UINT32 CACHE_STATS; // type of cache hit/miss counters

#include "pin_cache.H"

FILE * trace;
KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "memtrace.out", "specify output file name");

uint64_t ins_count=0;
uint64_t memref_single_count=0;
uint64_t memref_multi_count=0;
uint64_t num_maccess=0;

//intermediate data structure to batch write to file
#define TBUF_SIZE 1024
uint64_t t_buf[TBUF_SIZE];
uint64_t tb_i = 0;


namespace IL1
{
// 1st level instruction cache: 32 kB, 32 B lines, 32-way associative
const UINT32 cacheSize                         = 32 * KILO;
const UINT32 lineSize                          = 32;
const UINT32 associativity                     = 32;
const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_NO_ALLOCATE;

const UINT32 max_sets          = cacheSize / (lineSize * associativity);
const UINT32 max_associativity = associativity;

typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
} // namespace IL1
static IL1::CACHE il1("L1 Instruction Cache", IL1::cacheSize, IL1::lineSize, IL1::associativity);


namespace UL3
{
// 3rd level unified cache: 16 MB, 64 B lines, direct mapped
const UINT32 cacheSize                         = 16 * MEGA;
const UINT32 lineSize                          = 64;
const UINT32 associativity                     = 1;
const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

const UINT32 max_sets = cacheSize / (lineSize * associativity);

typedef CACHE_DIRECT_MAPPED(max_sets, allocation) CACHE;
} // namespace UL3
static UL3::CACHE ul3("L3 Unified Cache", UL3::cacheSize, UL3::lineSize, UL3::associativity);

static inline VOID dump_tbuf(THREADID tid) { //TODO add threaID to arg  
    //added this as an optimization, but doesn't seem to save runtime much :(
    for (uint64_t i = 0; i < tb_i; i++) {
        fprintf(trace, "%p\n", (void*)(t_buf[i]));
    }
    tb_i = 0;
}

static VOID Fini(int code, VOID* v) //TODO this should change to threadfini?
{
    dump_tbuf();
    std::cout << "num mem accesses: "<<num_maccess <<std::endl;
    std::cout << "Fini finished"<<std::endl;
}

static inline VOID recordAccess(ADDRINT addr, THREADID tid) { //TODO add threaID to arg
    num_maccess++;
    t_buf[tb_i] = addr;
    tb_i++;
    if (tb_i == TBUF_SIZE) {
        dump_tbuf(tid);
    }
    
}

static inline VOID Ul3Access(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType, THREADID tid) ////TODO add threaID to arg
{
    const BOOL ul3hit = ul3.Access(addr, size, accessType);
    if(!ul3hit){
        recordAccess(addr, tid);
    }
}



static VOID InsRef(ADDRINT addr, THREADID tid) //TODO add threaID to arg
{
    ins_count++;
    if(ins_count % 100000==0){
        std::cout<<"thread_"<<tid << " ins_count: " << ins_count / 1000 << " K" << std::endl;
        //Mark timestamp in the trace file
        dump_tbuf();
        fprintf(trace, "CYCLE_COUNT %ld\n", ins_count);
    }

    const UINT32 size                        = 1; // assuming access does not cross cache lines
    const CACHE_BASE::ACCESS_TYPE accessType = CACHE_BASE::ACCESS_TYPE_LOAD;

    //// first level I-cache (Got rid of l1/l2 for data cache, keeping IL1
    const BOOL il1Hit = il1.AccessSingleLine(addr, accessType);
    if (!il1Hit) Ul3Access(addr, size, accessType);

}

static VOID MemRefMulti(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType, THREADID tid) //TODO add threaID to arg
{
    //TODO figure out how to handle memref_multi
    Ul3Access(addr, size, accessType);
    return;

    //memref_multi_count++;
    //if(memref_multi_count % 10000==0){
    //std::cout<<"memref_multi_count : "<<memref_multi_count/10000<<"*10k"<<std::endl;
    //}

    //fprintf(trace, "%p\n", (void *)addr);
    //return;

    ////TODO figure out how to handle memref_multi
    //for(UINT32 i=0;i<size;i++){
    //  ADDRINT addr_i = addr + i*64;
    //  fprintf(trace, "%p\n", (void *)addr_i);
    //}
    //return;
    ////above is test code. TODO REMOVE
  


    
}

static VOID MemRefSingle(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType, THREADID tid) //TODO add threaID to arg
{
    Ul3Access(addr, size, accessType);
    return;
    //DBG counter
    //memref_single_count++;
    //if(memref_single_count % 10000==0) {std::cout<<"memref_single_count : "<<memref_single_count/10000<<"*10k"<<std::endl;}
    //DBG for all accesses
    //fprintf(trace, "%p\n", (void *)addr);
    //return;
    

}
static VOID Instruction(INS ins, VOID* v)
{

    // all instruction fetches access I-cache
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)InsRef, IARG_INST_PTR, IARG_THREAD_ID, IARG_END);

    if (!INS_IsStandardMemop(ins)) return;
    if (INS_MemoryOperandCount(ins) == 0) return;
    ;

    UINT32 readSize = 0, writeSize = 0;
    UINT32 readOperandCount = 0, writeOperandCount = 0;

    for (UINT32 opIdx = 0; opIdx < INS_MemoryOperandCount(ins); opIdx++)
    {
        if (INS_MemoryOperandIsRead(ins, opIdx))
        {
            readSize = INS_MemoryOperandSize(ins, opIdx);
            readOperandCount++;
            break;
        }
        if (INS_MemoryOperandIsWritten(ins, opIdx))
        {
            writeSize = INS_MemoryOperandSize(ins, opIdx);
            writeOperandCount++;
            break;
        }
    }

    if (readOperandCount > 0)
    {
        const AFUNPTR countFun = (readSize <= 4 ? (AFUNPTR)MemRefSingle : (AFUNPTR)MemRefMulti);

        // only predicated-on memory instructions access D-cache
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, countFun, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_UINT32,
                                 CACHE_BASE::ACCESS_TYPE_LOAD, IARG_THREAD_ID, IARG_END);
    }

    if (writeOperandCount > 0)
    {
        const AFUNPTR countFun = (writeSize <= 4 ? (AFUNPTR)MemRefSingle : (AFUNPTR)MemRefMulti);

        // only predicated-on memory instructions access D-cache
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, countFun, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_UINT32,
                                 CACHE_BASE::ACCESS_TYPE_STORE, IARG_THREAD_ID, IARG_END);
    }
}

extern int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    trace = fopen(KnobOutputFile.Value().c_str(), "w");
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    std::cout<<"before calling prog"<<std::endl;
    // Never returns
    PIN_StartProgram();
    std::cout<<"never returns"<<std::endl;
    return 0; // make compiler happy
}
