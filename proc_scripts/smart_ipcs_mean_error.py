import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Read the file into a pandas dataframe
data = pd.read_csv('ipcs.txt', sep=':', names=['name', 'ipc'])
print(data)
# Split the name into 'type' and 'postfix'
data[['type', 'postfix']] = data['name'].str.split('_', n=1, expand=True)

# Calculate normalized ipc
data['ipc'] = data['ipc'].astype(float)
normalized = data.pivot(index='postfix', columns='type', values='ipc')
normalized['normalized_ipc'] = normalized['cxi'] / normalized['base']

print()
print(normalized['normalized_ipc'])
print()
print(normalized)

# Calculate the average and standard deviation
average = np.mean(normalized['normalized_ipc'])
std_dev = np.std(normalized['normalized_ipc'])

print(f'Average: {average}')
print(f'Standard Deviation: {std_dev}')

# Plot the error bar
#plt.errorbar(normalized.index, normalized['normalized_ipc'], yerr=std_dev, fmt='o', color='black',
#             ecolor='lightgray', elinewidth=3, capsize=0)
plt.errorbar(['FMI'], [average], yerr=std_dev, fmt='o', color='tab:blue',
             ecolor='lightgray', elinewidth=3, capsize=0)
plt.title('Error Bar of Normalized IPC')
plt.xlabel('Benchmark')
plt.ylabel('Normalized IPC')
#plt.show()
plt.savefig('ipc_errbar.png', bbox_inches='tight')

