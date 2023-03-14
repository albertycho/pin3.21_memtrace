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

def load_object(filename):
    try:
        with open(filename, "rb") as f:
            return pickle.load(f)
    except Exception as ex:
        print("Error during unpickling object (Possibly unsupported):", ex)

hist_page_sharers=load_object("page_hist.pickle")
hist_page_sharers_R=load_object("page_hist_R.pickle")
hist_page_sharers_W=load_object("page_hist_W.pickle")
hist_total_access_sharers = load_object("access_hist.pickle")
hist_total_access_sharers_R = load_object("access_hist_R.pickle")
hist_total_access_sharers_R_toRWpage = load_object("access_hist_R_toRWpage.pickle")
hist_total_access_sharers_W = load_object("access_hist_W.pickle")
##### number of accesses hist
hist_page_sharers_nacc = load_object("page_hist_nacc.pickle")

hist_hop_W = load_object("hop_hist_W.pickle")
hist_hop_RO = load_object("hop_hist_RO.pickle")
hist_hop_RtoRW = load_object("hop_hist_RtoRW.pickle")

hist_hop_W_CI = load_object("hop_hist_W_CI.pickle")
hist_hop_RO_CI = load_object("hop_hist_RO_CI.pickle")
hist_hop_RtoRW_CI = load_object("hop_hist_RtoRW_CI.pickle")

allacc = sum(hist_total_access_sharers)
access_pdf = [x/allacc for x in hist_total_access_sharers]
access_pdf_R = [x/allacc for x in hist_total_access_sharers_R]
access_pdf_R_toRWpage = [x/allacc for x in hist_total_access_sharers_R_toRWpage]
access_pdf_W = [x/allacc for x in hist_total_access_sharers_W]

allpage = sum(hist_page_sharers)
pages_pdf = [x/allpage for x in hist_page_sharers]
pages_pdf_W = [x/allpage for x in hist_page_sharers_W]
pages_pdf_R = [x/allpage for x in hist_page_sharers_R]


##### number of accesses hist
pages_pdf_nacc=[]
for i in range(len(hist_page_sharers_nacc)):
    pdf_i = [x/allpage for x in hist_page_sharers_nacc[i]]
    pages_pdf_nacc.append(pdf_i)

#greens = ['#32CD32', '#7CFC00', '#008000', '#556B2F']
green_colors_brightness = ['#7CFC00', '#32CD32', '#008000', '#556B2F']
#blue_colors = ['#1f77b4', '#4363d8', '#3cb44b', '#ffe119', '#f58231']
blue_colors_brightness = ['#ffe119', '#3cb44b', '#f58231', '#1f77b4', '#4363d8']
#orange_colors = ['#FFA07A', '#FF8C00', '#FF6347', '#FFD700', '#FF4500']
orange_colors_brightness = ['#FFD700', '#FFA07A', '#FF8C00', '#FF6347', '#FF4500']
#red_colors = ['#FFC0CB', '#DC143C', '#FF5733', '#FF0000', '#8B0000']
red_colors_brightness = ['#FFC0CB', '#FF5733', '#FF0000', '#DC143C', '#8B0000']
purple_colors_brightness = ['#E6E6FA', '#D8BFD8', '#BA55D3', '#800080']
dark_purple_colors_brightness = ['#483D8B', '#4B0082', '#2E0854']
colors=[]
colors.append(green_colors_brightness)
#colors.append(blue_colors_brightness)
colors.append(dark_purple_colors_brightness)
colors.append(red_colors_brightness)
colors.append(orange_colors_brightness)
### PLOT ACCESS HIST
ifig,iax=plt.subplots()
X_axis = np.arange(len(hist_hop_W))
tp_hist_hop_W=transpose(hist_hop_W)
tp_hist_hop_RtoRW=transpose(hist_hop_RtoRW)
tp_hist_hop_RO=transpose(hist_hop_RO)


tp_hist_hop_W_CI=transpose(hist_hop_W_CI)
tp_hist_hop_RtoRW_CI=transpose(hist_hop_RtoRW_CI)
tp_hist_hop_RO_CI=transpose(hist_hop_RO_CI)


barwidth=0.2

#bottom=np.zeros(len(X_axis))
#iax.bar(X_axis - (barwidth*2)+(ii*barwidth), tp_hist_hop_W[ii], bottom=bottom, width=barwidth, label='')
#bottom=bottom+tp_hist_hop_W[ii]
#iax.bar(X_axis- (barwidth*2)+(ii*barwidth), tp_hist_hop_RtoRW[ii], bottom=bottom, width=barwidth)
#bottom=bottom+tp_hist_hop_RtoRW[ii]
#iax.bar(X_axis- (barwidth*2)+(ii*barwidth), tp_hist_hop_RO[ii], bottom=bottom, width=barwidth)


for ii in range(len(tp_hist_hop_W_CI)):
    #if(ii==0):
    #    continue
    print(ii)
    bottom=np.zeros(len(X_axis))
    iax.bar(X_axis - (barwidth*2)+(ii*barwidth), tp_hist_hop_W_CI[ii], bottom=bottom, width=barwidth, label = str(ii)+'hops', color=colors[ii][0])
    bottom=bottom+tp_hist_hop_W_CI[ii]
    iax.bar(X_axis- (barwidth*2)+(ii*barwidth), tp_hist_hop_RtoRW_CI[ii], bottom=bottom, width=barwidth, color=colors[ii][1])
    bottom=bottom+tp_hist_hop_RtoRW_CI[ii]
    iax.bar(X_axis- (barwidth*2)+(ii*barwidth), tp_hist_hop_RO_CI[ii], bottom=bottom, width=barwidth, color=colors[ii][2])


iax.legend()
iax.set_xticks(X_axis)
iax.set_xlabel("Number of threads sharing the accessed page")
iax.set_ylabel("Distribution of accesses")
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
#print("total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")


ifig.figsize=(30, 10)
ifig.savefig('hist_hop_CI.png', bbox_inches='tight')

ifig,iax=plt.subplots()
X_axis = np.arange(len(hist_hop_W))

for ii in range(len(tp_hist_hop_W)-1):
    #if(ii==0):
    #    continue
    print(ii)
    bottom=np.zeros(len(X_axis))
    iax.bar(X_axis - (barwidth*2)+(ii*barwidth), tp_hist_hop_W[ii], bottom=bottom, width=barwidth, label = str(ii)+'hops', color=colors[ii][0])
    bottom=bottom+tp_hist_hop_W[ii]
    iax.bar(X_axis- (barwidth*2)+(ii*barwidth), tp_hist_hop_RtoRW[ii], bottom=bottom, width=barwidth, color=colors[ii][1])
    bottom=bottom+tp_hist_hop_RtoRW[ii]
    iax.bar(X_axis- (barwidth*2)+(ii*barwidth), tp_hist_hop_RO[ii], bottom=bottom, width=barwidth, color=colors[ii][2])


iax.legend()
iax.set_xticks(X_axis)
iax.set_xlabel("Number of threads sharing the accessed page")
iax.set_ylabel("Distribution of accesses")
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
#print("total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")


ifig.figsize=(30, 10)
ifig.savefig('hist_hop.png', bbox_inches='tight')

#exit(0)


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





