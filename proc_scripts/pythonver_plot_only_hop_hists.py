#!/usr/bin/env python3

import os
import sys
import csv
import math
import statistics
import numpy as np
import matplotlib
import matplotlib.pyplot as plt 

import pickle


lats=[80,130,350,200]

def transpose(TDL):
    x=len(TDL)
    y=len(TDL[0])
    retarr=[]
    for j in range(y):
        tmp=[0]*x
        for i in range(x):
            tmp[i]=TDL[i][j]
        retarr.append(tmp)
    return retarr

def add_sum(TDL): ##take in hophist and add sum for 0,1,2,3 hops
    x=(len(TDL))
    y=len(TDL[0])
    #print(str(x))
    tmp=[0]*y
    tmp2=[0]*y
    for j in range(y):
        for i in range(x):
            tmp[j]=tmp[j]+TDL[i][j]
    retarr=TDL
    retarr.append(tmp2)
    retarr.append(tmp)
    return retarr;



def load_object(filename):
    try:
        with open(filename, "rb") as f:
            return pickle.load(f)
    except Exception as ex:
        print("Error during unpickling object (Possibly unsupported):", ex)

#hist_page_sharers=load_object("page_hist.pickle")
#hist_page_sharers_R=load_object("page_hist_R.pickle")
#hist_page_sharers_W=load_object("page_hist_W.pickle")
#hist_total_access_sharers = load_object("access_hist.pickle")
#hist_total_access_sharers_R = load_object("access_hist_R.pickle")
#hist_total_access_sharers_R_toRWpage = load_object("access_hist_R_toRWpage.pickle")
#hist_total_access_sharers_W = load_object("access_hist_W.pickle")
###### number of accesses hist
#hist_page_sharers_nacc = load_object("page_hist_nacc.pickle")

hist_hop_W = load_object("hop_hist_W.pickle")
hist_hop_RO = load_object("hop_hist_RO.pickle")
hist_hop_RtoRW = load_object("hop_hist_RtoRW.pickle")

hist_hop_W_CI = load_object("hop_hist_W_CI.pickle")
hist_hop_RO_CI = load_object("hop_hist_RO_CI.pickle")
hist_hop_RtoRW_CI = load_object("hop_hist_RtoRW_CI.pickle")

#allacc = sum(hist_total_access_sharers)
#access_pdf = [x/allacc for x in hist_total_access_sharers]
#access_pdf_R = [x/allacc for x in hist_total_access_sharers_R]
#access_pdf_R_toRWpage = [x/allacc for x in hist_total_access_sharers_R_toRWpage]
#access_pdf_W = [x/allacc for x in hist_total_access_sharers_W]
#
#allpage = sum(hist_page_sharers)
#pages_pdf = [x/allpage for x in hist_page_sharers]
#pages_pdf_W = [x/allpage for x in hist_page_sharers_W]
#pages_pdf_R = [x/allpage for x in hist_page_sharers_R]
#
#
###### number of accesses hist
#pages_pdf_nacc=[]
#for i in range(len(hist_page_sharers_nacc)):
#    pdf_i = [x/allpage for x in hist_page_sharers_nacc[i]]
#    pages_pdf_nacc.append(pdf_i)


colors=['tab:green','tab:blue', 'tab:red', 'tab:purple']

### PLOT ACCESS HIST
ifig,iax=plt.subplots()

hist_hop_W =       add_sum(hist_hop_W)
hist_hop_RO =      add_sum(hist_hop_RO)
hist_hop_RtoRW =   add_sum(hist_hop_RtoRW)

hist_hop_W_CI =    add_sum(hist_hop_W_CI)
hist_hop_RO_CI =   add_sum(hist_hop_RO_CI)
hist_hop_RtoRW_CI =add_sum(hist_hop_RtoRW_CI)


tp_hist_hop_W=transpose(hist_hop_W)
tp_hist_hop_RtoRW=transpose(hist_hop_RtoRW)
tp_hist_hop_RO=transpose(hist_hop_RO)


tp_hist_hop_W_CI=transpose(hist_hop_W_CI)
tp_hist_hop_RtoRW_CI=transpose(hist_hop_RtoRW_CI)
tp_hist_hop_RO_CI=transpose(hist_hop_RO_CI)

