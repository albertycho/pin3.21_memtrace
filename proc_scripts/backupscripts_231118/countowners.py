# Define a dictionary to store the count of each owner
owner_count = {str(i): 0 for i in range(16)}

# Open and read the file
with open('page_owner.txt', 'r') as file:
    lines = file.readlines()
    for line in lines:
        # Split the line to get the owner
        _, owner = line.strip().split()
        # Update the count of the owner
        owner_count[owner] += 1

# Print the count of each owner
for owner, count in owner_count.items():
    print(f'Owner {owner}: {count} occurrence(s)')

