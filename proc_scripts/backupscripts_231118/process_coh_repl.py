#import os
#import csv
#
## Output file
#outfile = open("collected_coh_repl_stats.csv", "w")
#
#outfile.write("filename, Mem_R, Mem_W, Coh_Invals, Coh_Block_Transfer,\n")
## Get all files in the directory
#filenames = os.listdir(".")
#
## Filter for files that contain "coh_repl" and sort them alphabetically
#filenames = sorted([filename for filename in filenames if "coh_repl" in filename])
#
## Loop over all files
#for filename in filenames:
#    with open(filename, "r") as file:
#        lines = file.readlines()
#
#        # Find the line with "Inst 5900000000"
#        start_index = next((i for i, line in enumerate(lines) if "Inst 5900000000" in line), None)
#        if start_index is None:
#            continue
#
#        # Extract the relevant values
#        mem_reads = next((line.split(":")[1].strip() for line in lines[start_index:] if line.startswith("Mem Reads")), None)
#        mem_writes = next((line.split(":")[1].strip() for line in lines[start_index:] if line.startswith("Mem Writes")), None)
#        coherence_invals = next((line.split(":")[1].strip() for line in lines[start_index:] if line.startswith("Coherence Invals")), None)
#        c_block_transfer = next((line.split(":")[1].strip() for line in lines[start_index:] if line.startswith("C Block Transfer")), None)
#
#        # Save the values to the output file
#        if mem_reads and mem_writes and coherence_invals and c_block_transfer:
#            outfile.write(f"{filename}, {mem_reads}, {mem_writes}, {coherence_invals}, {c_block_transfer}\n")
#
## Close the output file
#outfile.close()

import os
import csv
from collections import defaultdict

# Initialize a dictionary to hold app_name data
app_data = defaultdict(lambda: [None]*8)

# Get all files in the directory
filenames = os.listdir(".")

# Filter for files that contain "coh_repl" and sort them alphabetically
filenames = sorted([filename for filename in filenames if "coh_repl" in filename])

# Loop over all files
for filename in filenames:
    print(filename)
    app_name = filename.split("_")[0]
    size = filename.split("_")[2]
    size = size.split("repl")[1]
    print(app_name)
    print(size)
    with open(filename, "r") as file:
        lines = file.readlines()

        # Find the line with "Inst 5900000000"
        start_index = next((i for i, line in enumerate(lines) if "Inst 5900000000" in line), None)
        if start_index is None:
            continue

        # Extract the relevant values
        mem_reads = next((line.split(":")[1].strip() for line in lines[start_index:] if line.startswith("Mem Reads")), None)
        mem_writes = next((line.split(":")[1].strip() for line in lines[start_index:] if line.startswith("Mem Writes")), None)
        coherence_invals = next((line.split(":")[1].strip() for line in lines[start_index:] if line.startswith("Coherence Invals")), None)
        c_block_transfer = next((line.split(":")[1].strip() for line in lines[start_index:] if line.startswith("C Block Transfer")), None)

        # Save the values to the dictionary
        if size == '12M':
            app_data[app_name][:4] = [mem_reads, mem_writes, coherence_invals, c_block_transfer]
        elif size == '32M':
            app_data[app_name][4:] = [mem_reads, mem_writes, coherence_invals, c_block_transfer]

# Output file
outfile = open("collected_coh_repl_stats.csv", "w")
outfile.write("APP,Mem_R_12M,Mem_W_12M,Coh_Invals_12M,Coh_Block_Transfer_12M,Mem_R_32M,Mem_W_32M,Coh_Invals_32M,Coh_Block_Transfer_32M,\n")
# Write data to file
for app_name, data in app_data.items():
    if None not in data:  # Ensure we have data for both file sizes
        outfile.write(f"{app_name}, {', '.join(data)}\n")

# Close the output file
outfile.close()

