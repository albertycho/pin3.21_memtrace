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
KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "l3trace.out", "specify output file name");

uint64_t ins_count=0;
uint64_t ins_count_2=0;
uint64_t memref_single_count=0;
uint64_t memref_multi_count=0;

namespace ITLB
{
// instruction TLB: 4 kB pages, 32 entries, fully associative
const UINT32 lineSize                          = 4 * KILO;
const UINT32 cacheSize                         = 32 * lineSize;
const UINT32 associativity                     = 32;
const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

const UINT32 max_sets          = cacheSize / (lineSize * associativity);
const UINT32 max_associativity = associativity;

typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
} // namespace ITLB
static ITLB::CACHE itlb("ITLB", ITLB::cacheSize, ITLB::lineSize, ITLB::associativity);

namespace DTLB
{
// data TLB: 4 kB pages, 32 entries, fully associative
const UINT32 lineSize                          = 4 * KILO;
const UINT32 cacheSize                         = 32 * lineSize;
const UINT32 associativity                     = 32;
const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

const UINT32 max_sets          = cacheSize / (lineSize * associativity);
const UINT32 max_associativity = associativity;

typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
} // namespace DTLB
static DTLB::CACHE dtlb("DTLB", DTLB::cacheSize, DTLB::lineSize, DTLB::associativity);

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

namespace DL1
{
// 1st level data cache: 32 kB, 32 B lines, 32-way associative
const UINT32 cacheSize                         = 32 * KILO;
const UINT32 lineSize                          = 32;
const UINT32 associativity                     = 32;
const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_NO_ALLOCATE;

const UINT32 max_sets          = cacheSize / (lineSize * associativity);
const UINT32 max_associativity = associativity;

typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
} // namespace DL1
static DL1::CACHE dl1("L1 Data Cache", DL1::cacheSize, DL1::lineSize, DL1::associativity);

namespace UL2
{
// 2nd level unified cache: 2 MB, 64 B lines, direct mapped
const UINT32 cacheSize                         = 2 * MEGA;
const UINT32 lineSize                          = 64;
const UINT32 associativity                     = 1;
const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

const UINT32 max_sets = cacheSize / (lineSize * associativity);

typedef CACHE_DIRECT_MAPPED(max_sets, allocation) CACHE;
} // namespace UL2
static UL2::CACHE ul2("L2 Unified Cache", UL2::cacheSize, UL2::lineSize, UL2::associativity);

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

static VOID Fini(int code, VOID* v)
{
    //std::cerr << itlb;
    //std::cerr << dtlb;
    std::cerr << il1;
    std::cerr << dl1;
    std::cerr << ul2;
    std::cerr << ul3;

    //std::cout << itlb;
    //std::cout << dtlb;
    std::cout << il1;
    std::cout << dl1;
    std::cout << ul2;
    std::cout << ul3;
    std::cout << "Fini finished"<<std::endl;
}

static VOID Ul3Access(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType)
{
    const BOOL ul3hit = ul3.Access(addr, size, accessType);
    if(!ul3hit){
        fprintf(trace, "%p\n", (void *)addr);
    }
}

//temporary comment out while not used
//static VOID Ul2Access(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType)
//{
//    // second level unified cache
//    const BOOL ul2Hit = ul2.Access(addr, size, accessType);
//
//    // third level unified cache
//    //if (!ul2Hit) ul3.Access(addr, size, accessType);
//    if (!ul2Hit) Ul3Access(addr, size, accessType);
//}

static VOID InsRef(ADDRINT addr)
{
    ins_count_2++;
    if(ins_count_2 % 10000==0){
    std::cout<<"ins_count_2: "<<ins_count_2/10000<<"*10k"<<std::endl;
    }

    const UINT32 size                        = 1; // assuming access does not cross cache lines
    const CACHE_BASE::ACCESS_TYPE accessType = CACHE_BASE::ACCESS_TYPE_LOAD;

    //// ITLB
    //itlb.AccessSingleLine(addr, accessType);

    //// first level I-cache
    //const BOOL il1Hit = il1.AccessSingleLine(addr, accessType);

    //// second level unified Cache
    //if (!il1Hit) Ul2Access(addr, size, accessType);
    //Ul2Access(addr, size, accessType);
    Ul3Access(addr, size, accessType);
}

static VOID MemRefMulti(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType)
{
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
    
    
    //std::cout<<"in MemRefMulti, size: "<<size<<std::endl;
    // DTLB : acho: idk if we need this for our purpose. TODO commnet out and see
    //dtlb.AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);

    //// first level D-cache
    //const BOOL dl1Hit = dl1.Access(addr, size, accessType);

    //// second level unified Cache
    //if (!dl1Hit) Ul2Access(addr, size, accessType);

    //TODO figure out how to handle memref_multi
    //Ul2Access(addr, size, accessType);
    Ul3Access(addr, size, accessType);

    
}

static VOID MemRefSingle(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType)
{
    //memref_single_count++;
    //if(memref_single_count % 10000==0){
    //std::cout<<"memref_single_count : "<<memref_single_count/10000<<"*10k"<<std::endl;
    //}


    //fprintf(trace, "%p\n", (void *)addr);
    //return;
    
    ////above is test code. TODO REMOVE

    // DTLB
    //dtlb.AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);

    // first level D-cache
    //const BOOL dl1Hit = dl1.AccessSingleLine(addr, accessType);

    //// second level unified Cache
    //if (!dl1Hit) Ul2Access(addr, size, accessType);
    //Ul2Access(addr, size, accessType);
    Ul3Access(addr, size, accessType);

}
static VOID Instruction(INS ins, VOID* v)
{
    ins_count++;
    if(ins_count % 10000==0){
    std::cout<<"ins_count: "<<ins_count/10000<<"*10k"<<std::endl;
    }
    // all instruction fetches access I-cache
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)InsRef, IARG_INST_PTR, IARG_END);

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
                                 CACHE_BASE::ACCESS_TYPE_LOAD, IARG_END);
    }

    if (writeOperandCount > 0)
    {
        const AFUNPTR countFun = (writeSize <= 4 ? (AFUNPTR)MemRefSingle : (AFUNPTR)MemRefMulti);

        // only predicated-on memory instructions access D-cache
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, countFun, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_UINT32,
                                 CACHE_BASE::ACCESS_TYPE_STORE, IARG_END);
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
