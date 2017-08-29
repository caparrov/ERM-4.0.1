# ERM: Extended Roofline Model

Our tool to generate extended roofline plots is based on a DAG analysis
implemented in the LLVM Interpreter. The analysis is implemented in
the file DynamicAnalysis.cpp, 



## Resources

* [DAG-Based Performance model]
* [Execution flow in ERM](https://github.com/caparrov/ERM/blob/master/resources/execution-flow.md)
* [Limitations](https://github.com/caparrov/ERM/resources/limitations.md)
* [Comparison to measured performance](https://github.com/caparrov/ERM/resources/comparison.md)
* [Examples]




## Installation


You can clone the code from:

https://github.com/caparrov/ERM.git

It contains the entire LLVM directory, with the additional files in lib/Support
that implement the analysis, and some modifications in the interpreter
(lib/Execution/Interpreter/Execution.cpp), and 


To build it, create an empty directory, e.g. ERM-build, and within this
directory type:

C++11!!!

path-to-ERM/configure --enable-libffi --enable-optimized --with-binutils-include=/usr/local/binutils/include --enable-cxx11 CC=gcc CXX=g++

VERY IMPORTANT
: Requires cmake 3.8, gcc 4.8, etc. It takes time, so be patient :)
CC=/usr/local/bin/gcc GXX=/usr/local/bin/g++ cmake -DCMAKE_INSTALL_PREFIX=/usr -DLLVM_ENABLE_FFI=ON -DCMAKE_BUILD_TYPE=Release -DLLVM_BINUTILS_INCDIR=/usr/local/binutils/include -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLLVM_ENABLE_CXX1Y=ON -DLLVM_BUILD_LLVM_DYLIB=ON  -Wno-dev -DCMAKE_CXX_LINK_FLAGS="-Wl,-rpath,/usr/lib64 -L/usr/lib64" ../llvm-4.0.1.src

CC=/usr/local/bin/gcc CXX=/usr/local/bin/g++ cmake -DCMAKE_INSTALL_PREFIX=/local -DLLVM_ENABLE_FFI=ON -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_ENABLE_CXX1Y=ON -DLLVM_BINUTILS_INCDIR=/usr/local/binutils/include -DCMAKE_BUILD_TYPE=RelWithDebInfo      -Wno-dev ../llvm-4.0.1.src



Add DynamicAnalysis.cpp to /local/llvm-4.0.1.src/lib/Support/CMakeLists.txt


If you want the installation of ERM in a specific directory add the
—prefix=path option to the configure.


Then make, and make install.


## Install Contech

As explained in XX, . In order to , you need to install Contech from 

https://github.com/bprail/contech


Copy /local/contech/common/taskLib/libTask.a to /local/llvm-3.4-build/Release+Asserts/lib/libTask.a

## What's included

Within the download you'll find the following directories and files, You'll see something like this:


Mkae sure that in DyanamicAnalysis.h corresponds to the last intruction in ./include/llvm/IR/Instruction.def
#define LAST_INST 64


## Run with the LLVM interpreter



## Run with Contech


## Output




## Generate Extended Roofline Plots






