import sys
import os
import json
import math
from math import log, log10
import subprocess
from subprocess import Popen, PIPE
import matplotlib
import matplotlib.font_manager as fm
from matplotlib import rc
import matplotlib.pyplot as plt
import itertools
import numpy as np
from locale import format_string


#------------------------------------------------------------------------------
# Config parameters
#------------------------------------------------------------------------------
benchmark = 'mmm'
function = 'mmm'
input = '20'
double_precision = 1
config = 'SB'

#------------------------------------------------------------------------------
# Directories and paths to lli and clang
#------------------------------------------------------------------------------

SRC_DIR = 'src'
BIN_DIR = 'bin'
OUTPUT_DIR = 'output'
CONFIGS_DIR = 'configs'
LLI_PATH = '/local/bin'
CLANG_PATH = ''




#------------------------------------------------------------------------------
# Auxiliary data structures and settings used for plotting
#------------------------------------------------------------------------------

comp_nodes = {'fadd' : 0, 'fmul':1, 'ffma':2, 'fdiv':3, 'fshuffle' : 4,
    'fblend' : 5, 'fmov':6, 'register':7, 'l1_load' : 8, 'l1_store':9, 'l2':10,
    'llc' : 11, 'mem':12 }

buffer_nodes = {'rs' : 13, 'rob':14, 'lb':15, 'sb' : 16, 'lfb' : 17}

nodes = {}
nodes.update(comp_nodes)
nodes.update(buffer_nodes)
nodes.update({'no_node':-1})


hw_parameters = {'throughput':0, 'latency':1, 'rs':2, 'rob':3, 'lb':4, 'sb':5,
'lfb':6, 'register_file_size':7, 'l1_cache_size':8, 'l2_cache_size':9,
'llc_cache_size':10, 'fetch_bw':11, 'memory_model':12, 'memory-word-size':13,
'cache-line-size':14, 'constraint-ports':15, 'constraint-agus':16,
'parallel-issue':17, 'mem-access-granularity':18}


#font = fm.FontProperties(
#        family = 'Gill Sans', fname = 'GillSans.ttc')


background_color =(0.85,0.85,0.85)
dark_grey_color = (0.298, 0.298, 0.298)
grid_color = 'white'
matplotlib.rc('axes', facecolor = background_color)
matplotlib.rc('axes', edgecolor = grid_color)
matplotlib.rc('axes', linewidth = 1.2)
matplotlib.rc('axes', grid = True )
matplotlib.rc('axes', axisbelow = True)
matplotlib.rc('grid',color = grid_color)
matplotlib.rc('grid',linestyle='-' )
matplotlib.rc('grid',linewidth=0.7 )
matplotlib.rc('xtick.major',size =0 )
matplotlib.rc('xtick.minor',size =0 )
matplotlib.rc('ytick.major',size =0 )
matplotlib.rc('ytick.minor',size =0 )
#matplotlib.rc('font', family='serif')

X_MIN=0.001
X_MAX=30.0
Y_MIN=0.1
Y_MAX=14.0

PEAK_PERF_LABELS=['Peak Performance']
PEAK_BW_LABELS = ['L1Store', 'L1Load', 'L2', 'LLC','DRAM']
X_LABEL="Operational Intensity [Flops/Byte]"
Y_LABEL="Performance [Flops/Cycle]"
UTILIZATION_ROOFS=1
OVERLAP_ROOFS=1
AXIS_ASPECT_RATIO=1

colors = ['black','black', 'black','black','black','black','black','black','black','black','black','black','black','#55141C']



#------------------------------------------------------------------------------
# Functions for analyzing output file and generating the extended roofline plot
#------------------------------------------------------------------------------

# Return the total number of flops from outputfile
def get_total_flops_outputfile(outputfile):

    data = list()
    f = open(outputfile,'r') 
    for line in f:
        if "FLOPS" in line:
            data.append(line.split())
    flops = float(data[0][2])
    return flops


