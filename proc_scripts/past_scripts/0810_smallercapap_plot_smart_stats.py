from scipy.stats import gmean
import csv
import matplotlib.pyplot as plt
import numpy as np
# Open the file and read the data
#with open('collected_smart_stats.txt', 'r') as csvfile:
with open('collected_smart_stats.csv', 'r') as csvfile:
    reader = csv.reader(csvfile)
    next(reader)  # Skip the header
    benchmarks = []
    normalized_IPCs = []
    base_amats = []
    cxi_amats = []
    for row in reader:
        print(row)
        #if len(row) == 5:  # Make sure we have a full row of data
        benchmarks.append(row[0])
        base_IPC = float(row[1])
        cxi_IPC = float(row[2])
        normalized_IPCs.append(cxi_IPC / base_IPC if base_IPC else 0)  # Avoid division by zero
        base_amats.append(float(row[3]))
        cxi_amats.append(float(row[4]))

# Calculate the geometric mean for normalized IPCs
geomean_normalized_IPCs = gmean(normalized_IPCs)
benchmarks.append('MEAN')
normalized_IPCs.append(geomean_normalized_IPCs)

# Calculate the arithmetic mean for AMATs
arithmetic_mean_base_amats = np.mean(base_amats)
arithmetic_mean_cxi_amats = np.mean(cxi_amats)
base_amats.append(arithmetic_mean_base_amats)
cxi_amats.append(arithmetic_mean_cxi_amats)


plt.rcParams.update({'font.size': 14})
labelfontsize=20

print(base_amats)

# Plot normalized IPCs
plt.figure(figsize=(10, 4))
barwidth=0.5
plt.bar(benchmarks, normalized_IPCs, width=0.5, color='#9370db')
plt.axhline(1, color='black', linestyle='--')
plt.grid(axis='y', linestyle='--')
plt.gca().set_axisbelow(True)  # Set grid behind
#plt.xlabel('Benchmark')
plt.ylabel('Normalized IPC', fontsize=labelfontsize)
# Add annotation for geometric mean
mean_x_position = benchmarks.index('MEAN')
plt.text(mean_x_position, geomean_normalized_IPCs + 0.02, f'{geomean_normalized_IPCs:.2f}', ha='center')
#plt.title('Normalized IPC for each benchmark')
#plt.xticks(rotation=90)
#plt.show()
plt.savefig('smallpool_IPC.png', bbox_inches='tight')
plt.savefig('smallpool_IPC.pdf', bbox_inches='tight')

# Plot AMATs
plt.figure(figsize=(10, 4))
bar_width = 0.1
index = range(len(benchmarks))

plt.bar([i-bar_width*1.25 for i in index], base_amats, bar_width, label='Baseline', color='#ce4257')
plt.bar([i + bar_width*1.25 for i in index], cxi_amats, bar_width, label='StarNUMA', color='#ce4257', hatch='//')
plt.grid(axis='y', linestyle='--')
plt.gca().set_axisbelow(True)  # Set grid behind

#plt.xlabel('Benchmark')
plt.ylabel('AMAT (ns)', fontsize=labelfontsize)
#plt.title('AMAT for each benchmark')
plt.xticks([i + bar_width / 2 for i in index], benchmarks)
#plt.xticks([i + bar_width / 2 for i in index], benchmarks, rotation=90)
plt.legend()
#plt.show()
plt.savefig('smallpool_AMAT.png', bbox_inches='tight')
plt.savefig('smallpool.pdf', bbox_inches='tight')



