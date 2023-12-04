import matplotlib.pyplot as plt
import numpy as np
import csv
from scipy.stats import gmean

def plot_migration(input_file, output_file):
    # Read the CSV file
    benchmarks = []
    migrate_8k = []
    migrate_2k = []
    no_migration = []

    with open(input_file, 'r') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            benchmarks.append(row['benchmarks'])
            migrate_8k.append(float(row['Migrate 8K']) / float(row['No Migration']))
            migrate_2k.append(float(row['Migrate 2K']) / float(row['No Migration']))
            no_migration.append(1)  # Normalizing to the "No Migration" case, so this value is always 1

    # Compute geometric mean values and append
    migrate_8k_geomean = gmean(migrate_8k)
    migrate_2k_geomean = gmean(migrate_2k)
    no_migration_geomean = gmean(no_migration)

    migrate_8k.append(migrate_8k_geomean)
    migrate_2k.append(migrate_2k_geomean)
    no_migration.append(no_migration_geomean)
    benchmarks.append('MEAN')

    plt.rcParams.update({'font.size': 14})
    labelfontsize = 20
    plt.figure(figsize=(10, 4))

    # Create bar plots
    bar_width = 0.22
    index = np.arange(len(benchmarks))

    bars_migrate_8k = plt.bar(index - bar_width, migrate_8k, bar_width, label='8K pages', color='#9370db')
    bars_migrate_2k = plt.bar(index, migrate_2k, bar_width, label='2K pages', color='#faa460')
    bars_no_migration = plt.bar(index + bar_width, no_migration, bar_width, label='No Migration', color='#dda0dd')

    # Add edges to the bars with the maximum value
    for i in range(len(benchmarks)):
        max_value = max(migrate_8k[i], migrate_2k[i])
        if 'NUMA' not in input_file:
            max_value = max(max_value, no_migration[i])

        if 'NUMA' not in input_file and no_migration[i] == max_value:
            bars_no_migration[i].set_edgecolor('black')
            bars_no_migration[i].set_linewidth(1)
        elif migrate_2k[i] == max_value:
            bars_migrate_2k[i].set_edgecolor('black')
            bars_migrate_2k[i].set_linewidth(1)
        elif migrate_8k[i] == max_value:
            bars_migrate_8k[i].set_edgecolor('black')
            bars_migrate_8k[i].set_linewidth(1)

    plt.grid(axis='y', linestyle='--')
    plt.gca().set_axisbelow(True)  # Set grid behind

    plt.ylim([0, max(max(migrate_8k), max(migrate_2k), max(no_migration)) + 0.2])
    plt.axhline(1, color='black', linestyle='--')
    plt.ylabel('Normalized IPC', fontsize=labelfontsize)
    plt.xticks(index, benchmarks)

    # Create custom legend handles
    legend_handles = [Patch(facecolor='#9370db', label='8K pages'),
                      Patch(facecolor='#faa460', label='2K pages'),
                      Patch(facecolor='#dda0dd', label='No Migration')]
    plt.legend(handles=legend_handles, ncol=3)

    plt.tight_layout()
    plt.savefig(output_file + '.png', bbox_inches='tight')
    plt.savefig(output_file + '.pdf', bbox_inches='tight')

# Remember to call the function with appropriate file paths
# plot_migration('input_file.csv', 'output_file')