# Get the value of the hw parameter 'bottleneck' for node 'node' from the JSON
# config file
def get_config_value(config, bottleneck, node=0):
    
    if bottleneck == hw_parameters['throughput']:
        parameters =  config['execution-units-throughput'].split(',')
        for i, element in enumerate(parameters):
            parameters[i] = element.replace('{','').replace('}','')
            
        if node < comp_nodes['register']:
            if double_precision:
                 return parameters[node*2+1]
            else:
                return parameters[node*2]
        else:
             return parameters[node+comp_nodes['register']]
        
    if bottleneck == hw_parameters['latency']:
        parameters =  config['execution-units-latency'].split(',')
        for i, element in enumerate(parameters):
            parameters[i] = element.replace('{','').replace('}','')
        
        if node < comp_nodes['register']:
            if double_precision:
                 return parameters[node*2+1]
            else:
                return parameters[node*2]
        else:
             return parameters[node+comp_nodes['register']]
    
    if bottleneck == hw_parameters['parallel-issue']:
        parameters =  config['execution-units-parallel-issue'].split(',')
        for i, element in enumerate(parameters):
            parameters[i] = element.replace('{','').replace('}','')
        
        if node < comp_nodes['register']:
            if double_precision:
                 return parameters[node*2+1]
            else:
                return parameters[node*2]
        else:
             return parameters[node+comp_nodes['register']]
    
    if bottleneck==hw_parameters['mem-access-granularity']:
        if node < 0:
            sys.exit("Cannot get mem access granularity for node "+str(node))
        parameters =  config['mem-access-granularity'].split(',')
        for i, element in enumerate(parameters):
            parameters[i] = element.replace('{','').replace('}','')
        return parameters[node]
         
    if bottleneck == hw_parameters['rs']:
        return config["reservation-station-size"]
    if bottleneck == hw_parameters['rob']:
        return config["reorder-buffer-size"]
    if bottleneck == hw_parameters['lb']:
        return config["load-buffer-size"]
    if bottleneck == hw_parameters['sb']:
        return config["store-buffer-size"] 
    if bottleneck == hw_parameters['lfb']:
        return config["line-fill-buffer-size"] 
    if bottleneck == hw_parameters['l1_cache_size']:    
        return config["l1-cache-size"] 
    if bottleneck == hw_parameters['l2_cache_size']:
        return config["l2-cache-size"] 
    if bottleneck == hw_parameters['llc_cache_size']:
        return config["llc-cache-size"] 
    if bottleneck == hw_parameters['fetch_bw']:
        return config["instruction-fetch-bandwidth"]
    if bottleneck ==  hw_parameters['memory_model']:
        return config["x86-memory-model"]
    if bottleneck == hw_parameters['memory-word-size']:
        return config["memory-word-size"]
    if bottleneck == hw_parameters['cache-line-size']:
        return config["cache-line-size"]
    if bottleneck == hw_parameters['memory-word-size']:
        return config["memory-word-size"]     
    if bottleneck == hw_parameters['register_file_size']:
        return config['register-file-size']    


# Parse outputfile and config.json to collect all the data necessary to generate
# the extended roofline plot. Creating all the intermediate files is not really
# necessary, we could parse the files and use the values directly (I create
# these intermediate files to )
def write_data_roofline_plot(config_nr,  outputfile):

