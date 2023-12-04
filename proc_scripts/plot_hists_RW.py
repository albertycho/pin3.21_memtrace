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
import csv

def load_object(filename):
    try:
        with open(filename, 'r') as file:
            # Use a list comprehension to create a list where each element is a line from the file
            # Use .strip() to remove any trailing newline characters, and int() to convert the string to an integer
            numbers = [int(line.strip()) for line in file]
            return numbers
    except Exception as ex:
        print("Error during reading object (Possibly unsupported):", ex)

def load_2darr(filename):
    with open(filename, 'r') as f:
        reader = csv.reader(f)
        array = list(map(lambda row: list(map(int, row)), list(reader)))
        return array
    

#colors=['#dda0dd', '#8fbc8f', '#faa460', '#9370db','lightyellow','aliceblue']
colors=['#dda0dd',  '#faa460', '#8fbc8f','#9370db','lightyellow','aliceblue']


hist_page_sharers=load_object("page_hist.txt")
hist_page_sharers_R=load_object("page_hist_R.txt")
hist_page_sharers_W=load_object("page_hist_W.txt")
hist_total_access_sharers = load_object("access_hist.txt")
hist_total_access_sharers_R = load_object("access_hist_R.txt")
hist_total_access_sharers_R_toRWpage = load_object("access_hist_R_to_RWP.txt")
hist_total_access_sharers_W = load_object("access_hist_W.txt")
##### number of accesses hist
hist_page_sharers_nacc = load_2darr("page_hist_nacc.txt")

print(hist_page_sharers)

print(hist_total_access_sharers)

allacc = sum(hist_total_access_sharers)
access_pdf = [x/allacc for x in hist_total_access_sharers]
access_pdf_R = [x/allacc for x in hist_total_access_sharers_R]
access_pdf_R_toRWpage = [x/allacc for x in hist_total_access_sharers_R_toRWpage]
access_pdf_W = [x/allacc for x in hist_total_access_sharers_W]

allpage = sum(hist_page_sharers)
pages_pdf = [x/allpage for x in hist_page_sharers]
pages_pdf_W = [x/allpage for x in hist_page_sharers_W]
pages_pdf_R = [x/allpage for x in hist_page_sharers_R]


###### number of accesses hist
pages_pdf_nacc=[]
for i in range(len(hist_page_sharers_nacc)):
    pdf_i = [x/allpage for x in hist_page_sharers_nacc[i]]
    pages_pdf_nacc.append(pdf_i)


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
iax.bar(X_axis, access_pdf_W, label='Write', color=colors[0])
bottom=bottom+access_pdf_W
iax.bar(X_axis, access_pdf_R_toRWpage, label='Read to RW_page', bottom=bottom, color=colors[1])
bottom=bottom+access_pdf_R_toRWpage
iax.bar(X_axis, access_pdf_R, label='Read to RO_page', bottom=bottom, color=colors[2])
#iax.bar(X_axis, access_pdf_R, label='Read')

iax.legend()
iax.set_xticks(X_axis)
#iax.set_xlabel("Number of threads sharing the accessed page")
iax.set_xlabel("# sharers")
iax.set_ylabel("Distribution of accesses")
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
print("total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
iax.set_xlim([0.1,17])

ifig.figsize=(20, 10)
ifig.savefig('hist_access_RW.png', bbox_inches='tight')

### PLOT PAGES HIST
ifig,iax=plt.subplots()
X_axis = np.arange(len(pages_pdf))

bottom=np.zeros(len(X_axis)) # USE when distinguishing access per page
#iax.bar(X_axis, pages_pdf)
iax.bar(X_axis, pages_pdf_W, label='Written', color=colors[0]) 
iax.bar(X_axis, pages_pdf_R, label='Read Only', bottom=pages_pdf_W, color=colors[2]) 
iax.legend()

iax.set_xticks(X_axis)
#iax.set_xlabel("Number of threads sharing the page")
iax.set_xlabel("# sharers")
iax.set_ylabel("Distribution of Pages")
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")

ifig.figsize=(20, 10)
ifig.savefig('hist_pages_RW.png', bbox_inches='tight')

#exit(0)

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
#iax.set_xlabel("Number of threads sharing the page")
iax.set_xlabel("# sharers")
iax.set_ylabel("Distribution of Pages")
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
iax.set_xlim([0.1,17])
ifig.figsize=(20, 10)
ifig.savefig('hist_pages_nacc.png', bbox_inches='tight')





