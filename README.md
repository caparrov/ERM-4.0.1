# ERM: Extended Roofline Model

ERM is a tool for analyzing (modeled) bottlenecks of numerical kernels running on modern microarchitectures.

ERM is based on the the DAG-based performance model from [1]. Given a numerical kernel (written in C/C++), ERM generates its dynamic computation DAG (for the given input) and simulates its execution on a high-level model of a microarchicture. From the scheduled DAG, it extracts detailed per-cycle data about the execution, that is used to generate an extended roofline plot, an extension of the original roofline plot [2], with additional . The result is ageneralization of the roofline plot that integrates additional hardware-related bottlenecks as performance bounds into a singleviewgraph.




## Resources

* [DAG-Based Performance model]
* [Execution flow in ERM](https://github.com/caparrov/ERM/blob/master/resources/execution-flow.md)
* [Limitations](https://github.com/caparrov/ERM/resources/limitations.md)
* [Comparison to measured performance](https://github.com/caparrov/ERM/resources/comparison.md)
* [Examples]


## Build Instructions


### Requirements

* LLVM 4.0.1 
* CMake (version 3.8 or greater)
* GCC (version 4.3 or greater)
* Python (version 2.7 or greater)
* Matplotlib (version X or greater)

### Install LLVM

This repository contains the entire source code of LLVM (llvm.4.0.1.src), with additional files in llvm.4.0.1.src/lib/Support and llvm.4.0.1.src/include/llvm/Support that implement the DAG analysis, and some modifications in the interpreter
(llvm.4.0.1.src/lib/Execution/Interpreter/Execution.cpp). The original lib/Support/CMakeLists.txt is also modified to add the new files. To install LLVM,

1. Create an empty build directory:

```
mkdir llvm.4.0.1.build
cd llvm.4.0.1.build
```

2.  Then, execute the following command, replacing path/to/llvm/install and path/to/binutils/include with the path to the directory where LLVM is to be installed, and the path to the binutils:

```
CC=gcc CXX=g++ cmake -DCMAKE_INSTALL_PREFIX=/path/to/install -DLLVM_ENABLE_FFI=ON -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_ENABLE_CXX1Y=ON -DLLVM_BINUTILS_INCDIR=path/to/binutils/include -DCMAKE_BUILD_TYPE=RelWithDebInfo -Wno-dev ../llvm-4.0.1.src
```

3. Finally, make and install. It takes time, so be patient :)

```
make && make install
```

## Analyzing an application

ERM assumes the following directory layout:

    ERM
    ├── src                   # Source code of the numerical kernels (.cpp, .c. .h, etc.)
    ├── bin                   # LLVM bitcode files.
    ├── config                # JSON files that contain the microarchitectural configuration.
    ├── output                # ERM output and extended roofline plots.
    └── run-erm.py            # Python script to run ERM.


1. Prepare and locate your source file (C or C++) into the src directory.  

* By default, ERM analyzes the entire main function, but it is recommeded to specify the function to be analyzed. 
To make sure the function is not inlined (otherwise ERM cannot detect the function call to trigger the analysis), prepend `static __attribute__((noinline)`) to the function signature. 

* If you want to analyze the executio of the kernel in a warm cache scenario, make sure the function is called twice:

```
for (unsigned i = 0; i< 2; i++)
	kernel();
```

* If multiple files, 

2. Specifiy the microarchitectural parameters.

The parameters that define the model of the microarchitecture can be specified via command line to the interpreter (type 'lli --help' to see all command line options) or via a JSON file located in the configs directory. This option is preferred since the post-analysis to generate the extended roofline plot requires access to the microrachitectural parameters. All configurations files must have the format `configX.json`, where X is an ID for the configuration. For example, `configSB.json` contains the parameters that define a SB microarchiecture.


</sub>Command-line argument | Description</sub>
:----------- | :-----
  -address-generation-units=<uint>  |     Specify the number of address generation units. Default value is infinity
  -cache-line-size=<uint>           |     Specify the cache line size (B). Default value is 64 B
  -debug                             |    Generate debug information to allow debugging IR.
  -execution-units-latency=<number>  |    Specify the execution latency of the nodes(cycles). Default value is 1 cycle
  -execution-units-parallel-issue=<int> | Specify the number of nodes that can be executed in parallel based on ports execution. Default value is -1 cycle
  -execution-units-throughput=<number> |  Specify the execution bandwidth of the nodes(ops executed/cycles). Default value is -1 cycle
  -force-interpreter                   |  Force interpretation: disable JIT
  -function=<string>                   |  Name of the function to be analyzed
  -help                                |  Display available options (-help-hidden for more)
  -instruction-fetch-bandwidth=<int>   |  Specify the size of the reorder buffer. Default value is infinity 
  -l1-cache-size=<uint>                |  Specify the size of the L1 cache (in bytes). Default value is 32 KB
  -l2-cache-size=<uint>                |  Specify the size of the L2 cache (in bytes). Default value is 256 KB
  -line-fill-buffer-size=<uint>        |  Specify the size of the fill line buffer. Default value is infinity
  -llc-cache-size=<uint>               |  Specify the size of the L3 cache (in bytes). Default value is 20 MB  
  -load-buffer-size=<uint>             |  Specify the size of the load buffer. Default value is infinity  
  -mem-access-granularity=<uint>       |  Specify the memory access granularity for the different levels of the memory hierarchy (bytes). Default value is memory word size
  -memory-word-size=<uint>             |  Specify the size in bytes of a data item. Default value is 8 (double precision) 
  -reorder-buffer-size=<uint>           | Specify the size of the reorder buffer. Default value is infinity
  -reservation-station-size=<uint>      | Specify the size of a centralized reservation station. Default value is infinity  
  -store-buffer-size=<uint>            |  Specify the size of the store buffer. Default value is infinity



3. Configure the analysis with the following configuration variables in the script run-erm.py
 
* benchmark: name of the source code file without the extension.
* function: name of the function to be analyzed. 
* input: arguments for the execution of the input file, if any.
* double_precision: 1 if double-precision flaoting point, 0 is single-precision floating point.
* config: The variable config in the run-erm.py script must be initialized with the config ID (X).

* Initilize LLI_PATH and CLANG_PATH to the location where they are installed (prefix specificied in the LLVM installation).

### Example

To analyze matrix-matrix-multiplication (file provided as an example in the src directory) of size 100 (size specified in the command line) running on a modeled Sandy Bridge microarchitecture, these are the parameters:

```
benchmark = 'mmm'
function = 'mmm'
input = '100'
double_precision = 1
config = 'SB'

```


3. Run the script run-erm.py


```
python run-erm.py
```


This script does the following:

* Compiles de source code into LLVM bitcode, and put the resulting binary into the bin directoy

* Runs the 

* Stores the output of the interpreter (erm.out) and the extended roofline plot (name_of_app.pdf) in the output directory.


### Output




## References

[1] V. Caparrós Cabezas. "A DAG-Based Approach to ModelingBottlenecks on Modern Microarchitectures". Diss. ETH No. 24256 (2017)

[2] S. Williams, A. Waterman and D. Patterson. "Roofline: an insightful visual performance model for multicore architectures
". Communications of the ACM, 2009.