# These lists contain, for each node defined in 'nodes', the corresponding
# property, i.e., numer of nodes (n_ops), issue span (issue_span).
# See Pag. from [] for a detailed description of these properties.

    n_ops = list()
    issue_span = list()
    total_pan = list()
    total_pan_with_stalls = list()
    stalls_span = list()
    stalls_performance_bound = list()
    resource_issue_stall_span = list()
    overlaps_without_stalls = list()
    overlaps_with_stalls = list()
    throughputs = list()
    parallel_issue = list()
    total_throughputs = list()

    invert_comp_nodes = dict((v,k) for k,v in comp_nodes.items())
    invert_buffer_nodes = dict((v,k) for k,v in buffer_nodes.items())
    
    serie=os.path.basename(os.path.normpath(outputfile))
    
    data = [line.split() for line in open(os.path.expanduser(outputfile),'r')]
    
    statistics_info_row = 0
    res_issue_stall_span_info_row = 0
    res_res_without_stalls_overlap_info_row = 0
    res_res_with_stalls_overlap_info_row = 0
    total_line = 0
    stalls_span_row = 0
    stalls_performance_bound_row = 0
    
    for i, line in enumerate(data):
      if "Statistics"in line and statistics_info_row==0:
        statistics_info_row=i
      if "Resource-Resource" in line and "Overlap" in line and "without" in line and res_res_without_stalls_overlap_info_row==0:
        res_res_without_stalls_overlap_info_row=i
      if "Resource-Resource" in line and "Overlap" in line and not("without" in line) and res_res_with_stalls_overlap_info_row==0:
          res_res_with_stalls_overlap_info_row=i
      if "TOTAL" in line and "RESOURCE" not in line and total_line==0:
          total_line = i
      if "Stall" in line and "Cycles" in line and stalls_span_row==0:
          stalls_span_row=i
      if "ResourceIssue-Stall" in line and res_issue_stall_span_info_row==0:
          res_issue_stall_span_info_row=i
      if "Buffers" in line and "Bottlenecks" in line and stalls_performance_bound_row==0:
          stalls_performance_bound_row=i
    
    with open(CONFIGS_DIR+'/config'+str(config_nr)+'.json') as f:
        config = json.load(f)

    for i in range(len(comp_nodes)):
        # +3 because the format of Statistics is:
        # statistics_info_row    -> //                     Statistics                                    
        # statistics_info_row +1 -> //===--------------------------------------------------------------===//
        # statistics_info_row +2 -> RESOURCE    N_OPS_ISSUED    SPAN        ISSUE-SPAN    STALL-SPAN        MAX_OCCUPANCY
        n_ops.append(int(data[statistics_info_row+3+i][1]))
        total_pan.append(int(data[statistics_info_row+3+i][2]))
        issue_span.append(int(data[statistics_info_row+3+i][3]))
        total_pan_with_stalls.append(int(data[statistics_info_row+3+i][4])) 
         
    for i in range(len(buffer_nodes)):
        stalls_span.append(int(data[stalls_span_row+3+i][1]))
        stalls_performance_bound.append(float(data[stalls_performance_bound_row+3+i][1]))

    for i in range(len(comp_nodes)):
         overlaps_without_stalls.append(list())
         overlaps_with_stalls.append(list())
         for j in range(i+1,len(comp_nodes)):
             overlaps_without_stalls[i].append(float(data[res_res_without_stalls_overlap_info_row+j+3][i+1]))
             overlaps_with_stalls[i].append(float(data[res_res_with_stalls_overlap_info_row+j+3][i+1]))
   
    for i in range(len(comp_nodes)):
        resource_issue_stall_span.append(list())
        for j in range(len(buffer_nodes)): 
            resource_issue_stall_span[i].append(float(data[res_issue_stall_span_info_row+i+3][j+1]))

    for i in range(len(comp_nodes)):

        if i < comp_nodes['register']:
            if config['vector-code']=='1':
                vector_width = float(config['max-vector-width'])              
                throughputs.append(float(get_config_value(config,hw_parameters['throughput'], i))*vector_width)
            else:
                throughputs.append(get_config_value(config,hw_parameters['throughput'], i))
            parallel_issue.append(get_config_value(config,hw_parameters['parallel-issue'], i))
        else:
            if config['vector-code']=='1' and (i == comp_nodes['l1_load'] or i == comp_nodes['l1_store']):
                throughputs.append(float(get_config_value(config,hw_parameters['throughput'], i))*2)
            else:
                throughputs.append(get_config_value(config,hw_parameters['throughput'], i))
            parallel_issue.append(get_config_value(config,hw_parameters['parallel-issue'], i))
        
    for i in range(len(comp_nodes)):
        with open(OUTPUT_DIR+"/"+str(invert_comp_nodes[i])+"_throughput_"+serie, 'w') as file:
            if parallel_issue[i]=='-1':
                file.write(throughputs[i])
                total_throughputs.append(float(throughputs[i]))
            else:
                file.write(str(float(throughputs[i])*float(parallel_issue[i])))
                total_throughputs.append(float(throughputs[i])*float(parallel_issue[i]))
        
    for i in range(len(buffer_nodes)):
        with open(OUTPUT_DIR+"/"+str(invert_buffer_nodes[i+buffer_nodes['rs']])+"_throughput_"+serie, 'w') as file:
            file.write(str(stalls_performance_bound[i]))
            
 
    for i in range(nodes['fadd'], nodes['fmov']+1):
        with open(OUTPUT_DIR+"/"+str(invert_comp_nodes[i]+"_ops_"+serie), 'w') as file:
            flops = float(n_ops[i])
            file.write(str(flops))
    
    total_flops = get_total_flops_outputfile(outputfile)

    
    with open(OUTPUT_DIR+"/flops_"+serie, 'w') as file:
        file.write(str(total_flops))

    memory_word_size = int(get_config_value(config,hw_parameters['memory-word-size']))
    mem_access_granularities=list()
    for i in range(nodes['register'], nodes['mem']+1):
        mem_access_granularities.append(get_config_value(config,hw_parameters['mem-access-granularity'], i-nodes['register']))

    total_bytes_transferred = 0
    for i in range(nodes['register']+1, nodes['mem']+1):
        with open(OUTPUT_DIR+"/"+str(invert_comp_nodes[i]+"_ops_"+serie), 'w') as file:
            n_ops[i] = float(n_ops[i])*float(mem_access_granularities[i-nodes['register']])
            total_bytes_transferred += float(n_ops[i])
            file.write(str(n_ops[i]))
    
    for i in range(nodes['register'], nodes['register']+1):
        with open(OUTPUT_DIR+"/"+str(invert_comp_nodes[i]+"_ops_"+serie), 'w') as file:
            n_ops[i] = float(n_ops[i])*float(mem_access_granularities[i-nodes['register']])
            file.write(str(n_ops[i]))
        
    with open(OUTPUT_DIR+"/bytes_transferred_"+serie, 'w') as file:
        file.write(str(total_bytes_transferred))
        
    
    # Performances files
    # 0: Tissue
    # 1: Total Span
    # 7:Total span with stalls
    # 2-6: with the five buffers
    for i in range(len(comp_nodes)):
        with open(OUTPUT_DIR+"/performance_"+invert_comp_nodes[i]+"_0_"+serie, 'w') as file:
            if issue_span[i]==0:
                file.write("0")
            else:
                file.write(str(float(n_ops[i])/float(issue_span[i])))
        with open(OUTPUT_DIR+"/performance_"+invert_comp_nodes[i]+"_1_"+serie, 'w') as file:
            if total_pan[i]==0:
                file.write("0")
            else:
                file.write(str(float(n_ops[i])/float(total_pan[i])))
        with open(OUTPUT_DIR+"/performance_"+invert_comp_nodes[i]+"_7_"+serie, 'w') as file:
            if total_pan_with_stalls[i]==0:
                file.write("0")
            else:
                file.write(str(float(n_ops[i])/float(total_pan_with_stalls[i])))
                
    for i in range(len(comp_nodes)):
        for j in range(i+1,len(comp_nodes)):
             with open(OUTPUT_DIR+"/"+invert_comp_nodes[i]+"_"+invert_comp_nodes[j]+"_overlap_without_stalls_"+serie, 'w') as file:
                 file.write(str(overlaps_without_stalls[i][j-i-1]))

    for i in range(len(resource_issue_stall_span)):
        for j in range(len(resource_issue_stall_span[i])):
            with open(OUTPUT_DIR+"/performance_"+invert_comp_nodes[i]+"_"+str(j+2)+"_"+serie, 'w') as file:
                if resource_issue_stall_span[i][j]==0:
                    file.write("0")
                else:
                    file.write(str(float(n_ops[i])/float(resource_issue_stall_span[i][j])))

    for i in range(len(comp_nodes)):
        for j in range(i+1,len(comp_nodes)):
             with open(OUTPUT_DIR+"/"+invert_comp_nodes[i]+"_"+invert_comp_nodes[j]+"_overlap_with_stalls_"+serie, 'w') as file:
                 file.write(str(overlaps_with_stalls[i][j-i-1]))

    # Once the data is read, write it in the format the roofline plot script requires
    with open(OUTPUT_DIR+"/tsc_"+serie, 'w') as file:
        file.write(data[total_line+6][2])
    
    
