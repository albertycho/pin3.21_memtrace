import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches


# Read the file into a pandas DataFrame
df = pd.read_csv("icn_latency_stats_tp.txt")
df.columns = df.columns.str.strip()

# Extract APP as the index of the DataFrame
df.set_index('APP', inplace=True)

colors = ['#70b864','#a06cd5', '#b4daae','#c4a3e5', '#5ba0d9']

def custom_plot(df, cols, title, filename):
    bwidth = 0.2
    
    # Define font sizes
    title_fontsize = 16
    legend_fontsize = 14
    tick_label_fontsize = 12
    annotation_fontsize = legend_fontsize  # Matching legend font size

    ax = df[cols].plot(kind='bar', figsize=(12, 6), color=colors)
    for i, col in enumerate(cols):
        if i >= 2 and i < 4:  # Hatch for third and fourth bars
            for bar in ax.containers[i]:
                bar.set_hatch('//')

    # Annotating values on top of the "lat_all" bars
    for i, rect in enumerate(ax.containers[-1]):
        height = rect.get_height()
        ax.annotate('{}'.format(height),
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),  # 3 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom',
                    fontsize=annotation_fontsize)

    ax.set_title(title, fontsize=title_fontsize)  # Axis title font size
    ax.set_ylabel('Time in links/interconnects (ns)', fontsize=title_fontsize)  # Axis title font size
    ax.grid(axis='y', which='both', linestyle='--', linewidth=0.7, alpha=0.7)
    ax.set_xticklabels(df.index, rotation=0, ha='center', fontsize=tick_label_fontsize)  # Tick labels font size
    ax.set_xlabel("", fontsize=tick_label_fontsize)  # X-axis label font size (even though it's empty)

    # Adjust legend labels
    #labels = [col.replace("cxi_icn", "").replace("base_icn", "") for col in cols]
    labels = [col.replace("cxi_icnlat_", "").replace("base_icnlat_", "").replace("bt","block_transfer").replace("ma","mem_acc") for col in cols]
    labels[len(labels)-1]='AMAT_all'
    
    # Manually create legend patches with appropriate hatching and colors
    patches = [mpatches.Patch(facecolor=colors[i], label=labels[i], edgecolor='black') for i in range(len(cols))]
    for i in range(2, 4):  # Adding hatch for third and fourth bars in the legend
        patches[i].set_hatch('////')
    
    ax.legend(handles=patches, title=None, fontsize=legend_fontsize, ncol=3)  # Legend font size

    plt.tight_layout()
    plt.savefig(filename)




def custom_stacked_plot(df, cols, title, filename):
    # Font size settings
    title_fontsize = 16
    legend_fontsize = 14
    tick_label_fontsize = 12
    annotation_fontsize = legend_fontsize  # Matching legend font size

    # Normalize the values
    df_normalized = df[cols].div(df[cols].sum(axis=1), axis=0)

    # Create stacked bar plot
    ax = df_normalized.plot(kind='bar', stacked=True, figsize=(4, 6), color=colors)

    # Add hatching for the last two sets of bars
    for i, col in enumerate(cols[-2:]):
        idx = cols.index(col)  # get the index of the column in the full columns list
        for bar in ax.containers[idx]:
            bar.set_hatch('//')

    ax.set_title(title, fontsize=title_fontsize)
    ax.set_ylabel('Proportion', fontsize=title_fontsize)
    ax.grid(axis='y', which='both', linestyle='--', linewidth=0.7, alpha=0.7)
    ax.set_xticklabels(df.index, rotation=0, ha='center', fontsize=tick_label_fontsize)
    ax.set_xlabel("", fontsize=tick_label_fontsize)

    # Adjust legend labels
    labels = [col.replace("cxi_icn_", "").replace("base_icn_", "").replace("bt","block_transfer").replace("ma","mem_acc") for col in cols]

    # Manually create legend patches with appropriate colors and hatching
    patches = [mpatches.Patch(facecolor=colors[i], label=labels[i]) for i in range(len(cols))]
    for i in range(len(cols) - 2, len(cols)):  # Adding hatch for the last two bars in the legend
        patches[i].set_hatch('////')
    
    ax.legend(handles=patches, title=None, fontsize=legend_fontsize)

    plt.tight_layout()
    plt.savefig(filename)




# Filter columns starting with "base_icnlat_" and plot them
base_icnlat_cols = [col for col in df.columns if col.startswith('base_icnlat')]
custom_plot(df, base_icnlat_cols, 'Base ICN Latency', 'base_icnlat_plot.png')

# Filter columns starting with "cxi_icnlat_" and plot them
cxi_icnlat_cols = [col for col in df.columns if col.startswith('cxi_icnlat_')]
custom_plot(df, cxi_icnlat_cols, 'StarNUMA ICN Latency', 'cxi_icnlat_plot.png')


base_icn_n_cols = [col for col in df.columns if col.startswith('base_icn_')]
custom_stacked_plot(df, base_icn_n_cols, 'Base Access Types', 'base_icn_count_plot.png')

cxi_icn_n_cols = [col for col in df.columns if col.startswith('cxi_icn_')]
custom_stacked_plot(df, cxi_icn_n_cols, 'StarNUMA Access Types', 'cxi_icn_count_plot.png')



exit(0)



