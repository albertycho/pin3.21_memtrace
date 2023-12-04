import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

# Read the file into a pandas dataframe
df = pd.read_csv('collected_mem_blocktransfer_stats.txt', skipinitialspace=True)

# Normalize the data
df['norm_factor'] = df['cxi_allacc'] + df['cxi_bt']
for column in ['base_allacc', 'base_bt', 'cxi_allacc', 'cxi_bt', 'cxi_cxl_accs', 'cxi_cxl_bt']:
    df[column] /= df['norm_factor']

# Set benchmark column as index
df.set_index('benchmark', inplace=True)

# Define bar width and positions
bar_width = 0.12
mini_gap = bar_width*0.1
r0 = range(len(df))
r1 = [x - mini_gap for x in r0]
r2 = [x - mini_gap + bar_width for x in r0]
r3 = [x + mini_gap + 2*bar_width for x in r0]
r4 = [x + mini_gap + 3*bar_width for x in r0]

# Plotting
plt.rcParams.update({'font.size': 14})
labelfontsize=20
#colors = ['#dda0dd', '#8fbc8f', '#faa460', '#9370db']
#colors = ['#8ac847', '#8fbc8f','#9370db','PURPLE_100']
colors = ['#70b864', '#b4daae','#a06cd5','#c4a3e5']

fig, ax = plt.subplots(figsize=(12, 6))

ax.grid(True, which='both', axis='y', linestyle='--', linewidth=0.5, zorder=0)

# Plot base bars
ax.bar(r1, df['base_allacc'], width=bar_width, label='base_allacc', edgecolor='black', color=colors[0])
ax.bar(r2, df['base_bt'], width=bar_width, label='base_bt', edgecolor='black', color=colors[1])

# Plot cxi bars
ax.bar(r3, df['cxi_allacc'], width=bar_width, label='cxi_allacc', edgecolor='black', color=colors[0], hatch='/')
ax.bar(r4, df['cxi_bt'], width=bar_width, label='cxi_bt', edgecolor='black', color=colors[1], hatch='/')
ax.bar(r3, df['cxi_cxl_accs'], width=bar_width, label='cxi_cxl_accs', edgecolor='black', color=colors[2], hatch='/')
ax.bar(r4, df['cxi_cxl_bt'], width=bar_width, label='cxi_cxl_bt', edgecolor='black', color=colors[3], hatch='/')

# Labeling the x axis
plt.xticks([r + 1.5*bar_width for r in range(len(df))], df.index)
ax.set_ylabel('Mem Accesses VS. Block Trasnfers', fontsize=labelfontsize)

# Create legend & Show graphic
allacc_patch = mpatches.Patch(facecolor=colors[0], edgecolor='black',label='mem accesses')
bt_patch = mpatches.Patch(facecolor=colors[1], edgecolor='black',label='block transfers')
poolacc_patch = mpatches.Patch(facecolor=colors[2], edgecolor='black',label='pool accesses')
poolbt_patch = mpatches.Patch(facecolor=colors[3], edgecolor='black',label='pool block transfers')
baseline_patch = mpatches.Patch(facecolor='white', edgecolor='black', label='Baseline')
starnuma_patch = mpatches.Patch(facecolor='white', edgecolor='black', hatch='/', label='StarNUMA')

#ax.legend(handles=[allacc_patch, bt_patch, poolacc_patch, poolbt_patch, baseline_patch, starnuma_patch], loc="upper left", bbox_to_anchor=(1,1))
ax.legend(handles=[baseline_patch, starnuma_patch, allacc_patch, bt_patch, poolacc_patch, poolbt_patch], loc="best", ncol=3)
plt.tight_layout()
plt.savefig('mem_blocktransfers.png', bbox_inches='tight')
plt.savefig('mem_blocktransfers.pdf', bbox_inches='tight')




## Plotting
#plt.rcParams.update({'font.size': 14})
#labelfontsize=20
#colors=['#dda0dd', '#8fbc8f', '#faa460', '#9370db']
#
#fig, ax = plt.subplots(figsize=(12, 6))
#
#ax.grid(True, which='both', axis='y', linestyle='--', linewidth=0.5, zorder=0)
## Plot base bars
#ax.bar(r1, df['base_allacc'], width=bar_width, label='base_allacc', edgecolor='black')
#ax.bar(r2, df['base_bt'], width=bar_width, label='base_bt', edgecolor='black')
## Plot cxi bars
#ax.bar(r3, df['cxi_allacc'], width=bar_width, label='cxi_allacc', edgecolor='black')
#ax.bar(r4, df['cxi_bt'], width=bar_width, label='cxi_bt', edgecolor='black')
#ax.bar(r3, df['cxi_cxl_accs'], width=bar_width, label='cxi_cxl_accs', edgecolor='black')
#ax.bar(r4, df['cxi_cxl_bt'], width=bar_width, label='cxi_cxl_bt', edgecolor='black')

# Labeling the x axis
#plt.xticks([r + 1.5*bar_width for r in range(len(df))], df.index)
#
#ax.set_ylabel('Memory Accesses VS. Block Transfers', fontsize=labelfontsize)
## Create legend & Show graphic
#ax.legend(loc="upper left", bbox_to_anchor=(1,1))
#plt.tight_layout()
#plt.savefig('mem_blocktransfers.png', bbox_inches='tight')
#plt.savefig('mem_blocktransfers.pdf', bbox_inches='tight')       


exit(0)