# Add horizontal bound associated with the platform's peak performance 'peak_perf'
def add_perf_line(ax, peak_perf, label,  col='black', width=0.75, s='-'):
    if peak_perf!= 0:
        ax.axhline(y=peak_perf, linewidth=width, color=col, linestyle=s)
        y_coordinate_transformed = (log(peak_perf)-log(Y_MIN))/(log(Y_MAX/Y_MIN))
        ax.text(0.76,y_coordinate_transformed+0.01, label+" ("+str(peak_perf)+" F/C)", fontsize=8, transform=ax.transAxes)


# Add diagonal bound associated with the platform's bandwidth 'bw'
def add_BW_line(ax,bw, label):
    x = np.linspace(X_MIN, X_MAX, 100)
    y = x*bw
    ax.plot(x, y, linewidth=0.75, color='black')
    y_coordinate_transformed = (log(X_MIN*bw)-log(Y_MIN))/(log(Y_MAX/Y_MIN))+0.16 #0.16 is the offset of the lower axis
    ax.text(0.01,y_coordinate_transformed+0.05+0.0075*(len(str(bw))-1), label+' ('+str(bw)+' B/C)',fontsize=8, rotation=45, transform=ax.transAxes)

def add_modified_perf_line(ax, peak_perf, frac, label,  col='black', width=0.75, s='-'):
    if peak_perf!= 0:
        ax.axhline(y=peak_perf/frac, linewidth=width, color=col, linestyle=s)
        y_coordinate_transformed = (log(peak_perf/frac)-log(Y_MIN))/(log(Y_MAX/Y_MIN))
        ax.text(0.76,y_coordinate_transformed+0.01, label+" ("+str(peak_perf)+" F/C)/"+str(frac), fontsize=8, transform=ax.transAxes)
        


