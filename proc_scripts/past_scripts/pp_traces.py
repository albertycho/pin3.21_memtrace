#!/usr/bin/env python3                                                          
                                                                                
import os                                                                       
import sys                                                                      
import csv                                                                      
import math                                                                     
#import statistics                                                              
import numpy as np                                                              
import matplotlib                                                               
matplotlib.use('Agg')                                                           
matplotlib.rcParams['agg.path.chunksize'] = 10000                               
import matplotlib.pyplot as plt

import pickle

def save_object(obj,fname):
    try:
        #with open("data.pickle", "wb") as f:
        with open(fname, "wb") as f:
            pickle.dump(obj, f, protocol=pickle.HIGHEST_PROTOCOL)
    except Exception as ex:
        print("Error during pickling object (Possibly unsupported):", ex)


### TODO make this configurable or automate it?
n_thr = 4;
thr_offset_val=8;
n_thr_offset = n_thr+thr_offset_val;

pagesize = 4096; #assume 4KB pages
pagebits = 12;

page_access_counts=[] #pages and their access count, per thread
page_sharers={} #list of all unique pages and their sharer count

#hist_access_sharers={} ### per thread

hist_access_sharers_per_thread=[]
hist_total_access_sharers=[0]*(n_thr_offset)
hist_page_sharers=[0]*(n_thr_offset)

#ignore t0 and 1 for masstree, but for others?


#mtname = "memtrace_t8.out"
#mtname = "dummy_memtrace.out"

mtnames = [];
mtname_base = "memtrace_t"
for n in range(0,n_thr):
    i = n+thr_offset_val
    mtnames.append(mtname_base+str(i)+".out")
print(mtnames)
print(str(len(mtnames)))
#exit (0)
#mtnames.append("memtrace_t8.out")
#mtnames.append("memtrace_t9.out")

#### STEP 1
print("running step1");
for mtname in mtnames:
    f1 = open(mtname,'r')                                                       
    #f3 = open("zsim_final.out",'w')                                                 
                                                                                    
    line1 = f1.readline();
    
    pa_count={}
    while line1:
        #if 'CYCLE_COUNT' in line1:
        if 'INST_COUNT' in line1:
            line1=f1.readline();
            continue
        else:
            addr = int(line1,0)
            #print(addr)
            page = addr >> pagebits;
            ##### 1) inclement access count in pa_count
            if (page in pa_count):
                pcount = pa_count[page]
                pcount = pcount+1
            else: 
                pcount = 1
            pa_count[page] = pcount
            ##### 2) add page if not in full page list
            if not (page in page_sharers):
                page_sharers[page]=0; ## just add entry in this iteration
    
        line1 = f1.readline();
    f1.close()
    page_access_counts.append(pa_count);
    ### dbg code REMOVE TODO
    #pa_count={}
    #print("pa_count len: "+str(len(pa_count)))
    #print("page_access_counts[0] len: "+str(len(page_access_counts[0])))



#### STEP 2
print("running step2");
for pa_count in page_access_counts:
    for page in pa_count:
        scount = page_sharers[page]
        page_sharers[page]=scount+1

#### STEP3
print("running step3");
#### 3-1 populate page sharer histogram
#hist_total_access_sharers=[0]*n_thr
#hist_page_sharers=[0]*n_thr
for page in page_sharers:
    sharers=page_sharers[page]
    hist_page_sharers[sharers]=hist_page_sharers[sharers]+1

#### 3-2 populate access sharer histogram
#hist_access_sharers_per_thread=[]
for pa_count in page_access_counts:
    curthread_hist_access_sharers=[0]*(n_thr_offset)
    for page in pa_count:
        sharers=page_sharers[page]
        curthread_hist_access_sharers[sharers]=curthread_hist_access_sharers[sharers]+pa_count[page]
        hist_total_access_sharers[sharers]=hist_total_access_sharers[sharers]+pa_count[page]
    hist_access_sharers_per_thread.append(curthread_hist_access_sharers)

#### printing page histogram
#print("histogram of pages according to their number of sharers")
#for i in range(0,n_thr+1):
#    print(str(i)+": "+str(hist_page_sharers[i]))

####printing access histogram
print("\nAccess histogram for number of sharers on the accessed page:")
for i in range(0,n_thr_offset):
    print(str(i)+": "+str(hist_total_access_sharers[i]))
    #i=i+1
#print(hist_access_sharers_per_thread)





##### Printing below here
#print("page_access_counts len: "+str(len(page_access_counts)))
#print("pa_count len: "+str(len(pa_count)))
#print("page_access_counts[0] len: "+str(len(page_access_counts[0])))
print("page_sharers len(==number of all pages used): "+str(len(page_sharers)))
used_mem_inB=len(page_sharers)*pagesize
if(used_mem_inB > 1000000000): #larger than 1GB
    #usedmem_str = str((float(used_mem_inB) / 1000000000.0)) + "GB"
    usedmem_str = str('%.2f' % (float(used_mem_inB) / 1000000000.0)) + "GB"
else:
    usedmem_str = str('%.2f'% (float(used_mem_inB) / 1000000.0)) + "MB"
print("Used Memory: "+usedmem_str);
save_object(hist_total_access_sharers, "access_hist.pickle")
save_object(hist_page_sharers, "page_hist.pickle")
#print(pages)
