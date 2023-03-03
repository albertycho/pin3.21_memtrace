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
import os.path

def save_object(obj,fname):
    try:
        #with open("data.pickle", "wb") as f:
        with open(fname, "wb") as f:
            pickle.dump(obj, f, protocol=pickle.HIGHEST_PROTOCOL)
    except Exception as ex:
        print("Error during pickling object (Possibly unsupported):", ex)


### TODO make this configurable or automate it?
n_thr = 2;
thr_offset_val=2;
n_thr_offset = n_thr+thr_offset_val;

pagesize = 4096; #assume 4KB pages
pagebits = 12;

page_access_counts=[] #pages and their access count, per thread
page_access_counts_R=[] #pages and their access count, per thread
page_access_counts_W=[] #pages and their access count, per thread
page_sharers={} #list of all unique pages and their sharer count
page_Rs={} #list of all unique pages and their read count
page_Ws={} #list of all unique pages and their write count

#hist_access_sharers={} ### per thread

hist_access_sharers_per_thread=[]
hist_total_access_sharers=[0]*(n_thr_offset)
hist_total_access_sharers_R=[0]*(n_thr_offset)
hist_total_access_sharers_W=[0]*(n_thr_offset)
hist_page_sharers=[0]*(n_thr_offset)
#### split by RW
hist_page_sharers_R=[0]*(n_thr_offset)
hist_page_sharers_W=[0]*(n_thr_offset)
#### split by number of accesses
hist_page_sharers_nacc=[]
for i in range(10):
    tmparr=[0]*(n_thr_offset)
    hist_page_sharers_nacc.append(tmparr)
print("sancheck that log2(1)=0, "+str(math.log2(1)))
print("sancheck that log2(3)=1, VAL: "+str(int((math.log2(3)))))


#ignore t0 and 1 for masstree, but for others?


#mtname = "memtrace_t8.out"
#mtname = "dummy_memtrace.out"

mtnames = [];
mtname_base = "memtrace_t"
for n in range(0,n_thr):
    i = n+thr_offset_val
    mtnames.append(mtname_base+str(i)+".out")
    assert(os.path.exists(mtnames[n]))
print(mtnames)
print(str(len(mtnames)))
#exit (0)
#mtnames.append("memtrace_t8.out")
#mtnames.append("memtrace_t9.out")

#### STEP 1
print("running step1");
for mtname in mtnames:
    print('reading '+mtname)
    f1 = open(mtname,'r')                                                       
    #f3 = open("zsim_final.out",'w')                                                 
                                                                                    
    line1 = f1.readline();
    
    pa_count={}
    pa_count_R={}
    pa_count_W={}
    while line1:
        #if 'CYCLE_COUNT' in line1:
        if 'INST_COUNT' in line1:
            line1=f1.readline();
            continue
        else:
            tmp=line1.split(' ')
            addr = int(tmp[0],0)
            isW = True
            #print(tmp[1]);
            if(len(tmp)<2):
                print("invalid entry. run may have crashed")
                break;
            if('0' in tmp[1]):
                isW = False
            #print(addr)
            page = addr >> pagebits;
            ##### 1) inclement access count in pa_count
            if (page in pa_count):
                pcount = pa_count[page]
                pcount = pcount+1
                pa_count[page] = pcount
                if(isW):
                    pa_count_W[page]=(pa_count_W[page])+1
                else:
                    pa_count_R[page]=(pa_count_R[page])+1
            else: 
                #pcount = 1
                pa_count[page]=1
                if(isW):
                    pa_count_W[page]=1
                    pa_count_R[page]=0
                else:
                    pa_count_W[page]=0
                    pa_count_R[page]=1
            #pa_count[page] = pcount
            
            ##### 2) add page if not in full page list
            if not (page in page_sharers):
                page_sharers[page]=0; ## just add entry in this iteration
                page_Rs[page]=0; ## just add entry in this iteration
                page_Ws[page]=0; ## just add entry in this iteration
            if(isW):
                page_Ws[page]=(page_Ws[page])+1
            else:
                page_Rs[page]=(page_Rs[page])+1
    
        line1 = f1.readline();
    f1.close()
    page_access_counts.append(pa_count);
    page_access_counts_R.append(pa_count_R);
    page_access_counts_W.append(pa_count_W);
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
#### 3-1 populate access sharer histogram
#for pa_count in page_access_counts:
for ii in range(len(page_access_counts)):
    pa_count=page_access_counts[ii]
    pa_count_W=page_access_counts_W[ii]
    pa_count_R=page_access_counts_R[ii]
    curthread_hist_access_sharers=[0]*(n_thr_offset)
    for page in pa_count:
        sharers=page_sharers[page]
        curthread_hist_access_sharers[sharers]=curthread_hist_access_sharers[sharers]+pa_count[page]
        hist_total_access_sharers[sharers]=hist_total_access_sharers[sharers]+pa_count[page]
        hist_total_access_sharers_W[sharers]=hist_total_access_sharers_W[sharers]+pa_count_W[page]
        hist_total_access_sharers_R[sharers]=hist_total_access_sharers_R[sharers]+pa_count_R[page]
    hist_access_sharers_per_thread.append(curthread_hist_access_sharers)

#### 3-2 populate page sharer histogram
#hist_total_access_sharers=[0]*n_thr
#hist_page_sharers=[0]*n_thr
page_with_more_writes=0
page_with_more_reads=0
for page in page_sharers:
    sharers=page_sharers[page]
    hist_page_sharers[sharers]=hist_page_sharers[sharers]+1
    ### write only happens on dirty writeback. so at least as many reads than write. do sanity check
    if((page_Rs[page]) < (page_Ws[page])):
        #print("WARNING - MORE WRITES RECORDED THAN READS FOR THE PAGE, R: "+str(page_Rs[page])+", W: "+str(page_Ws[page])+", pageID:"+ str(hex(page<<pagebits))) 
        page_with_more_writes=page_with_more_writes+1
    else:
        page_with_more_reads=page_with_more_reads+1
    if((page_Rs[page]) > (100*(page_Ws[page]))):
        hist_page_sharers_R[sharers] = (hist_page_sharers_R[sharers]) + 1
    else:
        hist_page_sharers_W[sharers] = (hist_page_sharers_W[sharers]) + 1
    ### by access
    access_count = page_Rs[page] + page_Ws[page]
    ii = int(math.log2(access_count))
    if(ii > 9):
        ii=9
    hist_page_sharers_nacc[ii][sharers] =  (hist_page_sharers_nacc[ii][sharers]) +1
    #print

print("pages with more reads: "+str(page_with_more_reads))
print("pages with more writes: "+str(page_with_more_writes))


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
save_object(hist_total_access_sharers_R, "access_hist_R.pickle")
save_object(hist_total_access_sharers_W, "access_hist_W.pickle")
save_object(hist_page_sharers, "page_hist.pickle")
save_object(hist_page_sharers_R, "page_hist_R.pickle")
save_object(hist_page_sharers_W, "page_hist_W.pickle")
save_object(hist_page_sharers_nacc, "page_hist_nacc.pickle")
print("ppscript: objects saved")

allacc = sum(hist_total_access_sharers)
misc_sum_file = open("misc_stat.txt", "w")
misc_sum_file.write("Used Memory: "+usedmem_str);
misc_sum_file.write("total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
misc_sum_file.close()