X_axis = np.arange(len(hist_hop_W))

barwidth=0.2
xtick_labels = []
for i in range(len(X_axis) - 2 ):
    xtick_labels.append(str(i))
xtick_labels.append('')
xtick_labels.append('SUM')
#bottom=np.zeros(len(X_axis))
#iax.bar(X_axis - (barwidth*2)+(ii*barwidth), tp_hist_hop_W[ii], bottom=bottom, width=barwidth, label='')
#bottom=bottom+tp_hist_hop_W[ii]
#iax.bar(X_axis- (barwidth*2)+(ii*barwidth), tp_hist_hop_RtoRW[ii], bottom=bottom, width=barwidth)
#bottom=bottom+tp_hist_hop_RtoRW[ii]
#iax.bar(X_axis- (barwidth*2)+(ii*barwidth), tp_hist_hop_RO[ii], bottom=bottom, width=barwidth)

ifig.figsize=(100, 5)

lasti=len(hist_hop_W_CI)-1
print('lasti: '+str(lasti))
all_acc = sum(hist_hop_W_CI[lasti]) + sum(hist_hop_RtoRW_CI[lasti]) + sum(hist_hop_RO_CI[lasti])
print(str(all_acc))
#lasti=len(tp_hist_hop_W_CI)-1
sum_lat = 0
for ii in range(len(hist_hop_W_CI[lasti])):
    sum_lat=sum_lat+hist_hop_W_CI[lasti][ii]*lats[ii] + hist_hop_RtoRW_CI[lasti][ii]*lats[ii] + hist_hop_RO_CI[lasti][ii]*lats[ii]
    #t_hist_hop_W_CI[lasti][0]*lats[0] + t_hist_hop_RtoRW_CI[lasti][0]*lats[0] + t_hist_hop_RO_CI[lasti][0]*lats[0]
avg_lat = sum_lat/all_acc
print('avg_lat: '+str(avg_lat))
avg_in_text = 'avg_lat: '+str(int(avg_lat))+'ns'
bbox_props = dict(boxstyle='round', facecolor='white', alpha=0.5)
#plt.text(0.5, 0.95, avg_in_text, ha='center', va='center', transform=plt.gca().transAxes, bbox=bbox_props)
plt.text(0.5, 0.90, avg_in_text, ha='center', va='center', transform=plt.gca().transAxes, bbox=bbox_props, fontsize=22)

#iax.bar(X_axis, np.zeros(len(X_axis)), label='avg_lat: '+str(int(avg_lat))+'ns', color='black')
for ii in range(len(tp_hist_hop_W_CI)):
    #if(ii==0):
    #    continue
    labelname = str(ii)+'hops'
    if(ii==3):
        labelname='CXL Island'
    print(ii)
    bottom=np.zeros(len(X_axis))
    iax.bar(X_axis - (barwidth*2)+(ii*barwidth), tp_hist_hop_W_CI[ii], bottom=bottom, width=barwidth, label = labelname, color=colors[ii], alpha=1)
    bottom=bottom+tp_hist_hop_W_CI[ii]
    iax.bar(X_axis- (barwidth*2)+(ii*barwidth), tp_hist_hop_RtoRW_CI[ii], bottom=bottom, width=barwidth, color=colors[ii], alpha=0.7, hatch='///')
    bottom=bottom+tp_hist_hop_RtoRW_CI[ii]
    iax.bar(X_axis- (barwidth*2)+(ii*barwidth), tp_hist_hop_RO_CI[ii], bottom=bottom, width=barwidth, color=colors[ii], alpha=0.3, hatch='xxx')

### dummy bars to add color coding for legend
iax.bar(X_axis, np.zeros(len(X_axis)), label=' ', color='white', edgecolor='white', alpha=0)
iax.bar(X_axis, np.zeros(len(X_axis)), label='Read to RO', color='black', alpha=0.1, hatch='xx')
iax.bar(X_axis, np.zeros(len(X_axis)), label='Read to RW', color='black', alpha=0.3, hatch='///')
iax.bar(X_axis, np.zeros(len(X_axis)), label='Write', color='black', alpha=1)


plt.xlim(0.1,len(X_axis))
iax.legend()

