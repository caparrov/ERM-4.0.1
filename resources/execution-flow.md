# Execution flow in ERM


The following figure illustrates the execution flow of ERM. 



![Alt text](images/erm-execution-flow-steps-web.gif?raw=true "Optional Title")





It consists of three steps, namely, compilation,execution of the LLVM IR instruction trace, and analysis of the scheduled DAG, as explainednext.* **Step 1: Compilation**. The first step consists of compiling the source code of the applicationinto LLVM IR. To compile an application, 

```c++
clang -emit-llvm -c -g -O3 source.cpp -o source.bc
```

If the code contains intrinsics, weneed to specify the architecture flag, .e.g., -mavx for Intel AVX intrinsics, and -mfpu=neon forARM NEON; if we want to analyze scalar code we need the additional flags -fno-vectorize-fno-slp-vectorize to prevent vectorization.

* **Step 2: Execution of the LLVM IR instruction trace**. The next step in the simulationprocess is to execute the LLVM IR (also known as bitcode) to obtain the dynamicinstruction trace and generate and schedule the computation DAG. Our approach, hence,can be considered a trace-driven simulation, in which the instruction trace is simulated ona generic microarchitectural model. We take advantage of the modular design of LLVM toimplement our analysis with two alternative approaches: Using the LLVM interpreter andinstrumenting the bitcode file. Both approaches analyze the same LLVMIR instruction trace and both approaches produce exactly the same results because theyshare the source code of the library that implements the scheduling of the DAG. They differin the LLVM tools and modules involved in the analysis, and language and ISA extensionssupported.

** Interpreter **. The LLVM infrastructure includes an interpreter, lli, that executes bitcodefiles by analyzing LLVM IR instruction by instruction. The following figure shows the structure of
the run function of the interpreter: Thecode highlighted in green is the main loop over the instructions of the LLVM IR instructiontrace (this is original LLVM code), and the code highlighted in blue is the code we insert tocall the analysis function. 




The main advantage of using the interpreter is that it requires asingle step in the execution flow after compilation. Further, since the interpreter is part ofthe LLVM infrastructure, it is tightly integrated with the entire framework. Unfortunately,it is not actively maintained and many recent ISA extension, such as the vector intrinsicslisted in Table 2.4, are not supported. Also, it has limited support for some C++ features,such as variable-length argument functions.



** Instrumentation and execution of the dynamic trace **. This approach consists ofthree steps. First, the bitcode file is instrumented by inserting calls to a runtime librarythat generates the information necessary to build the dynamic dependence graph. Second,the instrumented bitcode is compiled and executed; when executed, the inserted runtimefunctions record the data of the dynamic computation DAG in a trace (e.g., dependencesbetween instructions, address and size of memory accesses, etc.) referred to as taskgraph.To perform these two steps ERM relies on Contech [1], an LLVM-based framework forgenerating dynamic task graphs. Finally, the dynamic task graph is analyzed with anLLVM pass using the LLVM analyzer opt. As with the interpreter, the structure of theLLVM pass, shown in 2.14(b), matches the scheduling Algorithm 1: The nested for loop,highlighted in green, iterates over the basic blocks of a task, and over the instructions ofeach basic block. Although this approach requires more steps and involves more LLVMmodules than using the interpreter, it does not have the limitation of language/ISA featuresnot supported; once an application has been compiled into LLVM IR and the dynamic taskgraph is generated, it can be analyzed.The two code snippets in Fig. 2.14 demonstrate the modularity of our approach to generatingand analyzing scheduled DAGs: Our library that implements the scheduling Algorithm 2can be used with any tool that parses a dynamic instruction trace according to Algorithm 1.


* **Step 3: Analysis of the scheduled DAG**. The final step is to analyze the scheduled DAGobtained in Step 2 and report the data defined in (2.8)–(2.19). Since the two approaches presented above use the same library for analyzing the dynamic instruction trace, bothapproaches produce exactly the same output information.


[1] 
