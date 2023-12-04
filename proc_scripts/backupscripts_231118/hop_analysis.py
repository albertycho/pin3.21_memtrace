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
        with open(fname, "wb") as f:
            pickle.dump(obj, f, protocol=pickle.HIGHEST_PROTOCOL)
    except Exception as ex:
        print("Error during pickling object (Possibly unsupported):", ex)

def gethop(a,b): #(accessing node, owner)
    if(b==100):
        return 3 #this is CXL island
    if(a==b): #local access
        return 0;
    agrp=(a>>2);
    bgrp=(b>>2);
    if(agrp==bgrp): #same chassis
        return 1;
    return 2;


### TODO make this configurable or automate it?
n_thr = 16;
thr_offset_val=2;
#n_thr_offset = n_thr+thr_offset_val;
n_thr_offset = n_thr+1;

pagesize = 4096; #assume 4KB pages
pagebits = 12;
tSH = 8
cumulate_stat = True

#page_access_counts=[] #pages and their access count, per thread
#page_access_counts_R=[] #pages and their access count, per thread
#page_access_counts_W=[] #pages and their access count, per thread
#page_sharers={} #list of all unique pages and their sharer count
#page_Rs={} #list of all unique pages and their read count
#page_Ws={} #list of all unique pages and their write count
#
##hist_access_sharers={} ### per thread
#
#hist_access_sharers_per_thread=[]
#hist_total_access_sharers=[0]*(n_thr_offset)
#hist_total_access_sharers_R=[0]*(n_thr_offset)
#hist_total_access_sharers_W=[0]*(n_thr_offset)
#hist_page_sharers=[0]*(n_thr_offset)
##### split by RW
#hist_page_sharers_R=[0]*(n_thr_offset)
#hist_page_sharers_W=[0]*(n_thr_offset)
##### split by number of accesses
#hist_page_sharers_nacc=[]
#for i in range(10):
#    tmparr=[0]*(n_thr_offset)
#    hist_page_sharers_nacc.append(tmparr)


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

### TODO open files before entering main loop.
    ### separate handle for each trace, put in array
f_handles = [];
for mtname in mtnames:
    f=open(mtname, 'rb')
    f_handles.append(f)


trace_not_done = True
phaselen = 1*1000*1000*1000 #### defined by number of instructions. 
next_period = phaselen
cur_phase = 0


page_owner={} #list of all unique pages and their owner node
page_owner_CI={} #list of all unique pages and their owner node for Solution
#### for baseline
hop_hist_W=[]
hop_hist_RO=[]
hop_hist_RtoRW=[]
for i in range(n_thr+1): #last entry for sum
    t_hop_hist_W    =[0]*4 ### hops can be 0~2
    t_hop_hist_RO   =[0]*4 
    t_hop_hist_RtoRW=[0]*4 
    hop_hist_W.append(t_hop_hist_W)
    hop_hist_RO.append(t_hop_hist_RO)
    hop_hist_RtoRW.append(t_hop_hist_RtoRW)

#### different list for our solution
hop_hist_W_CI=[]
hop_hist_RO_CI=[]
hop_hist_RtoRW_CI=[]
for i in range(n_thr+1): #last entry for sum
    t_hop_hist_W    =[0]*4 ### hops can be 0~2
    t_hop_hist_RO   =[0]*4 
    t_hop_hist_RtoRW=[0]*4 
    hop_hist_W_CI.append(t_hop_hist_W)
    hop_hist_RO_CI.append(t_hop_hist_RO)
    hop_hist_RtoRW_CI.append(t_hop_hist_RtoRW)


#print(hop_hist)
#exit(0)

while (trace_not_done):
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
    hist_total_access_sharers_R_toRWpage=[0]*(n_thr_offset)
    hist_total_access_sharers_W=[0]*(n_thr_offset)
    hist_page_sharers=[0]*(n_thr_offset)
    #### split by RW
    hist_page_sharers_R=[0]*(n_thr_offset)
    hist_page_sharers_W=[0]*(n_thr_offset)
    #### split by number of accesses
    hist_page_sharers_nacc=[]
    
    #### for baseline
    tmp_hop_hist_W=[]
    tmp_hop_hist_RO=[]
    tmp_hop_hist_RtoRW=[]
    for i in range(n_thr+1): #last entry for sum
        tmp_t_hop_hist_W    =[0]*4 ### hops can be 0~2
        tmp_t_hop_hist_RO   =[0]*4 
        tmp_t_hop_hist_RtoRW=[0]*4 
        tmp_hop_hist_W.append(tmp_t_hop_hist_W)
        tmp_hop_hist_RO.append(tmp_t_hop_hist_RO)
        tmp_hop_hist_RtoRW.append(tmp_t_hop_hist_RtoRW)

    #### for CI
    tmp_hop_hist_W_CI=[]
    tmp_hop_hist_RO_CI=[]
    tmp_hop_hist_RtoRW_CI=[]
    for i in range(n_thr+1): #last entry for sum
        tmp_t_hop_hist_W    =[0]*4 ### hops can be 0~2
        tmp_t_hop_hist_RO   =[0]*4 
        tmp_t_hop_hist_RtoRW=[0]*4 
        tmp_hop_hist_W_CI.append(tmp_t_hop_hist_W)
        tmp_hop_hist_RO_CI.append(tmp_t_hop_hist_RO)
        tmp_hop_hist_RtoRW_CI.append(tmp_t_hop_hist_RtoRW)





    for i in range(10):
        tmparr=[0]*(n_thr_offset)
        hist_page_sharers_nacc.append(tmparr)

    p_end_line = 'INST_COUNT '+str(next_period)
    print("phase "+str(cur_phase))
    curtid=0;
    for f1 in f_handles:
        print(f1.name)
        line1 = f1.read(8);
        pa_count={}
        pa_count_R={}
        pa_count_W={}
        period_not_done=True;

