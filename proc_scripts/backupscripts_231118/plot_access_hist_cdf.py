import numpy as np
import matplotlib.pyplot as plt
#sounds good. For the CDFs, please add "CDF" on each y axis title and change both x axis titles to #sharers
def plot_cdf(input_file, output_file):
    # Read the data from the text file
    with open(input_file, 'r') as file:
        data = [int(line.strip()) for line in file]

    # Compute the cumulative sum and normalize to create the CDF
    cdf = np.cumsum(data)
    cdf_normalized = cdf / cdf[-1]
   
    print(cdf_normalized)
    #plt.figure(figsize=(10, 8))
    plt.figure()
    
    plt.rcParams.update({'font.size': 18})
    labelfontsize=22
    


    # Plot the CDF
    #plt.plot(cdf_normalized, label='CDF', marker='o', color='black')
    plt.plot(range(1, len(cdf_normalized)), cdf_normalized[1:], label='CDF', marker='o', color='black')
    #if('access' in input_file):
    #    plt.xlabel('Accessed Page\'s Number of Sharers', fontsize=labelfontsize)
    #if('page' in input_file):
    #    plt.xlabel('Number of Sharers per Page', fontsize=labelfontsize)
    
    plt.xlabel('# sharers', fontsize=labelfontsize)
    plt.ylabel('CDF', fontsize=labelfontsize)
    
    plt.xticks(np.arange(0, len(cdf_normalized), 2))
    plt.ylim(0,1.1) 
    #plt.xlim(1,17) 

    #plt.ylabel('CDF')
    plt.grid(True, which='both', axis='both', linestyle='--', linewidth=0.5)
    plt.gca().set_axisbelow(True)
    plt.savefig(output_file+'.pdf', bbox_inches='tight')
    plt.savefig(output_file+'.png', bbox_inches='tight')
    #plt.savefig(output_file.replace('.png', '.pdf'), bbox_inches='tight')


def plot_pdf(input_file, output_file):
    # Read the data from the text file
    with open(input_file, 'r') as file:
        data = [int(line.strip()) for line in file]

    # Normalize to create the PDF
    pdf_normalized = [x / sum(data) for x in data]

    plt.figure()

    plt.rcParams.update({'font.size': 12})
    labelfontsize=16

    # Plot the PDF as bar plots
    plt.bar(range(len(pdf_normalized)), pdf_normalized, color='#9370db')
    
    if 'access' in input_file:
        plt.xlabel('Accessed Page\'s Number of Sharers', fontsize=labelfontsize)
    if 'page' in input_file:
        plt.xlabel('Number of Sharers per Page', fontsize=labelfontsize)

    #plt.ylabel('PDF')
    plt.grid(True, which='both', axis='both', linestyle='--', linewidth=0.5)
    plt.gca().set_axisbelow(True)
    plt.savefig(output_file+'.pdf', bbox_inches='tight')
    plt.savefig(output_file+'.png', bbox_inches='tight')


# Example usage
plot_cdf('page_hist.txt', 'page_cdf')
plot_cdf('access_hist.txt', 'access_cdf')
plot_pdf('page_hist.txt', 'page_pdf')
plot_pdf('access_hist.txt', 'access_pdf')

