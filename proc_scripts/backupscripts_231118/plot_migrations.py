import matplotlib.pyplot as plt
import numpy as np
import csv
from scipy.stats import gmean
from matplotlib.patches import Patch

def plot_migration(input_file, output_file):
    # Read the CSV file
    benchmarks = []
    migrate_32k = []
    migrate_8k = []
    migrate_2k = []
    migrate_512 = []
    no_migration = []

    with open(input_file, 'r') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            benchmarks.append(row['benchmark'])
            if(float(row['0'])==0):
                migrate_32k.append(0)
                migrate_8k.append(0)
                migrate_2k.append(0)
                migrate_512.append(0)
                no_migration.append(0)
            else:
                migrate_32k.append(float(row['32K']) / float(row['0']))
                migrate_8k.append(float(row['8K']) / float(row['0']))
                migrate_2k.append(float(row['2K']) / float(row['0']))
                migrate_512.append(float(row['512']) / float(row['0']))
                no_migration.append(1)  # Normalizing to "No Migration"

    benchmarks.append('')
    migrate_32k.append(0)
    migrate_8k.append(0)
    migrate_2k.append(0)
    migrate_512.append(0)
    no_migration.append(0)

    # Function to filter out zeros
    def filter_zeros(data):
        return [num for num in data if num != 0]
    
    # Compute geometric mean values for each list, excluding zeros
    migrate_32k_geomean = gmean(filter_zeros(migrate_32k))
    migrate_8k_geomean = gmean(filter_zeros(migrate_8k))
    migrate_2k_geomean = gmean(filter_zeros(migrate_2k))
    migrate_512_geomean = gmean(filter_zeros(migrate_512))
    no_migration_geomean = gmean(filter_zeros(no_migration))


    ## Compute geometric mean values and append
    #migrate_32k_geomean = gmean(migrate_32k)
    #migrate_8k_geomean = gmean(migrate_8k)
    #migrate_2k_geomean = gmean(migrate_2k)
    #migrate_512_geomean = gmean(migrate_512)
    #no_migration_geomean = gmean(no_migration)

    migrate_32k.append(migrate_32k_geomean)
    migrate_8k.append(migrate_8k_geomean)
    migrate_2k.append(migrate_2k_geomean)
    migrate_512.append(migrate_512_geomean)
    no_migration.append(no_migration_geomean)
    benchmarks.append('MEAN')

    color_list = ['#DDA0DD','#8FBC8F','#FAA460','#CD5C5C','#9370DB','#6495ED','#66CDAA']

    plt.rcParams.update({'font.size': 14})
    labelfontsize = 20
    #plt.figure(figsize=(12, 6))
    plt.figure(figsize=(10, 4))

    # Create bar plots
    bar_width = 0.15
    index = np.arange(len(benchmarks))

    bars_migrate_32k = plt.bar(index - bar_width * 2, migrate_32k, bar_width, label='32K pages', color=color_list[0])
    bars_migrate_8k = plt.bar(index - bar_width, migrate_8k, bar_width, label='8K pages', color=color_list[1])
    bars_migrate_2k = plt.bar(index, migrate_2k, bar_width, label='2K pages', color=color_list[2])
    bars_migrate_512 = plt.bar(index + bar_width, migrate_512, bar_width, label='512 pages', color=color_list[3])
    bars_no_migration = plt.bar(index + bar_width * 2, no_migration, bar_width, label='No Migration', color=color_list[4])


    max_bar_width_increase = -0.017  # Additional width for maximum bars
    bold_linewidth = 2

    for i in range(len(benchmarks)-1):
        max_value = max(migrate_32k[i], migrate_8k[i], migrate_2k[i], migrate_512[i], no_migration[i])

        if no_migration[i] == max_value:
            bars_no_migration[i].set_edgecolor('black')
            bars_no_migration[i].set_linewidth(bold_linewidth)
            bars_no_migration[i].set_width(bar_width + max_bar_width_increase)

        elif migrate_512[i] == max_value:
            bars_migrate_512[i].set_edgecolor('black')
            bars_migrate_512[i].set_linewidth(bold_linewidth)
            bars_migrate_512[i].set_width(bar_width + max_bar_width_increase)
        elif migrate_2k[i] == max_value:
            bars_migrate_2k[i].set_edgecolor('black')
            bars_migrate_2k[i].set_linewidth(bold_linewidth)
            bars_migrate_2k[i].set_width(bar_width + max_bar_width_increase)

        elif migrate_8k[i] == max_value:
            bars_migrate_8k[i].set_edgecolor('black')
            bars_migrate_8k[i].set_linewidth(bold_linewidth)
            bars_migrate_8k[i].set_width(bar_width + max_bar_width_increase)


        elif migrate_32k[i] == max_value:
            bars_migrate_32k[i].set_edgecolor('black')
            bars_migrate_32k[i].set_linewidth(bold_linewidth)
            bars_migrate_32k[i].set_width(bar_width + max_bar_width_increase)

    
    plt.grid(axis='y', linestyle='--')
    plt.gca().set_axisbelow(True)

    #plt.ylim([0, max(max(migrate_32k), max(migrate_8k), max(migrate_2k), max(migrate_512), max(no_migration)) + 0.2])
    plt.ylim([0, 2])
    plt.axhline(1, color='black', linestyle='--')
    plt.ylabel('Normalized IPC', fontsize=labelfontsize)
    plt.xticks(index, benchmarks)

    # Create custom legend handles
    legend_handles = [
        Patch(facecolor=color_list[0], label='32K pages'),
        Patch(facecolor=color_list[1], label='8K pages'),
        Patch(facecolor=color_list[2], label='2K pages'),
        Patch(facecolor=color_list[3], label='512 pages'),
        Patch(facecolor=color_list[4], label='No Migration')
    ]
    #plt.legend(handles=legend_handles, ncol=3, loc='lower left', framealpha=0.99)
    #plt.legend(handles=legend_handles, ncol=3, loc='best', framealpha=0.99, fontsize=15)
    plt.legend(handles=legend_handles, ncol=3, loc='best', framealpha=0.99, fontsize=14.5)

    plt.tight_layout()
    plt.savefig(output_file + '.png', bbox_inches='tight')
    plt.savefig(output_file + '.pdf', bbox_inches='tight')

# Remember to call the function with appropriate file paths
plot_migration('SNUMA_migrations.csv', 'eval_starNUMA_migration')
plot_migration('baseline_migrations.csv', 'eval_baseline_migration')