def add_modified_BW_line(ax,bw,frac, label, c, evalPoint = 0, width=0.75, s='-'):
    if bw != 0:
        if frac !=0 :    
            x = np.linspace(X_MIN, X_MAX, 100)
            y = x*bw/frac
            ax.plot(x, y, linewidth=width, color=c, linestyle=s)
            y_coordinate_transformed = (log(X_MIN*bw/frac)-log(Y_MIN))/(log(Y_MAX/Y_MIN)) +0.16#0.16 is the offset of the lower axis
            ax.text(0.01,y_coordinate_transformed+1, label+' ('+str(bw)+' B/C)/'+str(frac),fontsize=8, rotation=45,transform=ax.transAxes)


def add_overlap_line(ax,bw,pi, alpha,frac1=1, frac2=1, c=dark_grey_color, width=0.5, s='-'):
       if frac1!=0 and frac2!= 0 and bw != 0 and pi != 0: 
        x = np.linspace(X_MIN, X_MAX, 100000).tolist()
        y = []
        ridge_point = pi*frac2/(bw*frac1)
        l = []
        bisect.insort_left(x, ridge_point)
        for i in x:    
            y.append(i/((i*frac1)/pi+frac2/bw-alpha*min(frac2/bw,(i*frac1)/pi)))             
        ax.plot(x, y, linewidth=width, color=c, linestyle =s)
        y_coordinate_transformed = (log(X_MIN*bw/frac2)-log(Y_MIN))/(log(Y_MAX/Y_MIN))+0.16 #0.16 is the offset of the lower axis