iax.set_xticks(X_axis, xtick_labels)
iax.set_xlabel("Number of threads sharing the accessed page")
iax.set_ylabel("Distribution of accesses")
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
#print("total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
sumindex = len(tp_hist_hop_W_CI[0])-1;
h0_accs = tp_hist_hop_W_CI[0][sumindex] + tp_hist_hop_RtoRW_CI[0][sumindex] + tp_hist_hop_RO_CI[0][sumindex]
h1_accs = tp_hist_hop_W_CI[1][sumindex] + tp_hist_hop_RtoRW_CI[1][sumindex] + tp_hist_hop_RO_CI[1][sumindex]
h2_accs = tp_hist_hop_W_CI[2][sumindex] + tp_hist_hop_RtoRW_CI[2][sumindex] + tp_hist_hop_RO_CI[2][sumindex]
CI_accs = tp_hist_hop_W_CI[3][sumindex] + tp_hist_hop_RtoRW_CI[3][sumindex] + tp_hist_hop_RO_CI[3][sumindex]
print("CXL_ISLAND")
print("0 hop accs: "+str(h0_accs))
print("1 hop accs: "+str(h1_accs))
print("2 hop accs: "+str(h2_accs))
print("CXLI  accs: "+str(CI_accs))


plt.gcf().set_size_inches(20, 5)
#plt.figsize=(100, 10)
ifig.savefig('hist_hop_CI.png', bbox_inches='tight')

#exit(0)
ifig,iax=plt.subplots()
X_axis = np.arange(len(hist_hop_W))

lasti=len(hist_hop_W)-1
print('lasti: '+str(lasti))
all_acc = sum(hist_hop_W[lasti]) + sum(hist_hop_RtoRW[lasti]) + sum(hist_hop_RO[lasti])
print(str(all_acc))
sum_lat = 0
for ii in range(len(hist_hop_W[lasti])):
    sum_lat=sum_lat+hist_hop_W[lasti][ii]*lats[ii] + hist_hop_RtoRW[lasti][ii]*lats[ii] + hist_hop_RO[lasti][ii]*lats[ii]
avg_lat = sum_lat/all_acc
print('avg_lat: '+str(int(avg_lat)))
avg_in_text = 'avg_lat: '+str(int(avg_lat))+'ns'
bbox_props = dict(boxstyle='round', facecolor='white', alpha=0.5)
plt.text(0.5, 0.90, avg_in_text, ha='center', va='center', transform=plt.gca().transAxes, bbox=bbox_props, fontsize=22)
#dummy bar, so I can put avg_lat in the legend
#iax.bar(X_axis, np.zeros(len(X_axis)), label='avg_lat: '+str(int(avg_lat))+'ns', color='black')


for ii in range(len(tp_hist_hop_W)-1):
    #if(ii==0):
    #    continue
    print(ii)
    bottom=np.zeros(len(X_axis))
    iax.bar(X_axis - (barwidth*2)+(ii*barwidth), tp_hist_hop_W[ii], bottom=bottom, width=barwidth, label = str(ii)+'hops', color=colors[ii], alpha=1)
    bottom=bottom+tp_hist_hop_W[ii]
    iax.bar(X_axis- (barwidth*2)+(ii*barwidth), tp_hist_hop_RtoRW[ii], bottom=bottom, width=barwidth, color=colors[ii], alpha=0.7, hatch='///')
    bottom=bottom+tp_hist_hop_RtoRW[ii]
    iax.bar(X_axis- (barwidth*2)+(ii*barwidth), tp_hist_hop_RO[ii], bottom=bottom, width=barwidth, color=colors[ii], alpha=0.3, hatch='xxx')

### dummy bars to add color coding for legend
iax.bar(X_axis, np.zeros(len(X_axis)), label=' ', color='white', edgecolor='white', alpha=0)
iax.bar(X_axis, np.zeros(len(X_axis)), label='Read to RO', color='black', alpha=0.1, hatch='xx')
iax.bar(X_axis, np.zeros(len(X_axis)), label='Read to RW', color='black', alpha=0.3, hatch='///')
iax.bar(X_axis, np.zeros(len(X_axis)), label='Write', color='black', alpha=1)

