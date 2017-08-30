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

Clone the repository:

https://github.com/caparrov/ERM.git

ERM is based on LLVM 4.0.1 (last version available at the time of updating this repository).
It contains the entire LLVM directory (llvm.4.0.1.src), with the additional files in llvm.4.0.1.src/lib/Support
that implement the DAG analysis, and some modifications in the interpreter
(llvm.4.0.1.src/lib/Execution/Interpreter/Execution.cpp). To install ERM, you need to install LLVM. To do so, create
an empty build directory

```
mkdir llvm.4.0.1.build
cd llvm.4.0.1.build
```

### Requirements

Note that to install LLVM, CMake 3.4.3 and GCC 4.8 are the minimum required.


Then, execute the following command, replacing /path/to/llvm/install and path/to/binutils/include with the path to the directory where LLVM is to be installed, and the path to the binutils:

```
CC=gcc CXX=g++ cmake -DCMAKE_INSTALL_PREFIX=/path/to/install -DLLVM_ENABLE_FFI=ON -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_ENABLE_CXX1Y=ON - DLLVM_BINUTILS_INCDIR=path/to/binutils/include -DCMAKE_BUILD_TYPE=RelWithDebInfo -Wno-dev ../llvm-4.0.1.src
```

Add DynamicAnalysis.cpp to llvm-4.0.1.src/lib/Support/CMakeLists.txt

Finally, make and install LLVM. It takes time, so be patient :)

```
make
make install
```

#### Running an application

Locate your source file (C or C++) into the srs directory.

* Put the function such that it is no inlined and can be detected

* If run in a warm cache scenario, make sure the function is called twice:


*

The output directory containts the outuput of the interpreter, erm.out, and the PDF of the file.

## Output




[1] V. Caparrós Cabezas. "A DAG-Based Approach to ModelingBottlenecks on Modern Microarchitectures". Diss. ETH No. 24256 (2017)

[2]




