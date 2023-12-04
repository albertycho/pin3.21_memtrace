import matplotlib.pyplot as plt
from matplotlib.patches import Patch
import csv
import numpy as np

lats=[80,130,300,180]

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

base_amats=[0]*len(base_values)
cxi_amats =[0]*len(cxi_values)

for i, lat in enumerate(lats):
    print(lat)
    base_amats = [a+(b*lat) for a,b in zip(base_amats, [base[i] for base in base_values])]
    cxi_amats = [a+(b*lat) for a,b in zip(cxi_amats, [cxi[i] for cxi in cxi_values])]

print(base_amats)
print(cxi_amats)

# Plot base bars
bottom_values = [0] * len(base_values)
for i, color in enumerate(colors):
    #ax.bar(indices+bar_width/2 - (bar_width *4) + i * (bar_width), [base[i] for base in base_values], bar_width, color=color, alpha=0.9)
    plotvals = [base[i] for base in base_values]
    ax.bar(indices-bar_width*2.4, [base[i] for base in base_values], bar_width, color=color, alpha=0.9, bottom=bottom_values)
    #bottom_values = [base[i] for base in base_values] + bottom_values
    bottom_values = [a + b for a, b in zip(plotvals, bottom_values)]

## Plot cxi bars
cbottom_values = [0] * len(cxi_values)
for i, color in enumerate(colors):
    plotvals = [cxi[i] for cxi in cxi_values]
    ax.bar(indices -bar_width*1, [cxi[i] for cxi in cxi_values], bar_width, color=color, hatch='//', alpha=0.8, bottom = cbottom_values)
    cbottom_values = [a + b for a, b in zip(plotvals, cbottom_values)]

ax2 = ax.twinx()
ax2.bar(indices+bar_width*1, base_amats, bar_width*0.8, color='black', alpha=1)
ax2.bar(indices+bar_width*2.2, cxi_amats, bar_width*0.8, color='grey', hatch='//', alpha=0.8)

# Plot stacked bars for base values
#bottom_values = [0] * len(base_values[0])
#for i, color in enumerate(colors):
#    values = [base[i] for base in base_values]
#    ax.bar(indices, values, bar_width, color=color, alpha=0.9, bottom=bottom_values)
#    bottom_values = [sum(x) for x in zip(bottom_values, values)]
#
## Plot stacked bars for cxi values
#bottom_values = [0] * len(cxi_values[0])
#for i, color in enumerate(colors):
#    values = [cxi[i] for cxi in cxi_values]
#    ax.bar(indices + bar_width, values, bar_width, color=color, hatch='//', alpha=0.8, bottom=bottom_values)
#    bottom_values = [sum(x) for x in zip(bottom_values, values)]


scaling_factor = 250

# Set the primary y-axis limits
#ax1_ylim = ax.get_ylim()
#ax.set_ylim(ax1_ylim[0], ax1_ylim[1] * scaling_factor)
ax.set_ylim(0,1.1)
#ax2_ylim = ax2.get_ylim()
#ax2.set_ylim(ax2_ylim[0] * scaling_factor, ax2_ylim[1] * scaling_factor)
ax2.set_ylim(0,250*1.1)


# Set the x-ticks and their labels
ax.set_xticks(indices)
ax.set_xticklabels(benchmarks, rotation=0)
ax.set_ylabel('Access Distribution', fontsize=labelfontsize)
ax2.set_ylabel('AMAT(ns)', fontsize=labelfontsize)
# Create legend
legend_elements = [Patch(facecolor=color, label=headers[i+1].split('_')[-1]) for i, color in enumerate(colors)]
legend_elements.append(Patch(facecolor='white', label=''))
legend_elements.append(Patch(facecolor='black', label='Baseline'))
legend_elements.append(Patch(facecolor='grey', label='StarNUMA', hatch='//'))
ax2.legend(handles=legend_elements, loc='lower right', ncol=2)

plt.grid(True, which='both', axis='y', linestyle='--', linewidth=0.5)
plt.gca().set_axisbelow(True)

plt.tight_layout()
plt.savefig('smart_hophist.png', bbox_inches='tight')




exit(0)

