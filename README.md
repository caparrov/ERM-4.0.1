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

ERM requires the installation of LLVM 4.0.1 (last version available at the time of updating this repository).
To install LLVM, CMake 3.4.3 and GCC 4.8 are the minimum required.

To generate the extended roofline plot, ERM uses the python plotting library Matplolib.

### Install LLVM

This repository contains the entire LLVM directory (llvm.4.0.1.src), with the additional files in llvm.4.0.1.src/lib/Support
that implement the DAG analysis, and some modifications in the interpreter
(llvm.4.0.1.src/lib/Execution/Interpreter/Execution.cpp). To install LLVM, create an empty build directory

```
mkdir llvm.4.0.1.build
cd llvm.4.0.1.build
```


Then, execute the following command, replacing path/to/llvm/install and path/to/binutils/include with the path to the directory where LLVM is to be installed, and the path to the binutils:

```
CC=gcc CXX=g++ cmake -DCMAKE_INSTALL_PREFIX=/path/to/install -DLLVM_ENABLE_FFI=ON -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_ENABLE_CXX1Y=ON - DLLVM_BINUTILS_INCDIR=path/to/binutils/include -DCMAKE_BUILD_TYPE=RelWithDebInfo -Wno-dev ../llvm-4.0.1.src
```

Add DynamicAnalysis.cpp to llvm-4.0.1.src/lib/Support/CMakeLists.txt

Finally, make and install LLVM. It takes time, so be patient :)

```
make
make install
```

## Running an application

1. Locate your source file (C or C++) into the src directory.  

* By default, ERM analyzes the entire main function, but it is recommeded to specify the specific function to be analyzed. 
To do so, make sure the function is not inlined. Otherwise, ERM cannot detect the function call to trigger the analysis.
Put the function such that it is no inlined (prepend `static __attribute__((noinline)`) to the function signature) and can be detected:  

* If run in a warm cache scenario, make sure the function is called twice:

```
for (unsigned i = 0; i< 2; i++)
	kernel();
```

2. Setup configuration variables in the script run-erm.py
 
* benchmark: name of the source code file
* function: name of the function to be analyzed. 
* input: arguments for the execution of the input file, if any
* double_precision: 1 if double-precision flaoting point, 0 is single-precision floating point.
* config: 

### Example

To analyze matrix-matrix-multiplication (file provided as an example in the src directory) of size 100 (size specified in the command line) running on a modeled Sandy Bridge microarchitecture, these are the parameters:

```
benchmark = 'mmm'
function = 'mmm'
input = '20'
double_precision = 1
config = 'SB'

```
Finally, initilize LLI_PATH and CLANG_PATH to the location where they are installed (prefix specificied in the LLVM installation).

3. Run the script run-erm.py


```
python run-erm.py
```


This script does the following:

* Compiles de source code into LLVM bitcode, and put the resulting binary into the bin directoy

* Runs the 

* Stores the outuput of the interpreter, erm.out, and the extended roofline plot (name_of_app.pdf) in the output directory.


### Output




[1] V. Caparrós Cabezas. "A DAG-Based Approach to ModelingBottlenecks on Modern Microarchitectures". Diss. ETH No. 24256 (2017)

[2] S. Williams, A. Waterman and D. Patterson. "Roofline: an insightful visual performance model for multicore architectures
". Communications of the ACM, 2009.




