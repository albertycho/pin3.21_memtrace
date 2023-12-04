import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np

# Read the CSV file into a DataFrame
df = pd.read_csv("collected_coh_repl_stats.csv")

# Set the index to be the APP column
df.set_index('APP', inplace=True)

#print(df)
#print(df.columns)

# Normalize the columns
#for column in df.columns:
#    if not 'Mem_R_12M' in column:
#        df[column] = df[column] / df['Mem_R_12M']
#df['Mem_R_12M']=1

#norm_val = df['Mem_R_12M'] + df['Mem_W_12M']
norm_val = df['Mem_R_12M'] + df['Mem_W_12M']+df['Coh_Invals_12M']+df['Coh_Block_Transfer_12M']
dec_ratio_in32M = (df['Mem_R_32M'] + df['Mem_W_32M']+df['Coh_Invals_32M']+df['Coh_Block_Transfer_32M']) / norm_val
print(dec_ratio_in32M)
cache_action_ratio =  ( df['Coh_Invals_12M']+df['Coh_Block_Transfer_12M'] ) / norm_val
print(cache_action_ratio)
armean = np.mean(cache_action_ratio)
print(armean)

for column in df.columns:
    #if not 'Mem_R_12M' in column:
    df[column] = df[column] / norm_val



#df['Mem_R_12M']=1



# Separate the dataframe into two dataframes, one for 12M data and one for 32M data
df_12M = df[['Mem_R_12M', 'Mem_W_12M', 'Coh_Invals_12M', 'Coh_Block_Transfer_12M']]
df_32M = df[['Mem_R_32M', 'Mem_W_32M', 'Coh_Invals_32M', 'Coh_Block_Transfer_32M']]

color_mr='skyblue'
color_mw='tab:blue'
color_ci='salmon'
color_cb='tab:orange'#'red'
fig, ax = plt.subplots(figsize=(10,4))
fs=14
plt.rcParams['font.size'] = fs
barw=0.3

# Create a bar plot for 12M data
df_12M[['Mem_R_12M', 'Mem_W_12M', 'Coh_Invals_12M', 'Coh_Block_Transfer_12M']].plot.bar(stacked=True, position=1, width=barw, ax=ax, color=[color_mr,color_mw, color_ci, color_cb], align='center', label=['Mem_R', 'Mem_W','C_invals','C_block_Transfer'])#, edgecolor='black')
#df_12M[['Coh_Invals_12M', 'Coh_Block_Transfer_12M']].plot.bar(stacked=True, position=1, width=barw, ax=ax, color=[color_ci,color_cb], align='center')

# Create a bar plot for 32M data
df_32M[['Mem_R_32M', 'Mem_W_32M', 'Coh_Invals_32M', 'Coh_Block_Transfer_32M']].plot.bar(stacked=True, position=0, width=barw, ax=ax, color=[color_mr,color_mw, color_ci, color_cb], align='center', hatch='//', alpha=0.7)
#df_32M[['Coh_Invals_32M', 'Coh_Block_Transfer_32M']].plot.bar(stacked=True, position=-1, width=barw, ax=ax, color=[color_ci,color_cb], align='center', hatch='//', alpha=0.7)


# Custom Legend
handles = [mpatches.Patch(facecolor=color_mr,  label='Mem Rd'),
           mpatches.Patch(facecolor=color_mw,  label='Mem Wr'),
           mpatches.Patch(facecolor=color_ci,  label='C Invals'),
           mpatches.Patch(facecolor=color_cb,  label='C Block Trans'),
           #mpatches.Patch(facecolor='white', alpha=0),
           mpatches.Patch(facecolor='black' , label='8  MB LLC'),
           mpatches.Patch(facecolor='grey', alpha=0.5, hatch='//', label='32MB LLC')]

ax.legend(handles=handles, ncols=3, loc='upper right')
# Display the plot
#plt.show()
ax.set_xlim([-0.5, len(df.index)-0.5])
ax.set_ylim([0, 1.1])
ax.set_axisbelow(True)
plt.yticks(np.arange(0, 1.2, 0.1), fontsize=14)
#ax.set_yticklabels([(f'{i:.1f}' if i in [0,0.2,0.4,0.6,0.8,1.0] else '') for i in np.arange(0, 1.2, 0.1)])
ax.set_yticklabels([f'{i:.1f}' if (((i % 0.2) < 0.01) or ((i % 0.2) > 0.19 ))else '' for i in np.arange(0, 1.2, 0.1)])

#for i in np.arange(0, 1.2, 0.1):
#    print(i)
#    print(i%0.2)
#    #if i % 0.2 == 0:
#    #    print(i)

ax.grid(zorder=0, axis='y', markevery=0.1, alpha=0.5, linestyle='--')
plt.xticks(rotation=0, fontsize=fs)
#plt.figure(figsize=(20, 10))
#ax.set_ylabel("'Memory Accesses' and \n'Cache to Cache' Action")
ax.set_xlabel('')
#figsize=(6,2)
fig.savefig('coherence_plot.png', bbox_inches='tight')
fig.savefig('coherence_plot.pdf', bbox_inches='tight')


