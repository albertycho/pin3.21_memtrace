
import csv
import matplotlib.pyplot as plt

# Open the file and read the data
with open('collected_smart_stats.txt', 'r') as csvfile:
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


plt.rcParams.update({'font.size': 14})
labelfontsize=20

print(base_amats)

# Plot normalized IPCs
plt.figure(figsize=(10, 6))
barwidth=0.5
plt.bar(benchmarks, normalized_IPCs, width=0.5)
plt.axhline(1, color='black', linestyle='--')
plt.grid(axis='y', linestyle='--')
plt.gca().set_axisbelow(True)  # Set grid behind
#plt.xlabel('Benchmark')
plt.ylabel('Normalized IPC', fontsize=labelfontsize)
#plt.title('Normalized IPC for each benchmark')
#plt.xticks(rotation=90)
#plt.show()
plt.savefig('IPC.png', bbox_inches='tight')

# Plot AMATs
plt.figure(figsize=(10, 6))
bar_width = 0.35
index = range(len(benchmarks))

plt.bar(index, base_amats, bar_width, label='Baseline')
plt.bar([i + bar_width for i in index], cxi_amats, bar_width, label='StarNUMA')
plt.grid(axis='y', linestyle='--')
plt.gca().set_axisbelow(True)  # Set grid behind

#plt.xlabel('Benchmark')
plt.ylabel('AMAT (ns)', fontsize=labelfontsize)
#plt.title('AMAT for each benchmark')
plt.xticks([i + bar_width / 2 for i in index], benchmarks)
#plt.xticks([i + bar_width / 2 for i in index], benchmarks, rotation=90)
plt.legend()
#plt.show()
plt.savefig('AMAT.png', bbox_inches='tight')