iax.legend()
iax.set_xticks(X_axis,xtick_labels)
iax.set_xlabel("Number of threads sharing the accessed page")
iax.set_ylabel("Distribution of accesses")
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
#print("total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
plt.xlim(0.1,len(X_axis))
ifig.figsize=(30, 10)
plt.gcf().set_size_inches(20, 5)
ifig.savefig('hist_hop.png', bbox_inches='tight')

sumindex = len(tp_hist_hop_W[0])-1;
h0_accs = tp_hist_hop_W[0][sumindex] + tp_hist_hop_RtoRW[0][sumindex] + tp_hist_hop_RO[0][sumindex]
h1_accs = tp_hist_hop_W[1][sumindex] + tp_hist_hop_RtoRW[1][sumindex] + tp_hist_hop_RO[1][sumindex]
h2_accs = tp_hist_hop_W[2][sumindex] + tp_hist_hop_RtoRW[2][sumindex] + tp_hist_hop_RO[2][sumindex]
print("Baseline:")
print("0 hop accs: "+str(h0_accs))
print("1 hop accs: "+str(h1_accs))
print("2 hop accs: "+str(h2_accs))


exit(0)









#########Sancheck print############
##### printing page histogram
#print("histogram of pages according to their number of sharers")
#for i in range(0,len(hist_page_sharers)):
#    print(str(i)+": "+str(hist_page_sharers[i]))
#
#####printing access histogram
#print("\nAccess histogram for number of sharers on the accessed page:")
#for i in range(0,len(hist_total_access_sharers)):
#    print(str(i)+": "+str(hist_total_access_sharers[i]))



### PLOT ACCESS HIST
ifig,iax=plt.subplots()
X_axis = np.arange(len(hist_total_access_sharers))

#iax.bar(X_axis, hist_total_access_sharers)
#iax.bar(X_axis, access_pdf)
bottom=np.zeros(len(X_axis))
iax.bar(X_axis, access_pdf_W, label='Write')
bottom=bottom+access_pdf_W
iax.bar(X_axis, access_pdf_R_toRWpage, label='Read to RW_page', bottom=bottom)
bottom=bottom+access_pdf_R_toRWpage
iax.bar(X_axis, access_pdf_R, label='Read to RO_page', bottom=bottom)
#iax.bar(X_axis, access_pdf_R, label='Read')

iax.legend()
iax.set_xticks(X_axis)
iax.set_xlabel("Number of threads sharing the accessed page")
iax.set_ylabel("Distribution of accesses")
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
print("total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")


ifig.figsize=(20, 10)
ifig.savefig('hist_access_RW.png', bbox_inches='tight')

### PLOT PAGES HIST
ifig,iax=plt.subplots()
X_axis = np.arange(len(pages_pdf))

bottom=np.zeros(len(X_axis)) # USE when distinguishing access per page
#iax.bar(X_axis, pages_pdf)
iax.bar(X_axis, pages_pdf_W, label='Written') 
iax.bar(X_axis, pages_pdf_R, label='Read Only', bottom=pages_pdf_W) 
iax.legend()

iax.set_xticks(X_axis)
iax.set_xlabel("Number of threads sharing the page")
iax.set_ylabel("Distribution of Pages")
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")

ifig.figsize=(20, 10)
ifig.savefig('hist_pages_RW.png', bbox_inches='tight')


### PLOT PAGES HIST by NACC
ifig,iax=plt.subplots()
X_axis = np.arange(len(pages_pdf))

bottom=np.zeros(len(X_axis)) # USE when distinguishing access per page
#iax.bar(X_axis, pages_pdf)
for i in range(len(hist_page_sharers_nacc)):
    numacc=str(2**i)
    iax.bar(X_axis, pages_pdf_nacc[i], label=numacc+' accesses to the page', bottom=bottom) 
    bottom=bottom+pages_pdf_nacc[i]
#iax.bar(X_axis, pages_pdf_R, label='Read Only', bottom=pages_pdf_W) 
iax.legend()

iax.set_xticks(X_axis)
iax.set_xlabel("Number of threads sharing the page")
iax.set_ylabel("Distribution of Pages")
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")

ifig.figsize=(20, 10)
ifig.savefig('hist_pages_nacc.png', bbox_inches='tight')