#The first parameters is the small one
def add_overlap_mem_line(ax,bw1,bw2, frac1, frac2, alpha):
    x = np.linspace(X_MIN, X_MAX, 100)
    y = []
    for i in np.nditer(x.T):
        y.append(bw1*bw2*i/(bw2*(1-alpha)*frac1+bw1*frac2))
    ax.plot(x, y, linewidth=0.75, color='black')
    y_coordinate_transformed = (log(X_MIN*bw1)-log(Y_MIN))/(log(Y_MAX/Y_MIN))+0.16 #0.16 is the offset of the lower axis



def read_input_file(file_name):
    read_data = []
    file_in = open(OUTPUT_DIR+'/'+file_name,'r')
    lines = file_in.readlines()
    for line in lines:
        split_line = line.rstrip('\n').split(' ')
        read_data.append(split_line)
    #When there are several repetitions, we don't want this
    merged_read_data = list(itertools.chain.from_iterable(read_data))
    file_in.close()
    return merged_read_data





def generate_roofline_plot(config_nr = 'SB'):
  
    separate_buffers = False

    with open(CONFIGS_DIR+'/config'+str(config_nr)+'.json') as f:
        config = json.load(f)

    serie = '%s/erm.out' % (OUTPUT_DIR)

    write_data_roofline_plot(config_nr, serie)
    invert_comp_nodes =dict((v,k) for k,v in comp_nodes.items())
    invert_buffer_nodes =dict((v,k) for k,v in buffer_nodes.items())
    serie=os.path.basename(os.path.normpath(serie))

    # Create local variables for vector containing whether to plot lines
    add_theoretical_th_bottleneck =  [False]*len(comp_nodes)
    add_modified_th_bottleneck =[True]*len(comp_nodes)
    add_issue_bottleneck = [True]*len(comp_nodes)
    add_latency_bottleneck =[True]*len(comp_nodes)
    add_all_node_bottleneck =[True]*len(comp_nodes)
    add_overlap_bottleneck =[False]*len(comp_nodes)

    # Read all the data that has been written in temporary files by write_data_roofline_plot
    theoretical_throughputs = list()
    n_ops = list()
    performances = list()
    buffer_performances = list()

    for i in range(len(comp_nodes)):
         theoretical_throughputs.append(read_input_file(invert_comp_nodes[i]+'_throughput_'+serie))
         n_ops.append(read_input_file(invert_comp_nodes[i]+'_ops_'+serie))
         performances.append(list())
         for j in range(8):
             performances[i].append(read_input_file('performance_'+invert_comp_nodes[i]+'_'+str(j)+'_'+serie))
             
    for i in range(len(buffer_nodes)):
        buffer_performances.append(read_input_file(invert_buffer_nodes[i+buffer_nodes['rs']]+'_throughput_'+serie))

    n_cycles= [read_input_file('tsc_'+serie)]
    total_bytes_transferred=[read_input_file('bytes_transferred_'+serie)] 
    total_flops=[read_input_file('flops_'+serie)]                        

    overlaps_without_stalls=list()
    overlaps_with_stalls = list()
    for i in range(len(comp_nodes)):
        overlaps_without_stalls.append(list())
        overlaps_with_stalls.append(list())
        for j in range(i+1, len(comp_nodes)):
            overlaps_without_stalls[i].append(read_input_file(invert_comp_nodes[i]+"_"+invert_comp_nodes[j]+"_overlap_without_stalls_"+serie))
            overlaps_with_stalls[i].append(read_input_file(invert_comp_nodes[i]+"_"+invert_comp_nodes[j]+"_overlap_with_stalls_"+serie))
    
    # Generate the plot
    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.set_yscale('log') # Log scale - Roofline is always log-log plot
    ax.set_xscale('log')
    ax.axis([X_MIN,X_MAX,Y_MIN,Y_MAX])
    ax.set_aspect('equal')

     # Manually adjust xtick/ytick labels when log scale
    minloc =int(log10(X_MIN))
    maxloc =int(log10(X_MAX) +1)
    newlocs = []
    newlabels = []

    for k in range(minloc,maxloc):
        newlocs.append(10**k)
           # Do not plot the first label, it is ugly in the corner
        if 10**k <= 100:
            newlabels.append(str(10**k))
        else:
            newlabels.append(r'$10^ %d$' %k)

    plt.xticks(newlocs, newlabels)

    minloc =int(log10(Y_MIN))
    maxloc =int(log10(Y_MAX) +1)
    
    newlocs = []
    newlabels = []

    for k in range(minloc,maxloc):
        newlocs.append(10**k)
        if 10**k <= 100:
            newlabels.append(str(10**k))
        else:
            newlabels.append(r'$10^ %d$' %k)
    
    plt.yticks(newlocs, newlabels)

    for l in range(2,10):
        ax.axhline(y=l, linewidth=0.4, color='white')
    for l in np.arange(0.1,1,0.1):
      ax.axhline(y=l, linewidth=0.4, color='white')
    for l in np.arange(0.002,0.01,0.001):
        ax.axvline(x=l, linewidth=0.4, color='white')
    for l in np.arange(0.02,0.1,0.01):
      ax.axvline(x=l, linewidth=0.4, color='white')
    for l in np.arange(0.1,1,0.1):
        ax.axvline(x=l, linewidth=0.4, color='white')
    for l in np.arange(2,10,1):
      ax.axvline(x=l, linewidth=0.4, color='white')

    # Applications' performance
    xData=[]
    yData=[]
    
    W=float(total_flops[0][0])
    T =float(n_cycles[0][0])
    Q = float(total_bytes_transferred[0][0])
  
    yData.append(W/T)    
    if Q!=0:
        xData.append(W/Q)
    else:
        xData.append(W/X_MAX)   
        
    ax.scatter(xData,yData,s=50, color='black' ) # label=serie    

    # Horizontal bounds
    for i in range(nodes['fmov']):
        if n_ops[i][0]!="0.0":
            if add_theoretical_th_bottleneck[i]:
                add_modified_perf_line(ax,float(theoretical_throughputs[i][0]),1.0,invert_comp_nodes[i],'black', 2)
            if add_modified_th_bottleneck[i]:
                add_modified_perf_line(ax,float(theoretical_throughputs[i][0]),float(n_ops[i][0])/W,invert_comp_nodes[i],colors[i], 1.5)
            if add_issue_bottleneck[i]:
                add_modified_perf_line(ax,float(performances[i][0][0]),float(n_ops[i][0])/W,format_string("%s_issue",invert_comp_nodes[i]), colors[i], 0.5,'--')
            if add_latency_bottleneck[i]:
                add_modified_perf_line(ax,float(performances[i][1][0]),float(n_ops[i][0])/W,format_string("%s_latency",invert_comp_nodes[i]), colors[i], 0.5,'--')
            if add_all_node_bottleneck[i]:
                add_modified_perf_line(ax,float(performances[i][7][0]),float(n_ops[i][0])/W,format_string("%s_all",invert_comp_nodes[i]), colors[i], 0.5,'--')
            if separate_buffers==False:
                for j in range(2,7):
                    add_modified_perf_line(ax,float(performances[i][j][0]),float(n_ops[i][0])/W,format_string('%s %s', (invert_comp_nodes[i],invert_buffer_nodes[j-2+buffer_nodes['rs']] )),'black', 0.5,'--')
            if add_overlap_bottleneck[i] == True:
                for j in range(nodes['l1_load'], nodes['mem']+1):
                    overlap = overlaps_without_stalls[i][j-(i+1)][0]
                    if n_ops[j][0] != "0.0":
                        add_overlap_line(ax,float(performances[j][7][0]), float(performances[i][7][0]), float(overlap),float(n_ops[i][0])/W, float(n_ops[j][0])/Q)

    if separate_buffers==True:            
        for j in range(len(buffer_nodes)):
            if buffer_performances[j][0] != '-1.0':
                add_modified_perf_line(ax,float(buffer_performances[j][0]),1,format_string(invert_buffer_nodes[j+buffer_nodes['rs']]),'black', 0.5,'--')

    # Diagonal bounds
    for i in range(nodes['l1_load'], nodes['mem']+1):
    
        if n_ops[i][0]!="0.0":
            if add_theoretical_th_bottleneck[i]:
                add_modified_BW_line(ax,float(theoretical_throughputs[i][0]),1.0,format_string(invert_comp_nodes[i]),'black', xData, 2.0)
            if add_modified_th_bottleneck[i]:
                add_modified_BW_line(ax,float(theoretical_throughputs[i][0]),float(n_ops[i][0])/Q,format_string('%s',invert_comp_nodes[i]), colors[i],xData, 1.5)
            if add_issue_bottleneck[i]:
                add_modified_BW_line(ax,float(performances[i][0][0]),float(n_ops[i][0])/Q,format_string("%s_issue", invert_comp_nodes[i]), colors[i],xData,0.5,'--')
            if add_latency_bottleneck[i]:
                add_modified_BW_line(ax,float(performances[i][1][0]),float(n_ops[i][0])/Q,format_string("%s_latency",invert_comp_nodes[i]), colors[i],xData,0.5,'--')
            if add_all_node_bottleneck[i]:
                add_modified_BW_line(ax,float(performances[i][7][0]),float(n_ops[i][0])/Q,format_string("%s_all",invert_comp_nodes[i]), colors[i], xData, 0.5,'--')
            if separate_buffers==False:
                for j in range(2,7):
                    add_modified_BW_line(ax,float(performances[i][j][0]),float(n_ops[i][0])/Q,format_string('%s %s', (invert_comp_nodes[i],invert_buffer_nodes[j-2+buffer_nodes['rs']])), 'black',xData,0.5,'--')
            
    title = benchmark+"\n P = "+str(W/T) +" f/c\n I= "+str(W/Q)
    ax.set_title(title,fontsize=12,fontweight='bold')
    ax.set_xlabel(X_LABEL, fontsize=12)
    ax.set_ylabel(Y_LABEL, fontsize=12)
           
    print ("Exporting file with name "+benchmark+"-extended-roofline-plot.pdf")
    fig.savefig(OUTPUT_DIR+'/'+benchmark+'-extended-roofline-plot.pdf',dpi=250,  bbox_inches='tight')



if __name__ == '__main__':

    #Compile source file in src_dir directory
    cmd = 'clang -emit-llvm -O3 -c %s/%s.c -o %s/%s.bc' % (SRC_DIR, benchmark, BIN_DIR, benchmark)
    p = subprocess.Popen(cmd, shell=True, universal_newlines=True)
    p.wait() 
    
    # Run the bitcode file located in BIN_DIR and store the output in OUTPUT_DIR
    cmd = '%s/lli -force-interpreter -function %s -warm-cache -uarch SB %s/%s.bc %s 2> %s/erm.out' % (LLI_PATH, function, BIN_DIR, benchmark, input, OUTPUT_DIR)
    print (cmd)
    p = subprocess.Popen(cmd, shell=True, universal_newlines=True)
    p.wait() 
    
    # Generate roofline plot
    generate_roofline_plot()
    
    

    
