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


access_cdf = np.cumsum(access_pdf)
access_cdf_R = np.cumsum(access_pdf_R)
access_cdf_R_toRWpage = np.cumsum(access_pdf_R_toRWpage)
access_cdf_W = np.cumsum(access_pdf_W)
pages_cdf = np.cumsum(pages_pdf)
pages_cdf_W = np.cumsum(pages_pdf_W)
pages_cdf_R = np.cumsum(pages_pdf_R)


####### number of accesses hist
#pages_pdf_nacc=[]
#for i in range(len(hist_page_sharers_nacc)):
#    pdf_i = [x/allpage for x in hist_page_sharers_nacc[i]]
#    pages_pdf_nacc.append(pdf_i)


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
plt.rcParams.update({'font.size': 16})

label_fontsize=24
legend_fontsize=19
ticklabel_size=18
### PLOT ACCESS HIST
ifig,iax=plt.subplots()
X_axis = np.arange(len(hist_total_access_sharers))

#iax.bar(X_axis, hist_total_access_sharers)
#iax.bar(X_axis, access_pdf)
bottom=np.zeros(len(X_axis))
iax.bar(X_axis, access_cdf_W, label='Write', color=colors[0])
bottom=bottom+access_cdf_W
iax.bar(X_axis, access_cdf_R_toRWpage, label='Read to RW_page', bottom=bottom, color=colors[1])
bottom=bottom+access_cdf_R_toRWpage
iax.bar(X_axis, access_cdf_R, label='Read to RO_page', bottom=bottom, color=colors[2])
#iax.bar(X_axis, access_pdf_R, label='Read')

iax.legend(fontsize=legend_fontsize)
iax.set_xticks(X_axis, fontsize=ticklabel_size)
#iax.set_xlabel("Number of threads sharing the accessed page")
iax.set_xlabel("# sharers", fontsize=label_fontsize)
#iax.set_ylabel("Distribution of accesses")
iax.set_ylabel("CDF", fontsize=label_fontsize)
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

iax.set_xlim([0.1,17])

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
print("total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")


ifig.figsize=(20, 10)
ifig.savefig('hist_access_RW_cdf.png', bbox_inches='tight')

### PLOT PAGES HIST
ifig,iax=plt.subplots()
X_axis = np.arange(len(pages_cdf))

bottom=np.zeros(len(X_axis)) # USE when distinguishing access per page
#iax.bar(X_axis, pages_pdf)
iax.bar(X_axis, pages_cdf_W, label='RW page', color=colors[0]) 
iax.bar(X_axis, pages_cdf_R, label='RO page', bottom=pages_cdf_W, color=colors[2]) 
iax.legend(fontsize=legend_fontsize)

iax.set_xticks(X_axis, fontsize=ticklabel_size)
#iax.set_xlabel("Number of threads sharing the page")
iax.set_xlabel("# sharers", fontsize=label_fontsize)
iax.set_ylabel("CDF", fontsize=label_fontsize)
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)
iax.set_xlim([0.1,17])

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")

ifig.figsize=(20, 10)
ifig.savefig('hist_pages_RW_cdf.png', bbox_inches='tight')

#exit(0)




