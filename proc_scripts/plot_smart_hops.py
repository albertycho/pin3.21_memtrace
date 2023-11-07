import matplotlib.pyplot as plt
from matplotlib.patches import Patch
import csv
import numpy as np

lats=[80,130,360,180,413,280]

# Load the data
data = []
with open('collected_hop_stats.txt', 'r') as file:
    reader = csv.reader(file)
    headers = next(reader)  # Skip header
    data = list(reader)

benchmarks = [row[0] for row in data]

# Prepare the data for plotting
base_values = [[int(val) for val in row[1:7]] for row in data]
cxi_values = [[int(val) for val in row[7:13]] for row in data]

# Normalize the counts to PDFs
base_values = [[val/sum(group) for val in group] for group in base_values]
cxi_values = [[val/sum(group) for val in group] for group in cxi_values]

bar_width = 0.1  # The width of the bars
spacing = 0.02  # The spacing between bars
n_benchmarks = len(benchmarks)
indices = np.arange(n_benchmarks)

# Colors for the bars
#colors = ['tab:green', 'tab:blue', 'tab:red', 'tab:purple']
#colors=['#dda0dd', '#8fbc8f', '#faa460', '#9370db']
colors=['#dda0dd', '#8fbc8f', '#faa460', '#9370db','lightyellow','aliceblue']
plt.rcParams.update({'font.size': 14})
labelfontsize=20
# Create the figure and the axes
#fig, ax = plt.subplots(figsize=(10, 5))
fig, (ax1, ax2) = plt.subplots(nrows=2, figsize=(3, 6))  # Adjust the figure size as needed

base_amats=[0]*len(base_values)
cxi_amats =[0]*len(cxi_values)

for i, lat in enumerate(lats):
    print(lat)
    base_amats = [a+(b*lat) for a,b in zip(base_amats, [base[i] for base in base_values])]
    cxi_amats = [a+(b*lat) for a,b in zip(cxi_amats, [cxi[i] for cxi in cxi_values])]

print(base_amats)
print(cxi_amats)
base_amat_mean = np.mean(base_amats)
cxi_amat_mean = np.mean(cxi_amats)
amat_reduction = cxi_amat_mean / base_amat_mean
print(amat_reduction)

twohop_ratios=[]
#print(base_values)
for bv in base_values:
#    print(bv[2])
    twohop_ratios.append(bv[2])

print(twohop_ratios)
twohoprat_mean = np.mean(twohop_ratios)
print(twohoprat_mean)

ctwohop_ratios=[]
#print(base_values)
for cv in cxi_values:
#    print(bv[2])
    ctwohop_ratios.append(cv[3])

print(ctwohop_ratios)
ctwohoprat_mean = np.mean(ctwohop_ratios)
print(ctwohoprat_mean)

# Plot base bars (stacked bars)
bottom_values = [0] * len(base_values)
for i, color in enumerate(colors):
    plotvals = [base[i] for base in base_values]
    ax1.bar(indices - bar_width * 1.25, plotvals, bar_width, color=color, alpha=1, bottom=bottom_values, edgecolor='black')
    bottom_values = [a + b for a, b in zip(plotvals, bottom_values)]

# Plot cxi bars
cbottom_values = [0] * len(cxi_values)
for i, color in enumerate(colors):
    plotvals = [cxi[i] for cxi in cxi_values]
    ax1.bar(indices + bar_width * 1.25, plotvals, bar_width, color=color, hatch='//', alpha=1, bottom=cbottom_values, edgecolor='black')
    cbottom_values = [a + b for a, b in zip(plotvals, cbottom_values)]

# Plot amat bars
ax2.bar(indices - bar_width * 1.25, base_amats, bar_width * 0.8, color='#ce4257', alpha=1,edgecolor='black')
ax2.bar(indices + bar_width * 1.25, cxi_amats, bar_width * 0.8, color='#ce4257', hatch='//', alpha=0.8, edgecolor='black')

# Adjust layout to prevent overlap
plt.tight_layout()


# Set the primary y-axis limits
#ax1_ylim = ax.get_ylim()
#ax.set_ylim(ax1_ylim[0], ax1_ylim[1] * scaling_factor)
ax1.set_ylim(0,1.1)
#ax2_ylim = ax2.get_ylim()
#ax2.set_ylim(ax2_ylim[0] * scaling_factor, ax2_ylim[1] * scaling_factor)
ax2.set_ylim(0,300*1.1)
#ax1.set_xticks([])
ax1.set_xticklabels([])
ax1.grid(True, which='both', axis='y', linestyle='--', linewidth=0.5, zorder=0)
# Set the x-ticks and their labels
ax2.set_xticks(indices)
ax1.set_ylabel('Access\nDistribution', fontsize=labelfontsize)
ax2.set_ylabel('Estimated\nAMAT (ns)', fontsize=labelfontsize)
ax2.set_xticklabels(benchmarks, rotation=0)
#ax2.set_ylabel('Average Memory \nAccess Time (ns)', fontsize=labelfontsize, labelpad=10)
# Create legend
legend_elements = [Patch(facecolor=color, label=headers[i+1].split('_')[-1], edgecolor='black') for i, color in enumerate(colors)]
#legend_elements[3].label='CXL'
legend_elements[3] = Patch(facecolor=legend_elements[3].get_facecolor(), label='Pool', edgecolor='black')
legend_elements[0] = Patch(facecolor=legend_elements[0].get_facecolor(), label='Local', edgecolor='black')
legend_elements[4] = Patch(facecolor=legend_elements[4].get_facecolor(), label='BT_NoPool', edgecolor='black')
legend_elements[5] = Patch(facecolor=legend_elements[5].get_facecolor(), label='BT_Pool', edgecolor='black')
#legend_elements.append(Patch(facecolor='white', alpha=0, label=''))
legend_elements.append(Patch(facecolor='grey', label='Baseline',edgecolor='black'))
legend_elements.append(Patch(facecolor='grey', label='StarNUMA', hatch='//',edgecolor='black'))

#ax1.legend(handles=legend_elements, loc='upper right', ncol=3)
#ax1.legend(handles=legend_elements, loc='lower right', ncol=4)
ax1.legend(handles=legend_elements, ncol=4, loc='center', bbox_to_anchor=(0.5,1.15))
legend_elements.append(Patch(facecolor='white', alpha=0, label=''))
legend_elements.append(Patch(facecolor='black', label='Baseline'))
legend_elements.append(Patch(facecolor='grey', label='StarNUMA', hatch='//'))
#ax2.legend(handles=legend_elements, loc='lower left', ncol=2)

plt.grid(True, which='both', axis='y', linestyle='--', linewidth=0.5)
plt.gca().set_axisbelow(True)

#plt.tight_layout()
plt.savefig('hopdist_amat.png', bbox_inches='tight')
plt.savefig('hopdist_amat.pdf', bbox_inches='tight')




exit(0)

