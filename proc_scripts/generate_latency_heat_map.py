import numpy as np
import matplotlib.pyplot as plt

def get_lat(x,y):
    if(x==y):
        return 80
    xgroup=x>>2
    ygroup=y>>2
    if(xgroup==ygroup):
        return 130

    return 360

# Create a 16x16 grid
grid_size = 16
heatmap = np.zeros((grid_size, grid_size))

# Assign values to specific (x, y) pairs
# For example, let's assign higher values to a few specific coordinates
#coordinates = [(3, 6), (8, 10), (12, 2), (14, 14)]
#values = [10, 20, 15, 25]

#for (x, y), value in zip(coordinates, values):
#    heatmap[y, x] = value

for x in range(grid_size):
    for y in range(grid_size):
        heatmap[y,x]=get_lat(x,y)
    

# Create a heatmap using Matplotlib
plt.figure(figsize=(8, 8))
#plt.imshow(heatmap, cmap='Blues', interpolation='none')
plt.imshow(heatmap, cmap='PuRd', interpolation='nearest')
plt.imshow(heatmap, cmap='YlOrRd', interpolation='none')
for y in range(grid_size):
    for x in range(grid_size):
        if(heatmap[y, x] > 300):
            plt.text(x, y, str(int(heatmap[y, x])), color='white', ha='center', va='center')
        else:
            plt.text(x, y, str(int(heatmap[y, x])), color='black', ha='center', va='center')


for y in range(grid_size + 1):
    plt.axhline(y - 0.5, color='black', linewidth=0.5)
for x in range(grid_size + 1):
    plt.axvline(x - 0.5, color='black', linewidth=0.5)
#plt.colorbar(label='Value')
#plt.grid(color='black', linewidth=0.5)

#plt.title('Heatmap with Assigned Values')
#plt.xlabel('X')
#plt.ylabel('Y')
plt.xticks(range(grid_size), fontsize=12)
plt.yticks(range(grid_size), fontsize=12)
plt.tick_params(axis='both', which='both', length=0)
plt.gca().invert_yaxis()  # Invert y-axis to match array indexing
#plt.show()
plt.savefig('latency_heatmap.png', bbox_inches='tight')
plt.savefig('latency_heatmap.pdf', bbox_inches='tight')
