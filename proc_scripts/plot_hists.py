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

def load_object(filename):
    try:
        with open(filename, "rb") as f:
            return pickle.load(f)
    except Exception as ex:
        print("Error during unpickling object (Possibly unsupported):", ex)

hist_page_sharers=load_object("page_hist.pickle")
hist_total_access_sharers = load_object("access_hist.pickle")

allacc = sum(hist_total_access_sharers)
access_pdf = [x/allacc for x in hist_total_access_sharers]

allpage = sum(hist_page_sharers)
pages_pdf = [x/allpage for x in hist_page_sharers]

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
iax.bar(X_axis, access_pdf)

iax.set_xticks(X_axis)
iax.set_xlabel("Number of threads sharing the accessed page")
iax.set_ylabel("Distribution of accesses")
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")
print("total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")


ifig.figsize=(20, 10)
ifig.savefig('access_hist.png', bbox_inches='tight')

### PLOT PAGES HIST
ifig,iax=plt.subplots()
X_axis = np.arange(len(pages_pdf))

#iax.bar(X_axis, hist_total_access_sharers)
iax.bar(X_axis, pages_pdf)

iax.set_xticks(X_axis)
iax.set_xlabel("Number of threads sharing the page")
iax.set_ylabel("Distribution of Pages")
iax.grid(color='gray', linestyle='--', linewidth=0.2, markevery=int, zorder=1, alpha=0.4)

#iax.text(0,0.7,"Total accesses: "+str( '%.2f'% (float(allacc)/1000000.0))+"Million")

ifig.figsize=(20, 10)
ifig.savefig('pages_hist.png', bbox_inches='tight')




