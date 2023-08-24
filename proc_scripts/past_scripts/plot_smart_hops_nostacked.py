import matplotlib.pyplot as plt
from matplotlib.patches import Patch
import csv
import numpy as np

# Load the data
data = []
with open('collected_hop_stats.txt', 'r') as file:
    reader = csv.reader(file)
    headers = next(reader)  # Skip header
    data = list(reader)

benchmarks = [row[0] for row in data]

# Prepare the data for plotting
base_values = [[int(val) for val in row[1:5]] for row in data]
cxi_values = [[int(val) for val in row[5:9]] for row in data]

# Normalize the counts to PDFs
base_values = [[val/sum(group) for val in group] for group in base_values]
cxi_values = [[val/sum(group) for val in group] for group in cxi_values]

bar_width = 0.067  # The width of the bars
spacing = 0.02  # The spacing between bars
n_benchmarks = len(benchmarks)
indices = np.arange(n_benchmarks)

# Colors for the bars
colors = ['tab:green', 'tab:blue', 'tab:red', 'tab:purple']

plt.rcParams.update({'font.size': 14})
labelfontsize=20
# Create the figure and the axes
fig, ax = plt.subplots(figsize=(10, 5))

# Plot base bars
for i, color in enumerate(colors):
    ax.bar(indices+bar_width/2 - (bar_width *4) + i * (bar_width), [base[i] for base in base_values], bar_width, color=color, alpha=0.9)

## Plot cxi bars
for i, color in enumerate(colors):
    ax.bar(indices +bar_width/2+ i * (bar_width), [cxi[i] for cxi in cxi_values], bar_width, color=color, hatch='//', alpha=0.8)

# Set the x-ticks and their labels
ax.set_xticks(indices)
ax.set_xticklabels(benchmarks, rotation=0)
plt.ylabel('Access Distribution', fontsize=labelfontsize)

# Create legend
legend_elements = [Patch(facecolor=color, label=headers[i+1].split('_')[-1]) for i, color in enumerate(colors)]
legend_elements.append(Patch(facecolor='white', label=''))
legend_elements.append(Patch(facecolor='grey', label='Baseline'))
legend_elements.append(Patch(facecolor='grey', label='StarNUMA', hatch='//'))
ax.legend(handles=legend_elements, loc='upper right', ncol=1)

plt.grid(True, which='both', axis='y', linestyle='--', linewidth=0.5)
plt.gca().set_axisbelow(True)

plt.tight_layout()
plt.savefig('smart_hophist.png', bbox_inches='tight')




exit(0)

