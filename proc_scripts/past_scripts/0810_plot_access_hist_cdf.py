#import matplotlib.pyplot as plt
#
## Read the counts from the file
#counts = []
#with open('access_hist.txt', 'r') as file:
#    for line in file:
#        counts.append(int(line.strip()))
#
## Compute the CDF
#total_count = sum(counts)
#cumulative_counts = [sum(counts[:i+1]) / total_count for i in range(len(counts))]
#
## Plot the CDF
#plt.plot(cumulative_counts, marker='o')
#plt.xlabel('Bucket ID')
#plt.ylabel('Cumulative Distribution')
#plt.title('CDF of Counts Across Buckets')
#plt.grid(True)
#plt.show()

import numpy as np
import matplotlib.pyplot as plt

# Read the data from the text file
with open('access_hist.txt', 'r') as file:
    data = [int(line.strip()) for line in file]

# Compute the cumulative sum and normalize to create the CDF
cdf = np.cumsum(data)
cdf_normalized = cdf / cdf[-1]

plt.rcParams.update({'font.size': 12})
labelfontsize=16

# Plot the CDF
plt.plot(cdf_normalized, label='CDF', marker='o', color='black')
plt.xlabel('Accessed Page\'s Number of Sharers', fontsize=labelfontsize)
#plt.ylabel('CDF')
plt.grid(True, which='both', axis='both', linestyle='--', linewidth=0.5, markevery='int')
plt.gca().set_axisbelow(True)
plt.savefig('access_cdf.png', bbox_inches='tight')
#plt.savefig('access_cdf.pdf', bbox_inches='tight')