#        while (line1 and (not (p_end_line in line1))):
        while (line1 and period_not_done):
            cline1=int.from_bytes(line1, "little", signed=False)
            #if('INST_COUNT' in line1):
            if(cline1==0xc0ffee):
                print('c0ffee read')
                #line1=f1.readline();
                line1=f1.read(8);
                cline1= int.from_bytes(line1, "little", signed=False) 
                print('inst count: '+str(cline1))
                if(cline1==next_period):
                    period_not_done=False;
                    break;
                else:
                    #line1=f1.readline();
                    line1=f1.read(8);
                    continue
                #continue
            ### do actions
            addr = cline1
            isW = True
            #print(tmp[1]);
            if(cline1&1==0):
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
            if not (page in page_owner):
                page_owner[page]=curtid;
                page_owner_CI[page]=curtid;
    
            line1 = f1.read(8);

        if (not line1):
            trace_not_done=False
        #if(line1 and (p_end_line in line1)):
            #nothing
    
        page_access_counts.append(pa_count);
        page_access_counts_R.append(pa_count_R);
        page_access_counts_W.append(pa_count_W);
        curtid=curtid+1

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
            #hist_total_access_sharers_R[sharers]=hist_total_access_sharers_R[sharers]+pa_count_R[page]
            if(page_Ws[page]!=0):
                hist_total_access_sharers_R_toRWpage[sharers]=hist_total_access_sharers_R_toRWpage[sharers]+pa_count_R[page]
            else:
                hist_total_access_sharers_R[sharers]=hist_total_access_sharers_R[sharers]+pa_count_R[page]
        hist_access_sharers_per_thread.append(curthread_hist_access_sharers)
    
    #### 3-2 populate page sharer histogram
    #hist_total_access_sharers=[0]*n_thr
    #hist_page_sharers=[0]*n_thr
    for page in page_sharers:
        sharers=page_sharers[page]
        hist_page_sharers[sharers]=hist_page_sharers[sharers]+1
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

    #### STEP 4 populate Hop histogram ####
    print("running step4");
    #for pa_count in page_access_counts:
    #    for page in pa_count:
    for ii in range(len(page_access_counts)):
        pa_count=page_access_counts[ii]
        pa_count_W=page_access_counts_W[ii]
        pa_count_R=page_access_counts_R[ii]
        for page in pa_count:
            sharers=page_sharers[page]
            owner = page_owner[page]
            hop = gethop(ii,owner)
            
            owner_CI = page_owner_CI[page]
            hop_CI=gethop(ii,owner_CI)
            
            tmp_hop_hist_W[sharers][hop] = tmp_hop_hist_W[sharers][hop] + pa_count_W[page]
            tmp_hop_hist_W_CI[sharers][hop_CI] = tmp_hop_hist_W_CI[sharers][hop_CI] + pa_count_W[page]
            if(page_Ws[page] == 0):
                tmp_hop_hist_RO[sharers][hop] = tmp_hop_hist_RO[sharers][hop] + pa_count_R[page]
                tmp_hop_hist_RO_CI[sharers][hop_CI] = tmp_hop_hist_RO_CI[sharers][hop_CI] + pa_count_R[page]
            else:
                tmp_hop_hist_RtoRW[sharers][hop] = tmp_hop_hist_RtoRW[sharers][hop] + pa_count_R[page]
                tmp_hop_hist_RtoRW_CI[sharers][hop_CI] = tmp_hop_hist_RtoRW_CI[sharers][hop_CI] + pa_count_R[page]



    #### STEP 5 reassign owners based on current phase
    print("running step5");
    for page in page_owner:
        if not (page in page_sharers): ### no one touched this page in this phase
            continue
        sharers = page_sharers[page]
        owner_CI=-1;
        owner=-1;
        if (sharers >= tSH):
            #page_owner_CI[page]=100
            owner_CI=100;
            #continue
        if (sharers==1):
            continue
        most_acc = 0;
        owner = -1;
        for ii in range(len(page_access_counts)):
            if(page in page_access_counts[ii]):
                if (page_access_counts[ii][page] > most_acc):
                    most_acc = page_access_counts[ii][page]
                    owner = ii
        assert(owner!=-1)
        page_owner[page]=owner;
        if(owner_CI == -1):
            owner_CI = owner;
        page_owner_CI[page]=owner_CI;



    dirname = "1Bphase"+str(cur_phase)
    os.mkdir(dirname)
    save_object(hist_total_access_sharers, dirname+"/access_hist.pickle")
    save_object(hist_total_access_sharers_R, dirname+"/access_hist_R.pickle")
    save_object(hist_total_access_sharers_R_toRWpage, dirname+"/access_hist_R_toRWpage.pickle")
    save_object(hist_total_access_sharers_W, dirname+"/access_hist_W.pickle")
    save_object(hist_page_sharers, dirname+"/page_hist.pickle")
    save_object(hist_page_sharers_R, dirname+"/page_hist_R.pickle")
    save_object(hist_page_sharers_W, dirname+"/page_hist_W.pickle")
    save_object(hist_page_sharers_nacc, dirname+"/page_hist_nacc.pickle")

    save_object(tmp_hop_hist_W, dirname+"/hop_hist_W.pickle")
    save_object(tmp_hop_hist_RO, dirname+"/hop_hist_RO.pickle")
    save_object(tmp_hop_hist_RtoRW, dirname+"/hop_hist_RtoRW.pickle")
    
    save_object(tmp_hop_hist_W_CI, dirname+"/hop_hist_W_CI.pickle")
    save_object(tmp_hop_hist_RO_CI, dirname+"/hop_hist_RO_CI.pickle")
    save_object(tmp_hop_hist_RtoRW_CI, dirname+"/hop_hist_RtoRW_CI.pickle")

    ####print some stats
    print("page_sharers len(==number of all pages used): "+str(len(page_sharers)))
    used_mem_inB=len(page_sharers)*pagesize
    if(used_mem_inB > 1000000000): #larger than 1GB
        #usedmem_str = str((float(used_mem_inB) / 1000000000.0)) + "GB"
        usedmem_str = str('%.2f' % (float(used_mem_inB) / 1000000000.0)) + "GB"
    else:
        usedmem_str = str('%.2f'% (float(used_mem_inB) / 1000000.0)) + "MB"
    print("Used Memory: "+usedmem_str);

    allacc = sum(hist_total_access_sharers)
    misc_sum_file = open(dirname+"/misc_stat.txt", "w")
    misc_sum_file.write("Used Memory: "+usedmem_str+"\n");
    misc_sum_file.write("total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
    misc_sum_file.close()

    cur_phase = cur_phase + 1
    next_period=next_period + phaselen

    if (cumulate_stat):
        print("cumulate stat")
        for ii in range(len(hop_hist_W)):
            for jj in range(len(hop_hist_W[0])):
                hop_hist_W[ii][jj]=hop_hist_W[ii][jj] + tmp_hop_hist_W[ii][jj]
                hop_hist_RO[ii][jj]=hop_hist_RO[ii][jj] + tmp_hop_hist_RO[ii][jj]
                hop_hist_RtoRW[ii][jj]=hop_hist_RtoRW[ii][jj] + tmp_hop_hist_RtoRW[ii][jj]

                hop_hist_W_CI[ii][jj]=hop_hist_W_CI[ii][jj] + tmp_hop_hist_W_CI[ii][jj]
                hop_hist_RO_CI[ii][jj]=hop_hist_RO_CI[ii][jj] + tmp_hop_hist_RO_CI[ii][jj]
                hop_hist_RtoRW_CI[ii][jj]=hop_hist_RtoRW_CI[ii][jj] + tmp_hop_hist_RtoRW_CI[ii][jj]



#### save cumulative hop count ###
os.mkdir("cumulative")
save_object(hop_hist_W, "cumulative/hop_hist_W.pickle")
save_object(hop_hist_RO, "cumulative/hop_hist_RO.pickle")
save_object(hop_hist_RtoRW, "cumulative/hop_hist_RtoRW.pickle")

save_object(hop_hist_W_CI, "cumulative/hop_hist_W_CI.pickle")
save_object(hop_hist_RO_CI, "cumulative/hop_hist_RO_CI.pickle")
save_object(hop_hist_RtoRW_CI, "cumulative/hop_hist_RtoRW_CI.pickle")


###############WIP#####################    

