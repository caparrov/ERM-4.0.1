//=-------------------- llvm/Support/DynamicAnalysis.h ------======= -*- C++ -*//
//
//                     The LLVM Compiler Infrastructure
//
//  Victoria Caparros Cabezas <caparrov@inf.ethz.ch>
//===----------------------------------------------------------------------===//

#define INTERPRETER

#ifdef INTERPRETER
#include "llvm/Support/DynamicAnalysis.h"
#include "llvm/Support/ValuesAnalysis.h"
#else
#include "DynamicAnalysis.h"
#include "ValuesAnalysis.h"
#include "llvm/Support/CFG.h"
#endif

//===----------------------------------------------------------------------===//
//                        Constructor of the analyzer
//===----------------------------------------------------------------------===//

DynamicAnalysis::DynamicAnalysis(string TargetFunction,
                                 string Microarchitecture,
                                 unsigned MemoryWordSize,
                                 unsigned CacheLineSize,
                                 unsigned RegisterFileSize,
                                 unsigned L1CacheSize,
                                 unsigned L2CacheSize,
                                 unsigned LLCCacheSize,
                                 vector < float >ExecutionUnitsLatency,
                                 vector < double >ExecutionUnitsThroughput,
                                 vector < int >ExecutionUnitsParallelIssue,
                                 vector < unsigned >MemAccessGranularity,
                                 int AddressGenerationUnits,
                                 int InstructionFetchBandwidth,
                                 int ReservationStationSize,
                                 int ReorderBufferSize,
                                 int LoadBufferSize,
                                 int StoreBufferSize,
                                 int LineFillBufferSize,
                                 bool WarmCache,
                                 bool x86MemoryModel,
                                 bool ARMMemoryModel,
                                 bool SpatialPrefetcher,
                                 bool ConstraintPorts,
                                 bool ConstraintPortsx86,
                                 bool ConstraintPortsARM,
                                 bool ConstraintAGUs,
                                 int rep,
                                 bool InOrderExecution,
                                 bool ReportOnlyPerformance,
                                 unsigned PrefetchLevel,
                                 unsigned PrefetchDispatch,
                                 unsigned PrefetchTarget,
                                 string OutputDir,
                                 bool FloatPrecision,
                                 bool VectorCode,
                                 unsigned VectorWidth)
{
  // First, initialize local variable that define the number of execution units
  // and nodes in the high-level microarchitecture model.
  NTotalResources = MAX_RESOURCE_VALUE;
  
  NExecutionUnits = EXECUTION_UNITS;
  NArithmeticExecutionUnits = ARITHMETIC_EXECUTION_UNITS;
  NMovExecutionUnits = MOV_EXECUTION_UNITS;
  NMemExecutionUnits = MEM_EXECUTION_UNITS;
  NMiscExecutionUnits = MISC_EXECUTION_UNITS;
  
  // Number of nodes' types
  NArithmeticNodes = ARITHMETIC_NODES;
  NMovNodes = MOV_NODES;
  NMemNodes = MEM_NODES;
  NNodes = NODES;
  
  NBuffers = BUFFERS;
  NPorts = DISPATCH_PORTS;
  NAGUs = AGUS;
  NLoadAGUs = LOAD_AGUS;
  NStoreAGUs = STORE_AGUS;
  
  // Mapping between nodes and execution units. ExecutionUnit[] vector contains
  // one entry for every type of node, and the associated execution unit.
  
  for (unsigned i = 0; i < NNodes; i++)
    ExecutionUnit.push_back(0);

  ExecutionUnit[FP32_ADD_NODE] = FP32_ADDER;
  ExecutionUnit[FP64_ADD_NODE] = FP64_ADDER;
  
  ExecutionUnit[FP32_MUL_NODE] = FP32_MULTIPLIER;
  ExecutionUnit[FP64_MUL_NODE] = FP64_MULTIPLIER;
  
  ExecutionUnit[FP32_FMA_NODE] = FP32_FMADDER;
  ExecutionUnit[FP64_FMA_NODE] = FP64_FMADDER;
  
  ExecutionUnit[FP32_DIV_NODE] = FP32_DIVIDER;
  ExecutionUnit[FP64_DIV_NODE] = FP64_DIVIDER;
  
  ExecutionUnit[FP32_SHUFFLE_NODE] = FP32_SHUFFLE_UNIT;
  ExecutionUnit[FP64_SHUFFLE_NODE] = FP64_SHUFFLE_UNIT;
  
  ExecutionUnit[FP32_BLEND_NODE] = FP32_BLEND_UNIT;
  ExecutionUnit[FP64_BLEND_NODE] = FP64_BLEND_UNIT;
  
  ExecutionUnit[FP32_BOOL_NODE] = FP32_BOOL_UNIT;
  ExecutionUnit[FP64_BOOL_NODE] = FP64_BOOL_UNIT;
  
  ExecutionUnit[REGISTER_STORE_NODE] = REGISTER_STORE_CHANNEL;
  ExecutionUnit[REGISTER_LOAD_NODE] =REGISTER_LOAD_CHANNEL;
  
  ExecutionUnit[L1_STORE_NODE] = L1_STORE_CHANNEL;
  ExecutionUnit[L1_LOAD_NODE] = L1_LOAD_CHANNEL;
  
  ExecutionUnit[L2_STORE_NODE] = L2_STORE_CHANNEL;
  ExecutionUnit[L2_LOAD_NODE] = L2_LOAD_CHANNEL;
  
  ExecutionUnit[L3_STORE_NODE] = L3_STORE_CHANNEL;
  ExecutionUnit[L3_LOAD_NODE] = L3_LOAD_CHANNEL;
  
  ExecutionUnit[MEM_STORE_NODE] = MEM_STORE_CHANNEL;
  ExecutionUnit[MEM_LOAD_NODE] = MEM_LOAD_CHANNEL;
  
  ExecutionUnit[AGU_NODE] = ADDRESS_GENERATION_UNIT;
  
  ExecutionUnit[PORT_0_NODE] = PORT_0;
  ExecutionUnit[PORT_1_NODE] = PORT_1;
  ExecutionUnit[PORT_2_NODE] = PORT_2;
  ExecutionUnit[PORT_3_NODE] = PORT_3;
  ExecutionUnit[PORT_4_NODE] = PORT_4;
  ExecutionUnit[PORT_5_NODE] = PORT_5;
  
  ExecutionUnit[MISC_NODE] = MISC_UNIT;
  
  // Initialize local variables with command-line arguments
  this->TargetFunction = TargetFunction;
  if (Microarchitecture.compare("") == 0 && (ExecutionUnitsThroughput.empty() ||
                                             ExecutionUnitsLatency.empty() ||
                                             ExecutionUnitsParallelIssue.empty()))
  Microarchitecture = "INF";
  
  
  this->Microarchitecture = Microarchitecture;
  this->MemoryWordSize = MemoryWordSize ; // In number of bytes
  this->rep = rep;
  this->ReportOnlyPerformance = ReportOnlyPerformance;
  this->OutputDir = OutputDir;
  this->FloatPrecision = FloatPrecision;
  this->VectorCode = VectorCode;
  this->VectorWidth = VectorWidth;
  if(!VectorCode)
  this->VectorWidth = 1;

  // Default valies for Latency and throughput for every resource.
  // These values can be specified via command line.
  for (unsigned i = 0; i < NExecutionUnits; i++) {
    this->ExecutionUnitsLatency.push_back(1);	// Default value for latency
    this->ExecutionUnitsThroughput.push_back(INF);	// Infinite throughput
    this->ExecutionUnitsParallelIssue.push_back(INF);	// Infinite parallel issue
  
    if (i < NArithmeticExecutionUnits + NMovExecutionUnits)
      AccessGranularities.push_back(1);
    else
      AccessGranularities.push_back(this->MemoryWordSize);
    
    vector < float >BnkVec;
    for (unsigned j = 0; j < NBuffers + 2; ++j)
      BnkVec.push_back(INF);
    this->BnkMat.push_back(BnkVec);
  }
  
  // By default ShareThroughputAmongPorts is false. Set to true for SandyBridge.
  // For a generic platform, this cannot be specified in the command line,
  // would have to be specified manually in the code below.
  for (unsigned i = 0; i < NExecutionUnits + 1 + NMiscExecutionUnits+NPorts +
       NBuffers; i++)
  ShareThroughputAmongPorts.push_back(false);
  
  // ================================================================//
  //			Default parameters for Sandy Bridge uach
  // ================================================================//
  
  if (Microarchitecture.compare("SB") == 0) {
    if(FloatPrecision == 0)
      this->MemoryWordSize = 4; // Memory word size in bytes
    // else, default value which is 8
    this->CacheLineSize = 64/ this->MemoryWordSize ; // In number of memory words
    this->RegisterFileSize = 16;
    // Cache size must be given in number fo cache lines, because reuse distance
    // is calculated based on cache lines
    this->L1CacheSize = 32768 / 64;
    this->L2CacheSize = 262144 / 64;
    this->LLCCacheSize = 20971520 / 64;
    this->AddressGenerationUnits = 2;
    this->ReservationStationSize = 54;
    this->InstructionFetchBandwidth = 4;
    this->ReorderBufferSize = 168;
    this->LoadBufferSize = 64;
    this->StoreBufferSize = 36;
    this->LineFillBufferSize = 10;
    this->WarmCache = WarmCache;
    this->x86MemoryModel = true;
    this->ARMMemoryModel = false;
    this->SpatialPrefetcher = false;
    this->ConstraintPorts = true;
    this->ConstraintPortsx86 = true;
    this->ConstraintPortsARM = false;
    this->ConstraintAGUs = true;
    this->InOrderExecution = false;
    
    // Latencies, throughputs, etc, are specified in the following order
    // (from DynamicAnalysis.h)
    // {FP32_ADDER, FP64_ADDER, FP32_MULTIPLIER, FP64_MULTIPLIER,
    // FP32_FMADDER, FP64_FMADDER, FP32_DIVIDER, FP64_DIVIDER,
    // FP32_SHUFFLE_UNIT,FP64_SHUFFLE_UNIT, FP32_BLEND_UNIT, FP64_BLEND_UNIT,
    // FP32_BOOL_UNIT, FP64_BOOL_UNIT,
    // REGISTER_CHANNEL,  L1_LOAD_CHANNEL,  L1_STORE_CHANNEL,
    // L2_CHANNEL, L3_CHANNEL,  MEM_CHANNEL}
    // Units for which there are no nodes, have values set to zero.
    if(VectorCode){
      ShareThroughputAmongPorts[L1_LOAD_CHANNEL] = true;
      this->ExecutionUnitsLatency = {3, 3,/* FP32_ADDER, FP64_ADDER, */
                                     5, 5,/* FP32_MULTIPLIER, FP64_MULTIPLIER,*/
        0, 0, /* FP32_FMADDER, FP64_FMADDER, */
        45, 45, /* FP32_DIVIDER, FP64_DIVIDER,*/
        1, 1, /* FP32_SHUFFLE_UNIT,FP64_SHUFFLE_UNIT, */
        1, 1, /* FP32_BLEND_UNIT, FP64_BLEND_UNIT,*/
        1, 1, /* FP32_BOOL_UNIT, FP64_BOOL_UNIT,*/
        0, /* REGISTER_CHANNEL*/
        4, 4, /* L1_LOAD_CHANNEL, L1_STORE_CHANNEL,*/
        12, 30, 100}; /* L2_CHANNEL, L3_CHANNEL, MEM_CHANNEL*/
      // Same order as ExecutionUnitsLatency
      this->ExecutionUnitsThroughput= {4, 4,
        4, 4,
        0, 0,
        0.0909, 0.0909,
        4, 4,
        4, 4,
        4, 4,
        -1,
        16, 16,
        32, 32, 8};
    }else{
      this->ExecutionUnitsLatency= {3, 3, /* FP32_ADDER, FP64_ADDER, */
        5, 5, /* FP32_MULTIPLIER, FP64_MULTIPLIER,*/
        0, 0, /* FP32_FMADDER, FP64_FMADDER, */
        22, 22, /* FP32_DIVIDER, FP64_DIVIDER,*/
        1, 1, /* FP32_SHUFFLE_UNIT,FP64_SHUFFLE_UNIT,*/
        1, 1, /* FP32_BLEND_UNIT, FP64_BLEND_UNIT,*/
        1, 1, /* FP32_BOOL_UNIT, FP64_BOOL_UNIT,*/
        0, /* REGISTER_CHANNEL*/
        4, 4,  /* L1_LOAD_CHANNEL, L1_STORE_CHANNEL,*/
        12, 30, 100}; /* L2_CHANNEL, L3_CHANNEL, MEM_CHANNEL*/
      // Same order as ExecutionUnitsLatency
      this->ExecutionUnitsThroughput= {1, 1,
        1, 1,
        0, 0,
        0.04545, 0.04545,
        4, 4,
        4, 4,
        4, 4,
        -1,
        8, 8,
        32, 32,8};
    }
    this->ExecutionUnitsParallelIssue = {1, 1,
      1, 1,
      0, 0,
      1, 1,
      1, 1,
      2, 2,
      1, 1,
      -1,
      2, 1,
      1, 1, 1};
    
    AccessGranularities[REGISTER_LOAD_CHANNEL] = 8;
    AccessGranularities[L1_LOAD_CHANNEL] = 8;
    AccessGranularities[L1_STORE_CHANNEL] = 8;
    AccessGranularities[L2_LOAD_CHANNEL] = 64;
    AccessGranularities[L3_LOAD_CHANNEL] = 64;
    AccessGranularities[MEM_LOAD_CHANNEL] = 64;
    
  }else{
    // ================================================================//
    //				Haswell uarch, not implemented
    // ================================================================//
    if(Microarchitecture.compare("HW") == 0){
      report_fatal_error("Microarchitecture Haswell not implemented");
    }else{
      // ================================================================//
      //					ARM uach
      // ================================================================//
      if(Microarchitecture.compare("ARM-CORTEX-A9") == 0){
        NAGUs = 1;
        NPorts = 1;
        NTotalResources = 27;
        this->CacheLineSize = 32/ this->MemoryWordSize ;// In # of memory words
        this->RegisterFileSize = 32;
        this->L1CacheSize = 32768 / 32;
        this->L2CacheSize = 1048576 / 32;
        this->LLCCacheSize = 0;
        this->AddressGenerationUnits = -1;
        this->ReservationStationSize = 32;
        this->InstructionFetchBandwidth = 2;
        this->ReorderBufferSize = 40;
        this->LoadBufferSize = 0;
        this->StoreBufferSize = 4;
        this->LineFillBufferSize = 2;
        this->WarmCache = WarmCache;
        this->x86MemoryModel = 0;
        this->ARMMemoryModel = 1;
        this->SpatialPrefetcher = 0;
        this->ConstraintPorts = 1;
        this->ConstraintPortsx86 = 0;
        this->ConstraintPortsARM = 1;
        this->ConstraintAGUs = true;
        this->InOrderExecution = true;
        // ExecutionUnitsLatency, ExecutionUnitsThroughput and ExecutionUnitsParallelIssue
        // specified in the same order as explained for Sandy Bridge uarch (see above)
        this->ExecutionUnitsLatency = {4, 4,
          5, 6,
          0, 0 ,
          15, 25,
          1, 1,
          1, 1,
          1, 1,
          0,
          4, 4,
          37, 0, 100};
        this->ExecutionUnitsThroughput = {1, 1,
          1, 0.5,
          0, 0,
          0.1, 0.05,
          1, 1,
          1, 1,
          1, 1,
          -1,
          8, 8,
          4, -1, 8};
        this->ExecutionUnitsParallelIssue = {1, 1,
          1, 1,
          0, 0,
          1, 1,
          1, 1,
          1, 1,
          1, 1,
          -1,
          1, 1,
          1, 1,1};
        AccessGranularities[REGISTER_LOAD_CHANNEL] = 8;
        AccessGranularities[L1_LOAD_CHANNEL] = 8;
        AccessGranularities[L1_STORE_CHANNEL] = 8;
        AccessGranularities[L2_LOAD_CHANNEL] = 32;
        AccessGranularities[L3_LOAD_CHANNEL] = -1;
        AccessGranularities[MEM_LOAD_CHANNEL] = 32;
      }else{
        // ================================================================//
        //					Infinite uarch
        // ================================================================//
        if(Microarchitecture.compare("INF") == 0){
          this->RegisterFileSize = RegisterFileSize;
          // CacheLineSize in number of memory word sizes
          this->CacheLineSize = CacheLineSize/ this->MemoryWordSize ;
          this->L1CacheSize = 0;
          this->L2CacheSize = 0;
          this->LLCCacheSize = 0;
          this->AddressGenerationUnits = INF;
          this->ReservationStationSize = ReservationStationSize;
          this->InstructionFetchBandwidth = InstructionFetchBandwidth;
          this->ReorderBufferSize = ReorderBufferSize;
          this->LoadBufferSize = LoadBufferSize;
          this->StoreBufferSize = StoreBufferSize;
          this->LineFillBufferSize = LineFillBufferSize;
          this->WarmCache = WarmCache;
          this->x86MemoryModel = x86MemoryModel;
          this->ARMMemoryModel = ARMMemoryModel;
          this->SpatialPrefetcher = SpatialPrefetcher;
          this->ConstraintPorts = ConstraintPorts;
          this->ConstraintPortsx86 = ConstraintPortsx86;
          this->ConstraintAGUs = ConstraintAGUs;
          this->InOrderExecution = InOrderExecution;
        }else{
          // ================================================================//
          //		       uarch specified in the command line
          // ================================================================//
          this->RegisterFileSize = RegisterFileSize;
          // CacheLineSize in number of memory word sizes
          this->CacheLineSize = CacheLineSize/ this->MemoryWordSize ;
          this->L1CacheSize = L1CacheSize / CacheLineSize;
          this->L2CacheSize = L2CacheSize / CacheLineSize;
          this->LLCCacheSize = LLCCacheSize / CacheLineSize;
          this->AddressGenerationUnits = AddressGenerationUnits;
          this->ReservationStationSize = ReservationStationSize;
          this->InstructionFetchBandwidth = InstructionFetchBandwidth;
          this->ReorderBufferSize = ReorderBufferSize;
          this->LoadBufferSize = LoadBufferSize;
          this->StoreBufferSize = StoreBufferSize;
          this->LineFillBufferSize = LineFillBufferSize;
          this->WarmCache = WarmCache;
          this->x86MemoryModel = x86MemoryModel;
          this->ARMMemoryModel = ARMMemoryModel;
          this->SpatialPrefetcher = SpatialPrefetcher;
          this->ConstraintPorts = ConstraintPorts;
          this->ConstraintPortsx86 = ConstraintPortsx86;
          this->ConstraintPortsARM = ConstraintPortsARM;
          // If ConstraintPortsx86 is true, ConstraintPorts must be also true.
          if(ConstraintPortsx86 || ConstraintPortsARM)
            this->ConstraintPorts = true;
          this->ConstraintAGUs = ConstraintAGUs;
          this->InOrderExecution = InOrderExecution;
          
          if (!ExecutionUnitsThroughput.empty()){
            for (unsigned i = 0; i < NExecutionUnits; i++){
              if (i < NArithmeticExecutionUnits + NMovExecutionUnits &&
                  VectorCode){
                this->ExecutionUnitsThroughput[i] =
                VectorWidth*ExecutionUnitsThroughput[i];
              }else
                this->ExecutionUnitsThroughput[i] = ExecutionUnitsThroughput[i];
            }
          }else
            report_fatal_error("If Microarchitecture is not INF, the execution \
                              units throughput must be specified in the \
                              command line");
          
          if (!ExecutionUnitsParallelIssue.empty()){
            for (unsigned i = 0; i < NExecutionUnits; i++)
              this->ExecutionUnitsParallelIssue[i] = ExecutionUnitsParallelIssue[i];
          }else
            report_fatal_error("If Microarchitecture is not INF, the execution\
                              units parallel issue must be specified in the\
                              command line");
          
          if (!ExecutionUnitsLatency.empty()){
            for (unsigned i = 0; i < NExecutionUnits; i++)
              this->ExecutionUnitsLatency[i] = ExecutionUnitsLatency[i];
          }else
            report_fatal_error("If Microarchitecture is not INF, the execution \
                              units latency must be specified in the \
                              command line");
          
          
          if (!MemAccessGranularity.empty()){
            for (unsigned i = 0; i < NMemExecutionUnits; i++)
              AccessGranularities[i + NArithmeticExecutionUnits + NMovExecutionUnits]
              = MemAccessGranularity[i];
          }else
            report_fatal_error("If Microarchitecture is not INF, the memory\
                              access granularity must be specified in the \
                              command line");
        }
      }
    }
  }
  
  // ================================================================//
  //		Some general checks about the uarch parameters
  //    e.g., Cache line size must be a multiple of memory word size
  // ================================================================//
  
  if (!this->ExecutionUnitsLatency.empty() &&
      this->ExecutionUnitsLatency.size() != NExecutionUnits) {
    report_fatal_error("The number of latencies (" +
                       Twine(this->ExecutionUnitsLatency.size())+") does not \
                       match the number of execution units (" +
                       Twine(NExecutionUnits) +")");
  }
  
  if (!this->ExecutionUnitsThroughput.empty() &&
      this->ExecutionUnitsThroughput.size() != NExecutionUnits)
    report_fatal_error("The number of throughputs does not match the number\
                      of execution units");
  
  if (!this->ExecutionUnitsParallelIssue.empty()  &&
      this->ExecutionUnitsParallelIssue.size() != NExecutionUnits)
    report_fatal_error("The number of execution units parallel issue (" +
                       Twine(this->ExecutionUnitsParallelIssue.size()) +")does not\
                      match the number of execution units (" +
                       Twine(NExecutionUnits) + ")");
  
  if (L1CacheSize != 0 && L1CacheSize < CacheLineSize){
    report_fatal_error("L1 cache size ("+Twine(L1CacheSize) +") < cache line size\
                       (" + Twine(CacheLineSize) + ")");
  }
  
  if (L2CacheSize != 0 && L2CacheSize < CacheLineSize)
    report_fatal_error("L2 cache size < cache line size");
  
  if (LLCCacheSize != 0 && LLCCacheSize < CacheLineSize)
    report_fatal_error("LLC cache size < cache line size");
  
  if (this->CacheLineSize!= 1 && CacheLineSize % this->MemoryWordSize != 0){
    report_fatal_error("Cache line size ("+ Twine(CacheLineSize) + ") is not a \
                       multiple of memory word size ("+
                       Twine(this->MemoryWordSize)+")");
  }
  
  if (!MemAccessGranularity.empty()  &&
      MemAccessGranularity.size() != NMemExecutionUnits)
    report_fatal_error("Mem access granularities do not match the number of\
                      memory execution units");
  
  if (!this->ExecutionUnitsLatency.empty()) {
    for (unsigned i = 0; i < NExecutionUnits; i++){
      this->ExecutionUnitsLatency[i] =ceil(this->ExecutionUnitsLatency[i]);
      MaxLatencyResources = max(MaxLatencyResources,this->ExecutionUnitsLatency[i]);
    }
    RARDependences = true;
  }else{
    RARDependences = false;
    for (unsigned i = 0; i < NExecutionUnits; i++){
      MaxLatencyResources = max(MaxLatencyResources,this->ExecutionUnitsLatency[i]);
    }
  }
  
  // ================================================================//
  //		Mapping between nodes and dispatch ports
  // ================================================================//
  // IMPORTANT: Associate Dispatch ports to nodes instead of execution resources
  // because otherwise there is a problem when different nodes that share
  // execution unit but no dispatch ports
  
  vector < unsigned >emptyVector;

  for (unsigned i = 0; i < NArithmeticNodes + NMovNodes + NMemNodes; i++)
    DispatchPort.push_back(emptyVector);
  
  if (Microarchitecture.compare("SB") == 0 || ConstraintPortsx86 == true) {
    /*
     Port mapping in Sandy Bridge
     Port 0 -> FP_MUL, FP_DIV, FP_BLEND
     Port 1 -> FP_ADD
     Port 2 -> LOAD (L1, L2, L3 and MEM)
     Port 3 -> LOAD (L1, L2, L3 and MEM)
     Port 4 -> STORE_CHANNEL (L1, L2, L3 and MEM)
     Port 5 -> FP_SHUFFLE, FP_BLEND, FP_BOOL
     */
    
    emptyVector.push_back(PORT_1);
    DispatchPort[FP32_ADD_NODE] = emptyVector;
    DispatchPort[FP64_ADD_NODE] = emptyVector;
    
    emptyVector.clear();
    emptyVector.push_back(PORT_0);
    DispatchPort[FP32_MUL_NODE] = emptyVector;
    DispatchPort[FP64_MUL_NODE] = emptyVector;
    
    // Sandy Bridge does not have FMA => DispatchPort[FP64_FMA_NODE]
    //has the default empty vector
    
    emptyVector.clear();
    emptyVector.push_back(PORT_0);
    DispatchPort[FP32_DIV_NODE] = emptyVector;
    DispatchPort[FP64_DIV_NODE] = emptyVector;
    
    emptyVector.clear();
    emptyVector.push_back(PORT_5);
    DispatchPort[FP32_SHUFFLE_NODE] = emptyVector;
    DispatchPort[FP64_SHUFFLE_NODE] = emptyVector;
    
    emptyVector.clear();
    emptyVector.push_back(PORT_5);
    DispatchPort[FP32_BOOL_NODE] = emptyVector;
    DispatchPort[FP64_BOOL_NODE] = emptyVector;
    
    emptyVector.clear();
    emptyVector.push_back(PORT_0);
    emptyVector.push_back(PORT_5);
    DispatchPort[FP32_BLEND_NODE] = emptyVector;
    DispatchPort[FP64_BLEND_NODE] = emptyVector;
    
    // Registers don't have any associated dispatch port
    emptyVector.clear();
    DispatchPort[REGISTER_LOAD_NODE] = emptyVector;
    DispatchPort[REGISTER_STORE_NODE] = emptyVector;
    
    emptyVector.clear();
    emptyVector.push_back(PORT_4);
    DispatchPort[L1_STORE_NODE] = emptyVector;
    DispatchPort[L2_STORE_NODE] = emptyVector;
    DispatchPort[L3_STORE_NODE] = emptyVector;
    DispatchPort[MEM_STORE_NODE] = emptyVector;
    
    emptyVector.clear();
    emptyVector.push_back(PORT_2);
    emptyVector.push_back(PORT_3);
    DispatchPort[L1_LOAD_NODE] = emptyVector;
    DispatchPort[L2_LOAD_NODE] = emptyVector;
    DispatchPort[L3_LOAD_NODE] = emptyVector;
    DispatchPort[MEM_LOAD_NODE] = emptyVector;
    
    // ConstraintPortsx86 forces some conditions like divisions and
    // multiplications are issued in the same port. If parallel issue of a
    // given node is larger than the default ports in x86, then we add ports to
    // the corresponding pool of ports in DispathPort.
    
    // If we start with a SB uarch as a baseline and increase throughput or
    // parallel issue, the additional throughput is assigned to new ports (i.e.,
    // the constraint on ports is maintained for the original throughput, but
    // the additional one has no constraints and each additional operation is
    // mapped to a new port).
    
    if(ConstraintPortsx86){
      unsigned initialPortsSize = 0;
      unsigned bitVectorResourcesSize = 0;
      for (unsigned i = 0; i < NArithmeticNodes + NMovNodes; i++){
        if (this->ExecutionUnitsParallelIssue[ExecutionUnit[i]] != INF){
          // If there are more ops/cycle than ports associated with that op.
          if((unsigned)this->ExecutionUnitsParallelIssue[ExecutionUnit[i]] >
             DispatchPort[i].size()){
            initialPortsSize = DispatchPort[i].size();
            for (unsigned j = initialPortsSize;
                 j < (unsigned)this->ExecutionUnitsParallelIssue[ExecutionUnit[i]];
                 j++){
              NPorts++;
              NTotalResources++;
              bitVectorResourcesSize =
              FullOccupancyCyclesTree[0].tbv_map[0].BitVector.size();
              FullOccupancyCyclesTree[0].tbv_map[0].BitVector.
              resize(bitVectorResourcesSize+1,0);
              DispatchPort[i].push_back(PORT_0 + NPorts -1 );
              ShareThroughputAmongPorts.push_back(false);
            }
          }
        }else{
          // ExecutionUnitsParallelIssue cannot be INF unless there are no ops
          // of these kind, or it is a register
          if (DispatchPort[i].size() != 0){
            report_fatal_error("ExecutionUnitsParallelIssue cannot be INF\
                                if ConstraintPortsx86");
          }
        }
      }
      
      // Memory nodes - All load memory nodes (L1, ... ,mem) and all stores
      // memory nodes should have the same number of ports (except register!,
      // hence starting the for with +2)
      for (unsigned i = NArithmeticNodes + NMovNodes + 2;
           i < NArithmeticNodes + NMovNodes + NMemNodes; i++){
        if (this->ExecutionUnitsParallelIssue[ExecutionUnit[i]] != INF){
          if((unsigned)this->ExecutionUnitsParallelIssue[ExecutionUnit[i]] >
             DispatchPort[i].size()){
            initialPortsSize = DispatchPort[i].size();
            for (unsigned j = initialPortsSize;
                 j < (unsigned)this->ExecutionUnitsParallelIssue[ExecutionUnit[i]];
                 j++){
              NPorts++;
              NTotalResources++;
              bitVectorResourcesSize =
              FullOccupancyCyclesTree[0].tbv_map[0].BitVector.size();
              FullOccupancyCyclesTree[0].tbv_map[0].BitVector.
              resize(bitVectorResourcesSize+1,0);
              ShareThroughputAmongPorts.push_back(false);
              if (i == L1_LOAD_NODE || i == L2_LOAD_NODE || i == L3_LOAD_NODE
                  || i == MEM_LOAD_NODE ){
                // The extra port is associated with all load nodes, because all
                // loads should be issued by the same port. Parallel issue
                // determines how many can be done i parallel.
                DispatchPort[L1_LOAD_NODE].push_back(PORT_0+NPorts-1);
                DispatchPort[L2_LOAD_NODE].push_back(PORT_0+NPorts-1);
                DispatchPort[L3_LOAD_NODE].push_back(PORT_0+NPorts-1);
                DispatchPort[MEM_LOAD_NODE].push_back(PORT_0+NPorts-1);
              }else{
                if (i == L1_STORE_NODE || i == L2_STORE_NODE || i == L3_STORE_NODE
                    || i == MEM_STORE_NODE ){
                  DispatchPort[L1_STORE_NODE].push_back(PORT_0+NPorts-1);
                  DispatchPort[L2_STORE_NODE].push_back(PORT_0+NPorts-1);
                  DispatchPort[L3_STORE_NODE].push_back(PORT_0+NPorts-1);
                  DispatchPort[MEM_STORE_NODE].push_back(PORT_0+NPorts-1);
                }else{
                  report_fatal_error("Memory node not recognized\n");
                }
              }
            }
          }
        }else{
          if (DispatchPort[i].size() != 0){
            report_fatal_error("ExecutionUnitsParallelIssue cannot be INF\
                                if ConstraintPortsx86");
          }
        }
      }
    }
  }else{
    /*
     Port mapping in CORTEX-A9
     Port 0 -> FP32_ADD_NODE, FP64_ADD_NODE, FP32_MUL_NODE,
     FP64_MUL_NODE, FP32_FMA_NODE, FP64_FMA_NODE,
     LOAD (L1, L2, L3 and MEM),
     STORE_CHANNEL (L1, L2, L3 and MEM)
     */
    
    if (Microarchitecture.compare("ARM-CORTEX-A9") == 0 || ConstraintPortsARM == true){
      emptyVector.clear();
      emptyVector.push_back(PORT_0);
      DispatchPort[FP32_ADD_NODE] = emptyVector;
      DispatchPort[FP64_ADD_NODE] = emptyVector;
      DispatchPort[FP32_MUL_NODE] = emptyVector;
      DispatchPort[FP64_MUL_NODE] = emptyVector;
      DispatchPort[FP32_FMA_NODE] = emptyVector;
      DispatchPort[FP64_FMA_NODE] = emptyVector;
      DispatchPort[FP32_DIV_NODE] = emptyVector;
      DispatchPort[FP64_DIV_NODE] = emptyVector;
      
      
      DispatchPort[L1_LOAD_NODE] = emptyVector;
      DispatchPort[L2_LOAD_NODE] = emptyVector;
      DispatchPort[MEM_LOAD_NODE] = emptyVector;
      DispatchPort[L1_STORE_NODE] = emptyVector;
      DispatchPort[L2_STORE_NODE] = emptyVector;
      DispatchPort[MEM_STORE_NODE] = emptyVector;
      
    }else{
      if (ConstraintPorts){
        // Initial number of ports
        unsigned bitVectorResourcesSize  = 0;
        for (unsigned i = 0; i <  NArithmeticNodes + NMovNodes + NMemNodes; i++) {
          emptyVector.clear();
          if(this->ExecutionUnitsParallelIssue[ExecutionUnit[i]] != INF){
            for (unsigned j = 0;
                 j < (unsigned)this->ExecutionUnitsParallelIssue[ExecutionUnit[i]];
                 j++){
              emptyVector.push_back(PORT_0+NPorts);
              NPorts++;
              NTotalResources++;
              bitVectorResourcesSize =
              FullOccupancyCyclesTree[0].tbv_map[0].BitVector.size();
              FullOccupancyCyclesTree[0].tbv_map[0].BitVector.
              resize(bitVectorResourcesSize+1,0);
              ShareThroughputAmongPorts.push_back(false);
            }
          }else{
            if(i != REGISTER_LOAD_NODE && i != REGISTER_STORE_NODE ){
              report_fatal_error("Cannot constraint ports if parallel issue ("+
                                 Twine(this->ExecutionUnitsParallelIssue[ExecutionUnit[i]])
                                 +") is finite for execution unit "+
                                 getResourceName(ExecutionUnit[i])+"\n");
            }
          }
          DispatchPort[i] = emptyVector;
        }
      }else{
        emptyVector.clear();
        for (unsigned i = 0; i < NArithmeticNodes + NMovNodes + NMemNodes; i++){
          DispatchPort[i] = emptyVector;
        }
      }
    }
  }
  
  // Dispatch ports are associated with nodes
  for (unsigned i = 0; i < NArithmeticNodes + NMovNodes + NMemNodes; i++) {
    if (this->ConstraintPorts &&
        this->ExecutionUnitsParallelIssue[ExecutionUnit[i]] > 0 &&
        DispatchPort[i].size() != 0 &&
        DispatchPort[i].size() <
        (unsigned) this->ExecutionUnitsParallelIssue[ExecutionUnit[i]]) {
      if (i != REGISTER_LOAD_NODE && i != REGISTER_STORE_NODE){
        report_fatal_error("Execution unit "+ getResourceName(ExecutionUnit[i]) +
                           " can issue in parallel ("+
                           Twine(this->ExecutionUnitsParallelIssue[ExecutionUnit[i]]) +
                           ") more nodes of type "+ getNodeName(i) + " than\
                           ports associated with the corresponding node ("+
                           Twine(DispatchPort[i].size())+ ")");

      }
    }
    
    if ((this->ExecutionUnitsParallelIssue[ExecutionUnit[i]]==INF &&
         ConstraintPortsx86) && DispatchPort[i].size() != 0)
      report_fatal_error("Parallel Issue cannot infinite while constraining\
                         ports\n");
  }
  
  // +2 because REGISTER_CHANNEL (load and store) does not require AGUs but are
  // memory nodes anyway
  for (unsigned i = NArithmeticNodes + NMovNodes+2;
       i <= NArithmeticNodes + NMovNodes + NMemNodes; i++){
    if(this->ConstraintAGUs &&
       (this->ExecutionUnitsParallelIssue[ExecutionUnit[i]] == INF ||
        this->ExecutionUnitsThroughput[ExecutionUnit[i]] == INF)){
        dbgs() << "ExecutionUnit " <<  getResourceName(ExecutionUnit[i]) << "\n";
        dbgs() << "ExecutionUnitsThroughput " <<
         this->ExecutionUnitsThroughput[ExecutionUnit[i]] << "\n";
        dbgs() << "ExecutionUnitsParallelIssue " <<
         this->ExecutionUnitsParallelIssue[ExecutionUnit[i]] << "\n";
         dbgs() << "WARNING: Memory throughput (throughput or parallel issue)\
         should not be INF if constraintAGUs\n";
       }
  }
  
  //============================================================================
  // LATENCY AND THROUGHPUT OF BUFFERS
  //============================================================================
  // Although it has no effect, these
  // values are necessary to analyze the DAG, since buffer stalls are considered
  // nodes in the DAG.
  // IMPORTANT: The assignment below must be consistent with the units defined
  // in the enum in DynamicAnalysis.h
  for (unsigned i = 0; i < NBuffers; i++) {
    this->ExecutionUnitsLatency.push_back(1);	//Default value for latency
    this->ExecutionUnitsThroughput.push_back(1);	// Infinite throughput
    this->ExecutionUnitsParallelIssue.push_back(1);
    AccessGranularities.push_back(1);
  }
  
  //============================================================================
  // LATENCY AND THROUGHPUT OF AGUS
  //============================================================================
  if (NAGUs > 0) {
    this->ExecutionUnitsLatency.push_back(1);
    if (this->ConstraintAGUs) {
      this->ExecutionUnitsThroughput.push_back(1);
      this->ExecutionUnitsParallelIssue.push_back(NAGUs);
    }else {
      this->ExecutionUnitsThroughput.push_back(INF);
      this->ExecutionUnitsParallelIssue.push_back(INF);
    }
    AccessGranularities.push_back(1);
  }
  
  if (NLoadAGUs > 0) {
    this->ExecutionUnitsLatency.push_back(1);
    this->ExecutionUnitsThroughput.push_back(1);
    this->ExecutionUnitsParallelIssue.push_back(NLoadAGUs);
    AccessGranularities.push_back(1);
  }
  
  if (NStoreAGUs > 0) {
    this->ExecutionUnitsLatency.push_back(1);
    this->ExecutionUnitsThroughput.push_back(1);
    this->ExecutionUnitsParallelIssue.push_back(NStoreAGUs);
    AccessGranularities.push_back(1);
  }
  
  //============================================================================
  // LATENCY AND THROUGHPUT OF MISC UNITS
  //============================================================================
  for (unsigned i = 0; i < NMiscExecutionUnits; i++) {
    this->ExecutionUnitsLatency.push_back(1);
    this->ExecutionUnitsThroughput.push_back(INF);
    this->ExecutionUnitsParallelIssue.push_back(INF);
    AccessGranularities.push_back(1);
  }
  
  //============================================================================
  // LATENCY AND THROUGHPUT OF PORTS
  //============================================================================
  for (unsigned i = 0; i < NPorts; i++) {
    this->ExecutionUnitsLatency.push_back(1);	//Default value for latency
    if (this->ConstraintPorts) {
      this->ExecutionUnitsThroughput.push_back(1);
      this->ExecutionUnitsParallelIssue.push_back(1);
    }else {
      this->ExecutionUnitsThroughput.push_back(INF);	// Infinite throughput
      this->ExecutionUnitsParallelIssue.push_back(INF);
    }
    AccessGranularities.push_back(1);
  }
  
  if (this->ConstraintPorts){
    bool ShareMemoryThroughput = false;
    for (unsigned i = 0 ;
         i < NArithmeticExecutionUnits + NMovExecutionUnits + NMemExecutionUnits;
         i++){
      if(ShareThroughputAmongPorts[i] == true){
        ShareMemoryThroughput = true;
        break;
      }
    }
    if(ShareMemoryThroughput== false){
      size_t MaxParallelIssueLoads = 0;
      size_t MaxParallelIssueStores = 0;
      for (unsigned i = NArithmeticNodes + NMovNodes;
           i < NArithmeticNodes + NMovNodes + NMemNodes; i++){
        if (i == L1_LOAD_NODE || i == L2_LOAD_NODE || i == L3_LOAD_NODE ||
            i == MEM_LOAD_NODE ){
          MaxParallelIssueLoads = max(MaxParallelIssueLoads, DispatchPort[i].size());
        }else{
          if (i == L1_STORE_NODE || i == L2_STORE_NODE || i == L3_STORE_NODE ||
              i == MEM_STORE_NODE )
          MaxParallelIssueStores = max(MaxParallelIssueStores, DispatchPort[i].size());
        }
      }
    }
  }
  
  // We need AccessWidth and Throughput for every resource for which we calculate
  // span, including ports.
  // Before the + 1 below was NAGUs, but in reality for AGUs there is only one
  // resourc with throughput equal to the number of AGUs
  
  for (unsigned i = 0;
       i < NExecutionUnits + 1 + NMiscExecutionUnits + NPorts + NBuffers; i++) {
    
    unsigned IssueCycleGranularity = 0;
    unsigned AccessWidth = 0;
    
    if (i < NArithmeticExecutionUnits + NMovExecutionUnits) {
      // AccessWidth = VectorWidth;
      AccessWidth = 1;
      // Computational units throughput must also be rounded
      if (i < NArithmeticExecutionUnits) {
        if (this->ExecutionUnitsThroughput[i] != INF) {
          if (this->ExecutionUnitsThroughput[i] >= 1) {
            // If throughput is >=1 and less than VectorWdith, make sure it is
            // divisible
            if(this->ExecutionUnitsThroughput[i] <= VectorWidth){
              if(VectorWidth % int(this->ExecutionUnitsThroughput[i]) != 0 ){
                dbgs() << "Vector Width " <<  VectorWidth << "\n";
                dbgs() << "this->ExecutionUnitsThroughput[i] " <<
                this->ExecutionUnitsThroughput[i] << "\n";
                report_fatal_error("Throughput should be divisible by vector width");
              }
            }else{
              if(int(this->ExecutionUnitsThroughput[i]) % VectorWidth != 0 ){
                dbgs() << "Vector Width " <<  VectorWidth << "\n";
                dbgs() << "this->ExecutionUnitsThroughput[i] " <<
                this->ExecutionUnitsThroughput[i] << "\n";
                report_fatal_error("VectorWidth should be divisible by throughput");
              }
            }
          }
        }
      }
      else {
        if (this->ExecutionUnitsThroughput[i] != INF) {
          if (this->ExecutionUnitsThroughput[i] >= 1)
            this->ExecutionUnitsThroughput[i] = this->ExecutionUnitsThroughput[i];
        }
      }
    }
    else {
      if(i >= NArithmeticExecutionUnits + NMovExecutionUnits
          && i < NArithmeticExecutionUnits + NMovExecutionUnits + NMemExecutionUnits) {
        AccessWidth = roundNextMultiple (this->MemoryWordSize, AccessGranularities[i]);
        // Round throughput of memory resources to the next multiple of AccessWidth
        // (before it was MemoryWordSize)
        if (this->ExecutionUnitsThroughput[i] != INF) {
          if (this->ExecutionUnitsThroughput[i] < AccessWidth) {
            if (this->ExecutionUnitsThroughput[i] > 0 &&
                this->ExecutionUnitsThroughput[i] < 1 ) {
              float Inverse = ceil(1 / this->ExecutionUnitsThroughput[i]);
              float Rounded = roundNextPowerOfTwo(Inverse);
              if (Inverse == Rounded)
                this->ExecutionUnitsThroughput[i] = float (1)/float(Rounded);
              else
                this->ExecutionUnitsThroughput[i] = float (1)/float((Rounded/float(2)));
            }else{
              this->ExecutionUnitsThroughput[i] =
              roundNextPowerOfTwo(ceil (this->ExecutionUnitsThroughput[i]));
            }
          }else{
            // Round to the next multiple of AccessGranularities...
            this->ExecutionUnitsThroughput[i] =
            roundNextMultiple(this->ExecutionUnitsThroughput[i], AccessGranularities[i]);
          }
        }
      }else {
        AccessWidth = 1;
      }
    }
    
    if (this->ConstraintPortsx86){
      for (unsigned i = 0; i < NExecutionUnits; i++) {
        if (this->ExecutionUnitsParallelIssue[i] == INF &&
            DispatchPort[i].size() != 0) {
          dbgs() << "Execution unit: " << i<< "\n";
          dbgs() << "Execution unit: " << getResourceName(i) << "\n";
          dbgs() << "this->ExecutionUnitsParallelIssue[i] " <<
          this->ExecutionUnitsParallelIssue[i]<< "\n";
          report_fatal_error("Parallel Issue cannot be infinity if\
                              ConstraintPortsx86");
        }
      }
    }
    
    IssueCycleGranularity =
    getIssueCycleGranularity(i, AccessWidth, getNElementsAccess(i, AccessWidth,
                                                                this->VectorWidth));
    
    // Push the values of AccessWidth and IssueCycleGranularity into the
    // corresponding vectors. The values in these vectors are ordered
    // as the order specified above for all the execution units.
    AccessWidths.push_back(AccessWidth);
    IssueCycleGranularities.push_back(IssueCycleGranularity);
    if(this->ExecutionUnitsLatency[i] != 0 &&
       this->ExecutionUnitsLatency[i] < IssueCycleGranularity ){
      report_fatal_error("Latency ("+  Twine(this->ExecutionUnitsLatency[i]) + ") \
                         cannot be smaller than issue cycle granularity ("+
                         Twine(IssueCycleGranularity )+") for resource "+
                         getResourceName(i));
    }
  }
  
  //Some checkings....
  
  if (this->LoadBufferSize > 0 && this->ReservationStationSize == 0)
    report_fatal_error("RS cannot be zero if LB exists");
  
  if (this->StoreBufferSize > 0 && this->ReservationStationSize == 0)
    report_fatal_error("RS cannot be zero if SB exists");
  
  if (this->LineFillBufferSize > 0 && this->LoadBufferSize == 0)
    report_fatal_error("LB cannot be zero if LFB exists");
  
  //Check that access granularities are either memory word size or cache line size
  // Start from 1 because memory access granularity of register file is 1
  if (!MemAccessGranularity.empty()){
    for (unsigned i = 1; i < MemAccessGranularity.size(); i++)
    if (MemAccessGranularity[i] != MemoryWordSize &&
        MemAccessGranularity[i] != CacheLineSize){
          report_fatal_error("Memory access granularity ("+
                             Twine(MemAccessGranularity[i]) +
                             ") is not memory word size ("+ Twine(MemoryWordSize)+
                             "), nor cache line size ("+Twine(CacheLineSize)+ ")\n");
    }
  }
  
  if(this->L1CacheSize != 0 &&  this->L2CacheSize != 0 &&
     this->ExecutionUnitsLatency[L2_LOAD_CHANNEL] <=
     this->ExecutionUnitsLatency[L1_LOAD_CHANNEL])
    report_fatal_error("Latency of L1 cache cannot be larger or equal than\
                     latency to other levels");
  
  if(this->L2CacheSize != 0 && this->LLCCacheSize != 0 &&
     this-> ExecutionUnitsLatency[L3_LOAD_CHANNEL] <=
     this->ExecutionUnitsLatency[L2_LOAD_CHANNEL])
    report_fatal_error("Latency of L2 cache cannot be larger or equal than\
                     latency to other levels");
  
  if(this->LLCCacheSize != 0 &&
     this->ExecutionUnitsLatency[MEM_LOAD_CHANNEL]
     <= this->ExecutionUnitsLatency[L3_LOAD_CHANNEL])
    report_fatal_error("Latency of L3 cache cannot be larger or equal than\
                     latency to other levels");
  
  // Initialize global variables to zero...
  TotalInstructions = 0;
  InstructionFetchCycle = 0;
  RemainingInstructionsFetch = this->InstructionFetchBandwidth;
  
  FunctionCallStack = 0;
  BasicBlockBarrier = 0;
  
  LastLoadIssueCycle = 0;
  LastStoreIssueCycle = 0;
  LastInstructionIssueCycle = 0;
  
  NRegisterSpillsLoads = 0;
  NRegisterSpillsStores = 0;

  GlobalAddrForArtificialMemOps = roundNextMultiple(ULONG_MAX-64, 64);

  ReuseTree = NULL;
  PrefetchReuseTree = NULL;
  PrefetchReuseTreeSize = 0;
  LastIssueCycleFinal = 0;

  LoadBufferCompletionCyclesTree = NULL;
  DispatchToLoadBufferQueueTree = NULL;
  MinLoadBuffer = 0;
  MaxDispatchToLoadBufferQueueTree = 0;
  
  // This flag switches between two types of data structures implementing the
  // OoO buffers. If they are large, then threes are more appropiate. Otherwise,
  // Just use vectors
  SmallBuffers = false;
  
  // TODO: Remove
  DebugWarm = true;
  
  BitsPerCacheLine = log2(this->CacheLineSize * (this->MemoryWordSize));
  
  SourceCodeLine = 0;
  
  MaxLatencyResources = 0;
  
  
#ifndef STACK_DEQUE
  ReuseStack = *(new LinkedList<PointerToMemoryInstance>());
#endif
  
  // For resources with throughput and latency, i.e., resources for which we
  // insert cycles
  for (unsigned i = 0;
       i < NExecutionUnits + NPorts + NAGUs + NLoadAGUs + NStoreAGUs + NBuffers;
       i++) {
    InstructionsCount.push_back(0);
    InstructionsCountExtended.push_back(0);
    ScalarInstructionsCountExtended.push_back(0);
    VectorInstructionsCountExtended.push_back(0);
    InstructionsLastIssueCycle.push_back(0);
    IssueSpan.push_back(0);
    LatencyOnlySpan.push_back(0);
    SpanGaps.push_back(0);
    FirstNonEmptyLevel.push_back(0);
    MaxOccupancy.push_back(0);
    NInstructionsStalled.push_back(0);
    FirstIssue.push_back(0);
    AverageOverlaps.push_back(0);
    AverageOverlapsCycles.push_back(0);
    OverlapsCount.push_back(0);
    OverlapsDerivatives.push_back(1);
    OverlapsMetrics.push_back(0);
  }
  
  for (unsigned i = 0; i < NBuffers; i++)
    BuffersOccupancy.push_back(0);

#ifdef EFF_TBV
  for (int i = 0; i< MAX_RESOURCE_VALUE; i++)
    FullOccupancyCyclesTree.push_back(TBV_node());
#else
  FullOccupancyCyclesTree.push_back(*(new TBV()));
#endif
  
  for (unsigned i = 0;
       i < NExecutionUnits + NPorts + NAGUs + NLoadAGUs + NStoreAGUs + NBuffers;
       i++)
  AvailableCyclesTree.push_back(NULL);
  
  IssuePorts = vector < unsigned >();

  if ( NTotalResources != (NExecutionUnits + NPorts + NAGUs + NLoadAGUs +
                          NStoreAGUs + NBuffers)){
    dbgs() << "NTotalResources " << NTotalResources << "\n";
    dbgs() << "NExecutionUnits " << NExecutionUnits << "\n";
    dbgs() << "NPorts " << NPorts << "\n";
    dbgs() << "NAGUs " << NAGUs << "\n";
    dbgs() << "NLoadAGUs " << NLoadAGUs << "\n";
    dbgs() << "NStoreAGUs " << NStoreAGUs << "\n";
    dbgs() << "NBuffers " << NBuffers << "\n";
    report_fatal_error(" NTotalResources != (NExecutionUnits + NPorts + NAGUs +\
                       NLoadAGUs + NStoreAGUs + NBuffers");
  }
  
  
  CGSFCache.resize(NTotalResources);
  CISFCache.resize(NTotalResources);
  CLSFCache.resize(NTotalResources);

  // ================================================================//
  //		Some initizatializations for prefetcher
  // ================================================================//
  
  switch (PrefetchLevel) {
    case 1:
      this->PrefetchLevel = 0;
      PrefetchDestination = L1_LOAD_CHANNEL;
      break;
    case 2:
      this->PrefetchLevel = this->L1CacheSize;
      PrefetchDestination = L2_LOAD_CHANNEL;
      break;
    case 3:
      this->PrefetchLevel = this->L2CacheSize;
      PrefetchDestination = L3_LOAD_CHANNEL;
      break;
    default:
      report_fatal_error("Prefetch level not recognized");
      break;
  }
  
  switch (PrefetchDispatch) {
    case 0:
      this->PrefetchDispatch = 0;
      break;
    case 1:
      this->PrefetchDispatch = L1_STORE_NODE;
      break;
    case 2:
      this->PrefetchDispatch = L2_STORE_NODE;
      break;
    case 3:
      this->PrefetchDispatch = L3_STORE_NODE;
      break;
    default:
      report_fatal_error("Prefetch dispatch not recognized");
      break;
  }
  
  switch (PrefetchTarget) {
    case 2:
      this->PrefetchTarget = L1_STORE_CHANNEL;
      break;
    case 3:
      this->PrefetchTarget = L2_STORE_CHANNEL;
      break;
    case 4:
      this->PrefetchTarget = L3_STORE_CHANNEL;
      break;
    default:
      report_fatal_error("Prefetch target not recognized");
      break;
  }
}



//===----------------------------------------------------------------------===//
//                General functions for the analysis
//===----------------------------------------------------------------------===//

void DynamicAnalysis::insertCacheLineHistory(uint64_t CacheLine){
  if (CacheLinesHistory.size()==4){
    CacheLinesHistory.pop_front();
    CacheLinesHistory.push_back(CacheLine);
  }else{
    CacheLinesHistory.push_back(CacheLine);
  }
}

bool DynamicAnalysis::cacheLineRecentlyAccessed(uint64_t CacheLine){
  for(uint64_t n : CacheLinesHistory) {
    if(n == CacheLine-1 || n == CacheLine-1)
    return true;
  }
  return false;
}


#define HANDLE_MEMORY_TYPE(TY, type, operand)  \
Ty = I.getOperand(operand)->getType();  \
if(PointerType* PT = dyn_cast<PointerType>(Ty)){  \
switch (PT->getElementType()->getTypeID()) {  \
case Type::HalfTyID:  return FP_##TY##_16_BITS; \
case Type::FloatTyID:  {FloatPrecision = 0; return FP_##TY##_32_BITS; } \
case Type::DoubleTyID: {FloatPrecision = 1; return FP_##TY##_64_BITS; } \
case Type::X86_FP80TyID:  return FP_##TY##_80_BITS; \
case Type::PPC_FP128TyID:  return FP_##TY##_128_BITS; \
case Type::X86_MMXTyID:  return FP_##TY##_64_BITS;  \
case Type::VectorTyID: \
switch (PT->getElementType()->getVectorElementType()->getTypeID()) {  \
case Type::HalfTyID:  return FP_##TY##_16_BITS; \
case Type::FloatTyID:  \
{FloatPrecision = 0; return FP_##TY##_32_BITS; }  \
case Type::DoubleTyID:  \
{FloatPrecision = 1; return FP_##TY##_64_BITS; }\
case Type::IntegerTyID: \
switch (PT->getElementType()->getVectorElementType()->getIntegerBitWidth()) { \
case 4: return INT_##TY##_4_BITS; \
case 8: return INT_##TY##_8_BITS; \
case 16: return INT_##TY##_16_BITS; \
case 32: return INT_##TY##_32_BITS; \
case 64: return INT_##TY##_64_BITS; \
default: return MISC;  \
} \
default: return MISC;\
}\
case Type::IntegerTyID: \
IntegerTy = dyn_cast<IntegerType>(PT-> getElementType());  \
switch (IntegerTy -> getBitWidth()){  \
case 4: return INT_##TY##_4_BITS; \
case 8: return INT_##TY##_8_BITS; \
case 16: return INT_##TY##_16_BITS; \
case 32: return INT_##TY##_32_BITS; \
case 64: return INT_##TY##_64_BITS; \
default: return MISC;  \
} \
default: return MISC; \
} \
}else{  \
errs() << "Trying to load a non-pointer type.\n"; \
} \

#define HANDLE_FP_TYPE(TY)  \
Ty = I.getOperand(0)->getType();  \
switch (Ty->getTypeID()) {  \
case Type::FloatTyID:  {FloatPrecision = 0; return FP32_##TY;}  \
case Type::DoubleTyID: {FloatPrecision = 1;  return FP64_##TY;} \
case Type::VectorTyID: \
switch (Ty->getScalarType()->getTypeID()) {  \
case Type::FloatTyID:  {FloatPrecision = 0; return FP32_##TY;}  \
case Type::DoubleTyID:  {FloatPrecision = 1;  return FP64_##TY;} \
default: {return -1; errs() << "Vector FP Instruction type not recognized \
(16/32/64 bits).\n";}; \
}; \
default: {return -1; errs() << "FP Instruction type not recognized \
(16/32/64 bits).\n";}; \
} \

#define HANDLE_FP_NODE_TYPE(TY)  \
Ty = I.getOperand(0)->getType();  \
switch (Ty->getTypeID()) {  \
case Type::FloatTyID:  { FloatPrecision = 0; return FP32_##TY##_NODE; } \
case Type::DoubleTyID: { FloatPrecision = 1;  return FP64_##TY##_NODE; }\
case Type::VectorTyID: \
switch (Ty->getScalarType()->getTypeID()) {  \
case Type::FloatTyID:   { FloatPrecision = 0; return FP32_##TY##_NODE; } \
case Type::DoubleTyID:   { FloatPrecision = 1;  return FP64_##TY##_NODE; }\
default: errs() << "Vector FP Instruction type not recognized (16/32/64 bits).\n"; \
}; \
default: errs() << "FP Instruction type not recognized (16/32/64 bits).\n"; \
} \


string
DynamicAnalysis::getResourceName(unsigned Resource)
{
  switch (Resource) {
    case FP32_ADDER:
      return "FP32_ADDER";
    case FP64_ADDER:
      return "FP64_ADDER";
    case FP32_MULTIPLIER:
      return "FP32_MULTIPLIER";
    case FP64_MULTIPLIER:
      return "FP64_MULTIPLIER";
    case FP32_FMADDER:
      return "FP32_FMADDER";
    case FP64_FMADDER:
      return "FP64_FMADDER";
    case FP32_DIVIDER:
      return "FP32_DIVIDER";
    case FP64_DIVIDER:
      return "FP64_DIVIDER";
    case FP32_SHUFFLE_UNIT:
      return "FP32_SHUFFLE_UNIT";
    case FP64_SHUFFLE_UNIT:
      return "FP64_SHUFFLE_UNIT";
    case FP32_BLEND_UNIT:
      return "FP32_BLEND_UNIT";
    case FP64_BLEND_UNIT:
      return "FP64_BLEND_UNIT";
    case FP32_BOOL_UNIT:
      return "FP32_BOOL_UNIT";
    case FP64_BOOL_UNIT:
      return "FP64_BOOL_UNIT";
    case REGISTER_LOAD_CHANNEL:
      return "REGISTER_CHANNEL";
    case L1_LOAD_CHANNEL:
      return "L1_LOAD_CHANNEL";
    case L1_STORE_CHANNEL:
      return "L1_STORE_CHANNEL";
    case L2_LOAD_CHANNEL:
      return "L2";
    case L3_LOAD_CHANNEL:
      return "L3 ";
    case MEM_LOAD_CHANNEL:
      return "MEM_LOAD_CHANNEL";
    case ADDRESS_GENERATION_UNIT:
      return "AGU";
    
    case MISC_UNIT:
      return "MISC_EXECUTION_UNIT";
    case PORT_0:
      return "PORT_0";
    case PORT_1:
      return "PORT_1";
    case PORT_2:
      return "PORT_2";
    case PORT_3:
      return "PORT_3";
    case PORT_4:
      return "PORT_4";
    case PORT_5:
      return "PORT_5";
    
    case RS_STALL:
      return "RS";
    case ROB_STALL:
      return "ROB";
    case LB_STALL:
      return "LB";
    case SB_STALL:
      return "SB";
    case LFB_STALL:
      return "LFB";
    
    default:
    if (Resource > PORT_5){
      char numstr[21]; // enough to hold all numbers up to 64-bits
      sprintf(numstr, "%d", Resource-PORT_0);
      std::string prefix = "PORT_";
      return prefix  + numstr;
    }else{
      return "Resources not known";
    }
  }
}


string
DynamicAnalysis::getNodeName(unsigned Node)
{
  switch (Node) {
    case FP32_ADD_NODE:
      return "FP32_ADD_NODE";
    case FP64_ADD_NODE:
      return "FP64_ADD_NODE";
    case FP32_MUL_NODE:
      return "FP32_MUL_NODE";
    case FP64_MUL_NODE:
      return "FP64_MUL_NODE";
    case FP32_FMA_NODE:
      return "FP32_FMA_NODE";
    case FP64_FMA_NODE:
      return "FP64_FMA_NODE";
    case FP32_DIV_NODE:
      return "FP32_DIV_NODE";
    case FP64_DIV_NODE:
      return "FP64_DIV_NODE";
    case FP32_SHUFFLE_NODE:
      return "FP32_SHUFFLE_NODE";
    case FP64_SHUFFLE_NODE:
      return "FP64_SHUFFLE_NODE";
    case FP32_BLEND_NODE:
      return "FP32_BLEND_NODE";
    case FP64_BLEND_NODE:
      return "FP64_BLEND_NODE";
    case FP32_BOOL_NODE:
      return "FP32_BOOL_NODE";
    case FP64_BOOL_NODE:
      return "FP64_BOOL_NODE";
    case REGISTER_LOAD_NODE:
      return "REGISTER_LOAD_NODE";
    case REGISTER_STORE_NODE:
      return "REGISTER_STORE_NODE";
    case L1_LOAD_NODE:
      return "L1_LOAD_NODE";
    case L1_STORE_NODE:
      return "L1_STORE_NODE";
    case L2_LOAD_NODE:
      return "L2_LOAD_NODE";
    case L2_STORE_NODE:
      return "L2_STORE_NODE";
    case L3_LOAD_NODE:
      return "L3_LOAD_NODE ";
    case MEM_LOAD_NODE:
      return "MEM_LOAD_NODE ";
    case MEM_STORE_NODE:
      return "MEM_STORE_NODE";
    case AGU_NODE:
      return "AGU_NODE";
    case MISC_NODE:
      return "MISC_NODE";
    case PORT_0_NODE:
      return "PORT_0_NODE";
    case PORT_1_NODE:
      return "PORT_1_NODE";
    case PORT_2_NODE:
      return "PORT_2_NODE";
    case PORT_3_NODE:
    return "PORT_3_NODE";
      case PORT_4_NODE:
      return "PORT_4_NODE";
    case PORT_5_NODE:
      return "PORT_5_NODE";
    case RS_STALL_NODE:
      return "RS_NODE";
    case ROB_STALL_NODE:
      return "ROB_NODE";
    case LB_STALL_NODE:
      return "LB_NODE";
    case SB_STALL_NODE:
      return "SB_NODE";
    case LFB_STALL_NODE:
      return "LFB_NODE";
    
    default:
      return "Node not known";
  }
}


// Copy from Instruction.cpp-getOpcodeName()
// Opcode numbers defined in /include/llvm/IR/Instruction.def
int
DynamicAnalysis::getInstructionType(Instruction & I)
{
  IntegerType *IntegerTy;
  Type *Ty;
  unsigned Opcode = I.getOpcode ();
  
  switch (Opcode) {
    // Terminators
    case Instruction::Ret:
      return CTRL;
    case Instruction::Br:
      return CTRL;
    case Instruction::Switch:
      return CTRL;
    case Instruction::IndirectBr:
      return CTRL;
    case Instruction::Invoke:
      return CTRL;
    case Instruction::Resume:
      return CTRL;
    case Instruction::Unreachable:
      return CTRL;
    
    // Standard binary operators...
    case Instruction::Add:
      return INT_ADD;
    case Instruction::FAdd:
      HANDLE_FP_TYPE(ADD);
    case Instruction::Sub:
      return INT_SUB;
    case Instruction::FSub:
      HANDLE_FP_TYPE(SUB)
    case Instruction::Mul:
      return INT_MUL;
    case Instruction::FMul:
      HANDLE_FP_TYPE(MUL)
    case Instruction::UDiv:
      return INT_DIV;
    case Instruction::SDiv:
      return INT_DIV;
    case Instruction::FDiv:
      HANDLE_FP_TYPE(DIV)
    case Instruction::URem:
      return INT_REM;
    case Instruction::SRem:
      return INT_REM;
    case Instruction::FRem:
      HANDLE_FP_TYPE(DIV)
    
    // Logical operators...
    case Instruction::And:
      return MISC;
    case Instruction::Or:
      return MISC;
    case Instruction::Xor:
      return MISC;
    
    // Memory instructions...
    case Instruction::Alloca:
      return MISC_MEM;
    case Instruction::Load:
      HANDLE_MEMORY_TYPE (LD, Load, 0)
    case Instruction::Store:
      HANDLE_MEMORY_TYPE (ST, Store, 1);
    case Instruction::AtomicCmpXchg:
      return MISC_MEM;
    case Instruction::AtomicRMW:
      return MISC_MEM;
    case Instruction::Fence:
      return MISC_MEM;
    case Instruction::GetElementPtr:
      return MISC_MEM;
    
    // Convert instructions...
    case Instruction::Trunc:
      return MISC;
    case Instruction::ZExt:
      return MISC;
    case Instruction::SExt:
      return MISC;
    case Instruction::FPTrunc:
      return MISC;
    case Instruction::FPExt:
      return MISC;
    case Instruction::FPToUI:
      return MISC_CONVERT_SELECT;
    case Instruction::FPToSI:
      return MISC;
    //return MISC;
    case Instruction::UIToFP:
    //return MISC_CONVERT_SELECT;
      return MISC;
    case Instruction::SIToFP:
    //return MISC_CONVERT_SELECT;
      return MISC;
    case Instruction::IntToPtr:
    return MISC;
      case Instruction::PtrToInt:
      return MISC;
    case Instruction::BitCast:
      return MISC;
    
    // Other instructions...
    case Instruction::ICmp:
      return CTRL;
    case Instruction::FCmp:
      HANDLE_FP_TYPE(ADD);
    
    case Instruction::PHI:
      return CTRL;
    case Instruction::Select:
    {
       int type = MISC;
       for(unsigned i = 0; i<  I.getNumOperands(); i++){
       if(I.getOperand(i)->getType()->getTypeID() == Type::FloatTyID)
       type = MISC_CONVERT_SELECT;
       else if (I.getOperand(i)->getType()->getTypeID() == Type::DoubleTyID)
       type = MISC_CONVERT_SELECT;
       // TODO: Check for vector code
       };
      return type;
    }
    case Instruction::Call:
      return CTRL;
    case Instruction::Shl:
      return MISC;
    case Instruction::LShr:
      return MISC;
    case Instruction::AShr:
      return MISC;
    case Instruction::VAArg:
      return MISC;
    case Instruction::ExtractElement:
      HANDLE_FP_TYPE(BLEND)
    case Instruction::InsertElement:
      HANDLE_FP_TYPE(BLEND)
    case Instruction::ShuffleVector:
      HANDLE_FP_TYPE(SHUFFLE)
    case Instruction::ExtractValue:
      return FP32_BLEND;
    //return MISC;
    case Instruction::InsertValue:
      return MISC;
    case Instruction::LandingPad:
      return MISC;
    // These are custom codes defined in Execution.cpp
    case FP32_BLEND_INST:
      return FP32_BLEND;
    case FP64_BLEND_INST:
      return FP64_BLEND;
    case FP32_FMA_INST:
      return FP32_FMA;
    case FP64_FMA_INST:
      return FP64_FMA;
    case FP32_BOOL_INST:
      return FP32_BOOL;
    case FP64_BOOL_INST:
      return FP64_BOOL;
    // These are different from Instruction::ShuffleVector because if
    // a shuffle from an intrinsic, we cannot get the type from
    // the instruction, and hence HANDLE_FP_TYPE will not work.
    case FP32_SHUFFLE_INST:
      return FP32_SHUFFLE;
    case FP64_SHUFFLE_INST:
      return FP64_SHUFFLE;
    
    default:
      return -1;
  }
}


// TODO: Have a class for all the intrinsics, with members that are the
// different microops, the last repetition, etc.
// and use it. Hence, adding a new intrinsic is automatic
unsigned
DynamicAnalysis::getLastRepetitionIntrinsic(string functionName)
{
  if (functionName.find("llvm.x86.avx.vbroadcast") != string::npos){
    return 1;
  }else if (functionName.find( "llvm.x86.avx.maskload.pd.256") != string::npos){
    return 1;
  }
  else if (functionName.find( "llvm.x86.avx.maskstore.pd.256") != string::npos){
    return 0;
  }
  else if (functionName.find( "llvm.x86.avx.storeu.pd.256") != string::npos){
    return 0;
  }
  else if (functionName.find("llvm.x86.avx.blend")  != string::npos) {
    return 0;
  }
  else if (functionName.find("@llvm.x86.sse3.hadd.ps") != string::npos){
    return 0;
  }
  else if (functionName.find("llvm.x86.avx.hadd.pd.256") != string::npos){
    return 2;
  }
  else if (functionName.find( "llvm.x86.avx.vperm2f128.pd.256") != string::npos){
    return 0;
  }
  return 0;
}


unsigned
DynamicAnalysis::getLastNonMemRepetitionIntrinsic(string functionName)
{
  if (functionName.find( "llvm.x86.avx.maskstore.pd.256") != string::npos)
    return 1;
  else
    report_fatal_error("Function "+functionName+" not recognized");
  
  return 0;
}


void
DynamicAnalysis::getOperandsPositionsIntrinsic(string functionName,
                                               vector<int64_t> & positions,
                                               unsigned valueRep)
{
  if (functionName.find("exp") != string::npos) {
    positions.push_back(0);
  }else if (functionName.find("sqrt") != string::npos) {
    positions.push_back(0);
  }else if (functionName.find("cos") != string::npos) {
    positions.push_back(0);
  }else if (functionName.find("fabs") != string::npos) {
				positions.push_back(0);
  }else if (functionName.find("pow") != string::npos) {
    positions.push_back(0);
  }else if (functionName.find("floorf") != string::npos) {
    positions.push_back(0);
  }else if (functionName.find("llvm.x86.avx.vbroadcast") != string::npos) {
    //Broadcast a double-precision (64-bit) floating-point element from
    // memory to all elements of dstt (a 256-bit register)
    positions.push_back(-1);
  }else if (functionName.find( "llvm.x86.avx.vperm2f128.pd.256") != string::npos) {
    positions.push_back(0);
    positions.push_back(1);
  }else if (functionName.find( "llvm.x86.avx.blend.pd.256") != string::npos) {
    positions.push_back(0);
    positions.push_back(1);
  }else if (functionName.find( "llvm.x86.avx.hadd.pd.256") != string::npos) {
    positions.push_back(0);
    positions.push_back(1);
  }else if (functionName.find( "llvm.x86.avx.maskload.pd.256") != string::npos){
    positions.push_back(-1);
  }else if (functionName.find( "llvm.x86.avx.maskstore.pd.256") != string::npos){
    // The previous
    if(valueRep > 1)
      positions.push_back(-1);
    else
      positions.push_back(2);
  }else{
    report_fatal_error("Operands positions information not available for \
                       intrinsic "+functionName);
  }
}


int64_t
DynamicAnalysis::getStoreOperandPositionIntrinsic(string functionName)
{
  if (functionName.find("llvm.x86.avx.maskstore.pd.256") != string::npos) {
    return -1;
  }else if (functionName.find( "llvm.x86.avx.storeu.pd.256") != string::npos) {
    return 1;
  }else{
    report_fatal_error("Operands positions information not available for \
                       intrinsic/call to function "+functionName);
  }
}


uint64_t
DynamicAnalysis::getInstructionValueIssueCycle(Value * v)
{
  uint64_t InstructionIssueCycle = 0;
  map < Value *, uint64_t >::iterator IssueCycleMapIt;
  
  IssueCycleMapIt = InstructionValueIssueCycleMap.find (v);
  if (IssueCycleMapIt != InstructionValueIssueCycleMap.end()) {
    InstructionIssueCycle = IssueCycleMapIt->second;
    // Reset the value of issue cyle after reading it so that
    // when the next time this instruction is executed, it it not poluted
    // with a previous value. This problems arises when two instances of the
    // same instruction are represented by the same value.
    //  InstructionValueIssueCycleMap.erase(IssueCycleMapIt);
    IssueCycleMapIt->second = 0;
  }
  else
    InstructionIssueCycle = 0;	// First usage
  
  return InstructionIssueCycle;
}


void
DynamicAnalysis::insertInstructionValueIssueCycle(Value * v,
                                                  uint64_t InstructionIssueCycle,
                                                  bool IsPHINode)
{
  map < Value *, uint64_t >::iterator IssueCycleMapIt;
  IssueCycleMapIt = InstructionValueIssueCycleMap.find(v);
  
  if (IssueCycleMapIt != InstructionValueIssueCycleMap.end()) {
    if (IsPHINode == true)
    IssueCycleMapIt->second = InstructionIssueCycle;
    else{
      IssueCycleMapIt->second = max(IssueCycleMapIt->second,
                                    InstructionIssueCycle);
    }
  }else//Insert an entry for the instrucion.
  InstructionValueIssueCycleMap[v] = InstructionIssueCycle;
}


void
DynamicAnalysis::insertInstructionValueName(Value * v)
{
  map < Value *,Value * >::iterator it;
  
  it = InstructionValueInstructionNameMap.find (v);
  if (it == InstructionValueInstructionNameMap.end())
  InstructionValueInstructionNameMap[v] = v;
}


void
DynamicAnalysis::printInstructionValueNames()
{
  map < Value *,Value * >::iterator it;
  
  for (it = InstructionValueInstructionNameMap.begin();
       it!= InstructionValueInstructionNameMap.end(); it++)
  dbgs() << (*it).first << " " << *((*it).second) << "\n";
}

unsigned
DynamicAnalysis::getExtendedInstructionType(Instruction &I, int OpCode,
                                             int ReuseDistance,
                                             int RegisterStackReuseDistance)
{
  unsigned InstructionType = 0;
  Type *Ty;
  
  switch (OpCode) {
    case Instruction::Add:
#ifdef INT_FP_OPS
    return INT_ADD_NODE;
#else
    report_fatal_error("Instruction type not associated with a node");
#endif
    
    case Instruction::Sub:
#ifdef INT_FP_OPS
    return INT_ADD_NODE;
#else
    report_fatal_error("Instruction type not associated with a node");
#endif
    case Instruction::Mul:
#ifdef INT_FP_OPS
    return INT_MUL_NODE;
#else
    report_fatal_error("Instruction type not associated with a node");
#endif
    case Instruction::UDiv:
    case Instruction::SDiv:
#ifdef INT_FP_OPS
    return INT_DIV_NODE;
#else
    report_fatal_error("Instruction type not associated with a node");
#endif
    case Instruction::FAdd:
    HANDLE_FP_NODE_TYPE(ADD);
    case Instruction::FSub:
      HANDLE_FP_NODE_TYPE(ADD);
    
    case Instruction::FCmp:
      HANDLE_FP_NODE_TYPE(ADD);
    
    case Instruction::FMul:{
      HANDLE_FP_NODE_TYPE(MUL);
    }
    case Instruction::FDiv:
      HANDLE_FP_NODE_TYPE(DIV);
    
    case Instruction::ExtractValue:
      return FP32_BLEND_NODE;
    
    case Instruction::InsertElement:
    case Instruction::ExtractElement:
      HANDLE_FP_NODE_TYPE(BLEND);
    case Instruction::ShuffleVector:
      HANDLE_FP_NODE_TYPE(SHUFFLE);
    
    case Instruction::Load:
      if(RegisterStackReuseDistance >= 0)
        return REGISTER_LOAD_NODE;
      else{
        if(ReuseDistance < 0){
          if(L1CacheSize == 0)
            return L1_LOAD_NODE;
          else
            return MEM_LOAD_NODE;
        }else{
          if(L1CacheSize == 0)
          report_fatal_error("L1 cache size cannot be zero is reuse distance > 0");
          else{
            if(ReuseDistance <= (int) L1CacheSize)
              return L1_LOAD_NODE;
            else{
              if(L2CacheSize == 0)
                return MEM_LOAD_NODE;
              else{
                if(ReuseDistance <=  (int) L2CacheSize)
                  return L2_LOAD_NODE;
                else{
                  if(LLCCacheSize == 0)
                    return MEM_LOAD_NODE;
                  else{
                    if(ReuseDistance <=  (int) LLCCacheSize)
                      return L3_LOAD_NODE;
                    else
                      return MEM_LOAD_NODE;
                  }
                }
              }
            }
          }
        }
      }
      report_fatal_error("Instruction type not associated with a node");
      break;
    case Instruction::Store:
      if(RegisterStackReuseDistance >= 0)
      return REGISTER_STORE_NODE;
      else{
        if(ReuseDistance < 0){
          if(L1CacheSize == 0)
          return L1_STORE_NODE;
          else
          return MEM_STORE_NODE;
        }else{
          if(L1CacheSize == 0)
            report_fatal_error("L1 cache size cannot be zero is reuse distance > 0");
          else{
            if(ReuseDistance <= (int) L1CacheSize)
              return L1_STORE_NODE;
            else{
              if(L2CacheSize == 0)
                return MEM_STORE_NODE;
              else{
                if(ReuseDistance <=  (int) L2CacheSize)
                  return L2_STORE_NODE;
                else{
                  if(LLCCacheSize == 0)
                    return MEM_STORE_NODE;
                  else{
                    if(ReuseDistance <=  (int) LLCCacheSize)
                      return L3_STORE_NODE;
                    else
                      return MEM_STORE_NODE;
                  }
                }
              }
            }
          }
        }
      }
      report_fatal_error("Instruction type not associated with a node");
      break;
    case Instruction::Select:
      return FP32_BLEND_NODE;
    case Instruction::FPToUI:
    case Instruction::FPToSI:
    //case Instruction::UIToFP:
    //case Instruction::SIToFP:
      return MISC_NODE;
    case FP32_BLEND_INST:
      return FP32_BLEND_NODE;
    case FP64_BLEND_INST:
      return FP64_BLEND_NODE;
    case FP32_FMA_INST:
      return FP32_FMA_NODE;
    case FP64_FMA_INST:
      return FP64_FMA_NODE;
    case FP32_BOOL_INST:
      return FP32_BOOL_NODE;
    case FP64_BOOL_INST:
      return FP64_BOOL_NODE;
    case FP32_SHUFFLE_INST:
      return FP32_SHUFFLE_NODE;
    case FP64_SHUFFLE_INST:
      return FP64_SHUFFLE_NODE;
    
    default:
    report_fatal_error("Instruction type not associated with a node");
  }
  return InstructionType;
}

// getMemoryInstructionType has to be updated: reuse distance is given in
// number of distict cache lines
unsigned
DynamicAnalysis::getMemoryInstructionType(int ReuseDistance,
                                          uint64_t MemoryAddress,
                                          bool isLoad)
{
  if (ReuseDistance < 0) {
    if (L1CacheSize == 0) {
      if (isLoad == true)
      return L1_LOAD_NODE;
      else
      return L1_STORE_NODE;
    }
    else {
      return MEM_LOAD_NODE;
    }
  }
  
  if (ReuseDistance <= (int) RegisterFileSize)
  return REGISTER_LOAD_NODE;
  
  if ((int) RegisterFileSize < ReuseDistance && ReuseDistance <= (int) L1CacheSize){
    if (isLoad == true)
    return L1_LOAD_NODE;
    else
    return L1_STORE_NODE;
  }
  
  if ((int) L1CacheSize < ReuseDistance && ReuseDistance <= (int) L2CacheSize)
  return L2_LOAD_NODE;
  
  if ((int) L2CacheSize < ReuseDistance && ReuseDistance <= (int) LLCCacheSize)
  return L3_LOAD_NODE;
  
  return MEM_LOAD_NODE;
  
}

//===----------------------------------------------------------------------===//
//        Routines for handling memory addresses and cache lines
//===----------------------------------------------------------------------===//

CacheLineInfo DynamicAnalysis::getCacheLineInfo(uint64_t v)
{
  CacheLineInfo Info;
  uint64_t CacheLineIssueCycle = 0;
  uint64_t CacheLineLastAccess = 0;
  map < uint64_t, CacheLineInfo >::iterator IssueCycleMapIt;
  IssueCycleMapIt = CacheLineIssueCycleMap.find(v);
  
  if (IssueCycleMapIt != CacheLineIssueCycleMap.end()) {
    CacheLineIssueCycle = IssueCycleMapIt->second.IssueCycle;
    CacheLineLastAccess = IssueCycleMapIt->second.LastAccess;
  }else{
    CacheLineIssueCycle = 0;	// First usage
    CacheLineLastAccess = 0;
  }
  Info.LastAccess = CacheLineLastAccess;
  Info.IssueCycle = CacheLineIssueCycle;
  
  return (Info);
}


uint64_t
DynamicAnalysis::getCacheLineLastAccess(uint64_t v)
{
  uint64_t InstructionLastAccess = 0;
  map < uint64_t, CacheLineInfo >::iterator IssueCycleMapIt;
  
  IssueCycleMapIt = CacheLineIssueCycleMap.find (v);
  if (IssueCycleMapIt != CacheLineIssueCycleMap.end())
  InstructionLastAccess = IssueCycleMapIt->second.LastAccess;
  
  return InstructionLastAccess;
}


uint64_t
DynamicAnalysis::getMemoryAddressIssueCycle(uint64_t v)
{
  uint64_t IssueCycle = 0;
  map < uint64_t, uint64_t >::iterator IssueCycleMapIt;
  IssueCycleMapIt = MemoryAddressIssueCycleMap.find(v);
  
  if (IssueCycleMapIt != MemoryAddressIssueCycleMap.end())
    IssueCycle = IssueCycleMapIt->second;
  
  return IssueCycle;
}

  
void
DynamicAnalysis::insertCacheLineInfo(uint64_t v, CacheLineInfo Info)
{
  map < uint64_t, CacheLineInfo >::iterator IssueCycleMapIt;
  IssueCycleMapIt = CacheLineIssueCycleMap.find(v);
  
  if (IssueCycleMapIt != CacheLineIssueCycleMap.end()) {
    IssueCycleMapIt->second.IssueCycle = max(IssueCycleMapIt->second.IssueCycle,
                                             Info.IssueCycle);
    IssueCycleMapIt->second.LastAccess = Info.LastAccess;
  }
  else {			//Insert an entry for the instrucion.
    CacheLineIssueCycleMap[v].IssueCycle = Info.IssueCycle;
    CacheLineIssueCycleMap[v].LastAccess = Info.LastAccess;
  }
}


void
DynamicAnalysis::insertCacheLineLastAccess(uint64_t v, uint64_t LastAccess)
{
  map < uint64_t, CacheLineInfo >::iterator IssueCycleMapIt;
  IssueCycleMapIt = CacheLineIssueCycleMap.find(v);
  
  if (IssueCycleMapIt != CacheLineIssueCycleMap.end())
    IssueCycleMapIt->second.LastAccess = LastAccess;
  else				//Insert an entry for the instrucion.
    CacheLineIssueCycleMap[v].LastAccess = LastAccess;
}


void
DynamicAnalysis::insertMemoryAddressIssueCycle(uint64_t v, uint64_t Cycle)
{
  map < uint64_t, uint64_t >::iterator IssueCycleMapIt;
  IssueCycleMapIt = MemoryAddressIssueCycleMap.find(v);
  
  if (IssueCycleMapIt != MemoryAddressIssueCycleMap.end())
    IssueCycleMapIt->second = Cycle;
  else				//Insert an entry for the instrucion.
    MemoryAddressIssueCycleMap[v] = Cycle;
}

  
//===----------------------------------------------------------------------===//
//        Routines for managing bandwidth and throughput
//===----------------------------------------------------------------------===//

#ifdef EFF_TBV
void DynamicAnalysis::getTreeChunk(uint64_t i, unsigned int ExecutionResource)
{
  unsigned int size = FullOccupancyCyclesTree[ExecutionResource].BitVector.size();
  if (i >= size){
    FullOccupancyCyclesTree[ExecutionResource].BitVector.resize(i+ SplitTreeRange);
  }
}
#else
uint64_t DynamicAnalysis::getTreeChunk(uint64_t i)
{
  uint64_t TreeChunk = i / SplitTreeRange;
  if (TreeChunk >= FullOccupancyCyclesTree.size()) {
    FullOccupancyCyclesTree.resize(TreeChunk + 1);
  }
  return TreeChunk;
}
#endif

unsigned
DynamicAnalysis::findNextAvailableIssueCyclePortAndThroughtput(unsigned InstructionIssueCycle,
                                                               unsigned ExtendedInstructionType,
                                                               unsigned NElementsVector)
{
  unsigned ExecutionResource = ExecutionUnit[ExtendedInstructionType];
  unsigned InstructionIssueCycleThroughputAvailable = InstructionIssueCycle;
  
  uint64_t InstructionIssueCycleFirstTimeAvailable = 0;
  uint64_t InstructionIssueCyclePortAvailable = InstructionIssueCycle;
  uint64_t Port = 0;
  
  bool FoundInThroughput = false;
  bool FoundInPort;
  
  if(ExecutionUnitsThroughput[ExecutionResource]==0){
    report_fatal_error("Throughput cannot be 0 for resource "+
                       getResourceName(ExecutionResource));
  }

  if (ConstraintPorts)
    FoundInPort = false;
  else
    FoundInPort = true;
  
  while (FoundInThroughput == false || FoundInPort == false) {
    if(ConstraintPorts == false)
      InstructionIssueCyclePortAvailable = InstructionIssueCycleThroughputAvailable;
    // First, find next available issue cycle based on node throughput
    InstructionIssueCycleThroughputAvailable =
				findNextAvailableIssueCycle(InstructionIssueCyclePortAvailable,
                                    ExecutionResource, NElementsVector);
    if (InstructionIssueCycleThroughputAvailable == InstructionIssueCyclePortAvailable)
      FoundInThroughput = true;
    
    // Check that the port is available
    // Get the ports with which this node is assocaited
    if (ConstraintPorts) {
      bool FirstPortAvailableFound = false;
      if(DispatchPort[ExtendedInstructionType].size()== 0 )
        FoundInPort = true;
      for (unsigned i = 0; i < DispatchPort[ExtendedInstructionType].size(); i++){
        // IssuePorts contains ports that have issued instructions in
        // "issuecyclegranularity" cycles before or after current
        // cycle. IssuePorts is filled in ThereIsAvailableBandwidth,
        // which is called from FindNextAvailableIssueCycle
        bool PortAlreadyDispatch = false;
        for (unsigned j = 0; j< IssuePorts.size();j++){
          if (DispatchPort[ExtendedInstructionType][i] == IssuePorts[j]){
            PortAlreadyDispatch = true;
            break;
          }
        }
        
        if (IssuePorts.size() == 0 || ( PortAlreadyDispatch== false)) {
          // Checking availability in port
          InstructionIssueCyclePortAvailable =
          findNextAvailableIssueCycle(InstructionIssueCycleThroughputAvailable,
                                      DispatchPort[ExtendedInstructionType][i]);
 
          if (InstructionIssueCyclePortAvailable !=
              InstructionIssueCycleThroughputAvailable) {
            
            if(FirstPortAvailableFound == false){ // First time
              FirstPortAvailableFound = true;
              InstructionIssueCycleFirstTimeAvailable = InstructionIssueCyclePortAvailable;
            }else {
              InstructionIssueCyclePortAvailable =
              min (InstructionIssueCyclePortAvailable,
                   InstructionIssueCycleFirstTimeAvailable);
              if (InstructionIssueCyclePortAvailable ==
                  InstructionIssueCycleFirstTimeAvailable) {
                Port = i;
                if (InstructionIssueCyclePortAvailable ==
                    InstructionIssueCycleThroughputAvailable) {
                  FoundInPort = true;
                  break;
                }
              }
            }
          }else {
            // If Node is NULL, it is available for sure.
            FoundInPort = true;
            Port = i;
            break;
          }
        }
      }
    } // End of if(ConstraintPorts)
  }// End of while
  
  //Insert issue cycle in Port and in resource
  if(ConstraintPorts && DispatchPort[ExtendedInstructionType].size() != 0)
    insertNextAvailableIssueCycle(InstructionIssueCyclePortAvailable,
                                  DispatchPort[ExtendedInstructionType][Port]);
  
  // Insert in resource
  if (DispatchPort[ExtendedInstructionType].size() != 0)
    insertNextAvailableIssueCycle(InstructionIssueCyclePortAvailable,
                                  ExecutionResource,
                                  getNElementsAccess(ExecutionResource,
                                                     AccessWidths[ExecutionResource],
                                                     NElementsVector),
                                DispatchPort[ExtendedInstructionType][Port]);
  else
    insertNextAvailableIssueCycle(InstructionIssueCyclePortAvailable,
                                  ExecutionResource,
                                  getNElementsAccess(ExecutionResource,
                                                     AccessWidths[ExecutionResource],
                                                     NElementsVector));
  
  return InstructionIssueCyclePortAvailable;
}


unsigned
DynamicAnalysis::getNElementsAccess(unsigned ExecutionResource,
                                    unsigned AccessWidth,
                                    unsigned nElementsVector)
  {
  unsigned nElementsAccess = 1;
  if (ExecutionResource <  NArithmeticExecutionUnits + NMovExecutionUnits)
    nElementsAccess =  nElementsVector;
  else{
    if (ExecutionResource <
        NArithmeticExecutionUnits + NMovExecutionUnits + NMemExecutionUnits){
      if(VectorWidth*MemoryWordSize < AccessWidth)
        nElementsAccess = 1;
      else
        nElementsAccess = nElementsVector;
    }
  }
  return nElementsAccess;
}

  
unsigned
DynamicAnalysis::getIssueCycleGranularity(unsigned ExecutionResource,
                                          unsigned AccessWidth,
                                          unsigned NElementsVector)
  {
  unsigned IssueCycleGranularity = 1;
  
  if (ExecutionUnitsThroughput[ExecutionResource] != INF &&
      ExecutionUnitsParallelIssue[ExecutionResource] == INF){
    IssueCycleGranularity =
    unsigned(ceil(AccessWidth * NElementsVector/
                  (ExecutionUnitsThroughput[ExecutionResource])));
  }
  
  if (ExecutionUnitsThroughput[ExecutionResource] == INF
      && ExecutionUnitsParallelIssue[ExecutionResource] != INF) {
    if (ShareThroughputAmongPorts[ExecutionResource]) {
      IssueCycleGranularity =
      unsigned (ceil (AccessWidth * NElementsVector/
                      (ExecutionUnitsParallelIssue[ExecutionResource])));
    }
  }
  
  if (ExecutionUnitsThroughput[ExecutionResource] != INF &&
      ExecutionUnitsParallelIssue[ExecutionResource] != INF) {
    if (ShareThroughputAmongPorts[ExecutionResource]) {
      // *2 becuase throughput is shared among 2 ports (in the case of SB)
      IssueCycleGranularity =
      unsigned (ceil (AccessWidth * NElementsVector/
                      (ExecutionUnitsThroughput[ExecutionResource] *2)));
      
    }else{
      if (ExecutionResource >= L2_LOAD_CHANNEL)
        IssueCycleGranularity =
      unsigned (ceil (AccessWidth/(ExecutionUnitsThroughput[ExecutionResource])));
      else{
        double intpart;
        double tmpIssueCycleGranularity =
        (double)(AccessWidth *  NElementsVector)/
        (ExecutionUnitsThroughput[ExecutionResource]);
        double fractpart = modf (tmpIssueCycleGranularity , &intpart);
        if (fractpart <= 0.005)
          IssueCycleGranularity =
            unsigned (floor (AccessWidth *  NElementsVector/
                         (ExecutionUnitsThroughput[ExecutionResource])));
        else
          IssueCycleGranularity =
          unsigned (ceil (AccessWidth * NElementsVector/
                          (ExecutionUnitsThroughput[ExecutionResource])));
      }
    }
  }
  return IssueCycleGranularity;
}


float
DynamicAnalysis::getEffectiveThroughput(unsigned ExecutionResource,
                                        unsigned AccessWidth,
                                        unsigned NElementsVector)
  {
  // IssueCycleGranularity already takes into account the number of elements
  // in the vector
  unsigned IssueCycleGranularity =
    getIssueCycleGranularity(ExecutionResource,AccessWidth,
                             getNElementsAccess(ExecutionResource, AccessWidth,
                                                NElementsVector));
  if (ExecutionUnitsParallelIssue[ExecutionResource] != INF) {
    if (ShareThroughputAmongPorts[ExecutionResource]) {
      if (NElementsVector > 1)
        return (float)NElementsVector/(float)IssueCycleGranularity;
      else
        return ((float)NElementsVector/(float)IssueCycleGranularity)*
      (ExecutionUnitsParallelIssue[ExecutionResource]);
    }else{
      return NElementsVector*
      ExecutionUnitsParallelIssue[ExecutionResource]/(float)IssueCycleGranularity;
    }
  }else{
    return INF;
  }
}
  

unsigned
DynamicAnalysis::getNodeWidthOccupancy(unsigned ExecutionResource,
                                       unsigned AccessWidth,
                                       unsigned NElementsVector)
  {
  unsigned NodeWidthOccupancy =0;
  
  if (ExecutionUnitsThroughput[ExecutionResource] == INF &&
     ExecutionUnitsParallelIssue[ExecutionResource] == INF)
    NodeWidthOccupancy = AccessWidth*NElementsVector;
  
  if (ExecutionUnitsThroughput[ExecutionResource] != INF &&
      ExecutionUnitsParallelIssue[ExecutionResource] == INF) {
    if(AccessWidth*NElementsVector <= ExecutionUnitsThroughput[ExecutionResource])
      NodeWidthOccupancy = AccessWidth*NElementsVector;
    else
      NodeWidthOccupancy = ceil(ExecutionUnitsThroughput[ExecutionResource]);
  }
  
  if (ExecutionUnitsThroughput[ExecutionResource] == INF &&
      ExecutionUnitsParallelIssue[ExecutionResource] != INF)
    NodeWidthOccupancy = AccessWidth*NElementsVector;
  
  if (ExecutionUnitsThroughput[ExecutionResource] != INF &&
      ExecutionUnitsParallelIssue[ExecutionResource] != INF) {
    if (ShareThroughputAmongPorts[ExecutionResource]) {
      if(AccessWidth*NElementsVector <=
         ExecutionUnitsThroughput[ExecutionResource] *
         ExecutionUnitsParallelIssue[ExecutionResource])
        NodeWidthOccupancy = AccessWidth*NElementsVector;
      else
        NodeWidthOccupancy = ExecutionUnitsThroughput[ExecutionResource]*
        ExecutionUnitsParallelIssue[ExecutionResource];
    }else{
      if (AccessWidth*NElementsVector <= ExecutionUnitsThroughput[ExecutionResource])
        NodeWidthOccupancy = AccessWidth*NElementsVector;
      else
        NodeWidthOccupancy = ExecutionUnitsThroughput[ExecutionResource];
    }
  }
  return NodeWidthOccupancy;
}


bool
DynamicAnalysis::getLevelFull(unsigned ExecutionResource,
                              unsigned AccessWidth,
                              unsigned NElementsVector,
                              unsigned NodeIssueOccupancy,
                              unsigned NodeWidthOccupancy,
                              bool PotentiallyFull)
{
  bool LevelFull  = false;

  if (ExecutionUnitsThroughput[ExecutionResource] == INF &&
      ExecutionUnitsParallelIssue[ExecutionResource] != INF) {
    if (NodeIssueOccupancy == (unsigned)ExecutionUnitsParallelIssue[ExecutionResource])
      LevelFull = true;
  }
  
  if (ExecutionUnitsThroughput[ExecutionResource] != INF &&
      ExecutionUnitsParallelIssue[ExecutionResource] != INF) {
    if (NodeIssueOccupancy == (unsigned)ExecutionUnitsParallelIssue[ExecutionResource])
      LevelFull = true;
    
    else if (ShareThroughputAmongPorts[ExecutionResource]) {
      if (PotentiallyFull){
        // If share and checking whether the level will potentially get full,
        // we don't allow the existing width plus the width of the new op to be
        // larger than th*parallel issue, unless NodeIssueOccupany is zero
        // (it will become one after issuing this op). In this case, it could
        // happen that even  sharing throughput among ports, an operation width
        // is larger that th*parallel issue, although  I have not seen this in
        // practice. Hence, put an error when this rare case happens.
        if (NodeWidthOccupancy +
            getNodeWidthOccupancy(ExecutionResource, AccessWidth, NElementsVector) >
            ExecutionUnitsThroughput[ExecutionResource] *
            ExecutionUnitsParallelIssue[ExecutionResource]){
          if (NodeIssueOccupancy > 0)
            LevelFull = true;
          else{
            report_fatal_error("Even sharing throughput among ports, the operation\
                               width is larger than throughout*parallel issue");
          }
        }
      }else{
        if(NodeWidthOccupancy >= ExecutionUnitsThroughput[ExecutionResource]* ExecutionUnitsParallelIssue[ExecutionResource])
        LevelFull = true;
      }
    }
    
  }
  return LevelFull;
}


bool
DynamicAnalysis::thereIsAvailableBandwidth(unsigned NextAvailableCycle,
                                           unsigned ExecutionResource,
                                           unsigned NElementsVector,
                                           bool & FoundInFullOccupancyCyclesTree,
                                           bool TargetLevel)
{
#ifndef EFF_TBV
  unsigned  TmpTreeChunk = 0;
#endif
  bool EnoughBandwidth;
  unsigned AccessWidth;
  unsigned IssueCycleGranularity = 0;
  
  Tree < uint64_t > *Node;

  // Reset IssuePorts
  IssuePorts = vector < unsigned >();
  
  if (TargetLevel == true && FoundInFullOccupancyCyclesTree == false) {
    AccessWidth = AccessWidths[ExecutionResource];
    IssueCycleGranularity = getIssueCycleGranularity(ExecutionResource,
                                                     AccessWidth,
                                                     NElementsVector);
    
    // Assume initially that there is enough bandwidth
    EnoughBandwidth = true;
    
    //There is enough bandwidth if:
    // 1. The comp/load/store width fits within the level, or the level is empty.
    if (AvailableCyclesTree[ExecutionResource] != NULL) {
      Node = AvailableCyclesTree[ExecutionResource];
      if (Node != NULL && Node->key >= NextAvailableCycle) {
        AvailableCyclesTree[ExecutionResource] =
        splay((uint64_t)NextAvailableCycle, AvailableCyclesTree[ExecutionResource]);
        Node = AvailableCyclesTree[ExecutionResource];
        if (Node->key == NextAvailableCycle){
          // Get if the level would potentially (the last argument set to true)
          // get full after inserting the current op.
          // (not get full, but actually that instruction width does not fit or
          // there is not wnough issue bandwidth)
          EnoughBandwidth = !getLevelFull(ExecutionResource,
                                          AccessWidth, NElementsVector,
                                          Node->issueOccupancy,
                                          Node->widthOccupancy, true);
        }
      }
      // Else, if Node->key < NextAvailableCycle, that means that the last
      // available  cycle is smaller than the potential cycle, so it is available
      // for sure.
    }
    // Else, if ExecutionUnitsThroughput[ExecutionResource] >= 1, the level is
    // either full (which is not because otherwise this would not being executed),
    // or is available, but still need to check previous and later cycles.
    
    // 2. If IssueCycleGranularity > 1, we have to make sure that there were no
    // instructions executed with the same IssueCycleGranularity in previous
    // cycles. We have to do this because we don't include latency cycles in
    // AvailableCyclesTree.
    
    if (EnoughBandwidth == true) {
      int64_t StartingCycle = 0;
      int64_t tmp = (int64_t) NextAvailableCycle -
      (int64_t)IssueCycleGranularity + (int64_t) 1;
      
      if (tmp < 0)
        StartingCycle = 0;
      else
        StartingCycle = NextAvailableCycle - IssueCycleGranularity + 1;
      
      for (uint64_t i = StartingCycle; i < NextAvailableCycle; i++) {
#ifdef EFF_TBV
        getTreeChunk(i,ExecutionResource);
#else
        TmpTreeChunk = getTreeChunk(i);
#endif
        // If corresponding cycle, for the corresponding resource, is full
#ifdef EFF_TBV
        if (FullOccupancyCyclesTree[ExecutionResource].get_node(i)) {
#else
          if (FullOccupancyCyclesTree[TmpTreeChunk].get_node(i, ExecutionResource)) {
#endif
            FoundInFullOccupancyCyclesTree = true;
            EnoughBandwidth = false;
            break;
#ifdef EFF_TBV
          }
#else
        }
#endif
        else {
          AvailableCyclesTree[ExecutionResource] =
          splay(i, AvailableCyclesTree[ExecutionResource]);
          Node = AvailableCyclesTree[ExecutionResource];
          if (Node != NULL && Node->key == i) {
            if (Node->issuePorts.size() > 0) {
              for (unsigned port = 0; port < Node->issuePorts.size(); port++) {
                IssuePorts.push_back(Node->issuePorts[port]);
                if (ExecutionUnitsParallelIssue[ExecutionResource] != INF &&
                    IssuePorts.size() ==
                    (unsigned)ExecutionUnitsParallelIssue[ExecutionResource]){
                  EnoughBandwidth = false;

                }
              }
            }
          }
        }
      }
    }
    
    // 3. The same as 2 but for next cycles. If there were loads executed on those cycles,
    // there would not be available bandwith for the current load.
    if (EnoughBandwidth == true) {
      for (uint64_t i = NextAvailableCycle + 1;
           i < NextAvailableCycle + IssueCycleGranularity; i++) {
#ifdef EFF_TBV
        getTreeChunk(i, ExecutionResource);
#else
        TmpTreeChunk = getTreeChunk(i);
#endif
        
#ifdef EFF_TBV
        if(FullOccupancyCyclesTree[ExecutionResource].get_node(i)){
#else
          if (FullOccupancyCyclesTree[TmpTreeChunk].get_node(i, ExecutionResource)) {
#endif
            FoundInFullOccupancyCyclesTree = true;
            EnoughBandwidth = false;
            break;
#ifdef EFF_TBV
          }
#else
        }
#endif
        else {
          AvailableCyclesTree[ExecutionResource] =
          splay(i, AvailableCyclesTree[ExecutionResource]);
          Node = AvailableCyclesTree[ExecutionResource];
          if (Node != NULL && Node->key == i) {
            if (Node->issuePorts.size() > 0) {
              for (unsigned port = 0; port < Node->issuePorts.size(); port++) {
                IssuePorts.push_back(Node->issuePorts[port]);
                if (ExecutionUnitsParallelIssue[ExecutionResource] != INF &&
                    IssuePorts.size() ==
                    (unsigned)ExecutionUnitsParallelIssue[ExecutionResource]){
                  EnoughBandwidth = false;
                }
              }
            }
          }
        }
      }
    }
  }
  else {
    EnoughBandwidth = true;
  }
  return EnoughBandwidth;
}


uint64_t
DynamicAnalysis::
  findNextAvailableIssueCycleUntilNotInFullOrEnoughBandwidth(unsigned NextCycle,
                                                             unsigned ExecutionResource,
                                                             bool & FoundInFullOccupancyCyclesTree,
                                                             bool & EnoughBandwidth)
{
  unsigned NextAvailableCycle = NextCycle;
  unsigned OriginalCycle;
  Tree < uint64_t > *Node = AvailableCyclesTree[ExecutionResource];
  Tree < uint64_t > *LastNodeVisited = NULL;
  
  if(ExecutionUnitsThroughput[ExecutionResource] == 0)
    report_fatal_error("Throughput value not valid for resource " +
                     getResourceName(ExecutionResource));
  
  NextAvailableCycle++;
  OriginalCycle = NextAvailableCycle;
  
  // If we loop over the first while because there is not enough bandwidth.
  // Node might be NULL because this loop has already been executed.
  Node = AvailableCyclesTree[ExecutionResource];
  
  while (Node) {
    if (Node->key > NextAvailableCycle) {
      if (NextAvailableCycle == OriginalCycle) {	// first iteration
        NextAvailableCycle = Node->key;
        LastNodeVisited = Node;
      }
      // Search for a smaller one
      Node = Node->left;
    }else if (Node->key < NextAvailableCycle) {
      if (Node->key == OriginalCycle) {
        NextAvailableCycle = OriginalCycle;
        LastNodeVisited = Node;
        break;
      }else if (Node->key > OriginalCycle) {
        //Search for a even smaller one
        NextAvailableCycle = Node->key;
        LastNodeVisited = Node;
        // Search for a smaller one
        Node = Node->left;
      } else {//Node->key < OriginalCycle
        // Search for a larger one, but do not store last node visited...
        Node = Node->right;
      }
    } else {			//Node->key = NextAvailableCycle
      NextAvailableCycle = Node->key;
      LastNodeVisited = Node;
      break;
    }
  }
  
  //LastNodeVisited contains the next available cycle. But we still need to check
  //that it is available for lower and upper levels.
  if (LastNodeVisited != NULL && LastNodeVisited->key >= OriginalCycle)
    NextAvailableCycle = LastNodeVisited->key;

  FoundInFullOccupancyCyclesTree = false;
  EnoughBandwidth = false;
  return NextAvailableCycle;
}


// Find next available issue cycle depending on resource availability.
// Returns the DAG level occupancy after the insertion
unsigned
DynamicAnalysis::findNextAvailableIssueCycle(unsigned OriginalCycle,
                                             unsigned ExecutionResource,
                                             uint8_t NElementsVector,
                                             bool TargetLevel)
{
  uint64_t NextAvailableCycle = OriginalCycle;
  bool FoundInFullOccupancyCyclesTree = true;
  bool EnoughBandwidth = false;
  unsigned TreeChunk = 0;
  
  if(ExecutionUnitsThroughput[ExecutionResource]==0)
    report_fatal_error("Throughput value not valid for resource "+
                     getResourceName(ExecutionResource) );
  
  // Get the node, if any, corresponding to this issue cycle.
#ifdef EFF_TBV
  getTreeChunk(NextAvailableCycle, ExecutionResource);
#else
  TreeChunk = getTreeChunk(NextAvailableCycle);
#endif
  
  // If full is null, then it is available for sure -> WRONG! It might happen
  // that FULL is NULL because a new chunk was created.
  // If it is not NULL and there is something scheduled in this cycle..
  // (we don't include the condition FullOccupancyNode->BitVector[ExecutionResource]==1
  // here because it could happen that it cannot be executed because of throughput<1
  // and something executed in earlier or later cycles.
#ifdef EFF_TBV
  if (!FullOccupancyCyclesTree[ExecutionResource].empty()) {
#else
    if (!FullOccupancyCyclesTree[TreeChunk].empty()) {
#endif
      // Check if it is in full, but first make sure full is not NULL (it could
      // happen it is NULL after changing the NextAvailableCycle).
#ifdef EFF_TBV
      if(FullOccupancyCyclesTree[ExecutionResource].get_node(NextAvailableCycle))
        FoundInFullOccupancyCyclesTree = true;
#else
      if (FullOccupancyCyclesTree[TreeChunk].get_node(NextAvailableCycle,
                                                      ExecutionResource))
        FoundInFullOccupancyCyclesTree = true;
#endif
      else
        FoundInFullOccupancyCyclesTree = false;

      // If it is not in full, it is available. But we have to make sure that
      // there is enough bandwidth (to avoid having large trees, we don't include
      // the latency cycles, so we have to make sure we don't issue in in latency cycles)
      if (ExecutionResource < NExecutionUnits) {
        if (FoundInFullOccupancyCyclesTree == true) {
          NextAvailableCycle =
          findNextAvailableIssueCycleUntilNotInFullOrEnoughBandwidth(NextAvailableCycle,
                                                                     ExecutionResource,
                                                                     FoundInFullOccupancyCyclesTree,
                                                                     EnoughBandwidth);
        }
        EnoughBandwidth =
        thereIsAvailableBandwidth(NextAvailableCycle, ExecutionResource,
                                  NElementsVector, FoundInFullOccupancyCyclesTree,
                                  TargetLevel);
        
        if (EnoughBandwidth == false) {
          while (EnoughBandwidth == false) {
            NextAvailableCycle =
            findNextAvailableIssueCycleUntilNotInFullOrEnoughBandwidth(NextAvailableCycle,
                                                                       ExecutionResource,
                                                                       FoundInFullOccupancyCyclesTree,
                                                                       EnoughBandwidth);
            EnoughBandwidth =
            thereIsAvailableBandwidth(NextAvailableCycle, ExecutionResource,
                                      NElementsVector,
                                      FoundInFullOccupancyCyclesTree,
                                      TargetLevel);
            
#ifdef EFF_TBV
            getTreeChunk(NextAvailableCycle, ExecutionResource);
#else
            TreeChunk = getTreeChunk(NextAvailableCycle);
#endif
          }
        }
      }else {
        if (FoundInFullOccupancyCyclesTree == true) {
          while (FoundInFullOccupancyCyclesTree) {
            //Check if it is in full
#ifdef EFF_TBV
            if(FullOccupancyCyclesTree[ExecutionResource].get_node(NextAvailableCycle)){
#else
              if (FullOccupancyCyclesTree[TreeChunk].get_node(NextAvailableCycle, ExecutionResource)) {
#endif
                // Try next cycle
                NextAvailableCycle++;
#ifdef EFF_TBV
                getTreeChunk(NextAvailableCycle, ExecutionResource);
#else
                TreeChunk = getTreeChunk(NextAvailableCycle);
#endif
                FoundInFullOccupancyCyclesTree = true;
#ifdef EFF_TBV
              }
#else
            }
#endif
            else {
              FoundInFullOccupancyCyclesTree = false;
            }
          }
        }
      }
#ifdef EFF_TBV
    }
#else
  }
#endif
  else{ // If FullOccupancyCyclesTree is empty
    if (TreeChunk != 0) {
      // Full is NULL, but check that TreeChunk is not zero. Otherwise,
      // Full is not really NULL
      if (ExecutionResource < NExecutionUnits) {
        FoundInFullOccupancyCyclesTree = false;
        EnoughBandwidth =
        thereIsAvailableBandwidth(NextAvailableCycle, ExecutionResource,
                                  NElementsVector, FoundInFullOccupancyCyclesTree,
                                  TargetLevel);
        
        if (FoundInFullOccupancyCyclesTree == true || EnoughBandwidth == false) {
          NextAvailableCycle =
          findNextAvailableIssueCycleUntilNotInFullOrEnoughBandwidth(NextAvailableCycle,
                                                                     ExecutionResource,
                                                                     FoundInFullOccupancyCyclesTree,
                                                                     EnoughBandwidth);
        }
      }
    }
  }
  return NextAvailableCycle;
}


// Find next available issue cycle depending on resource availability
bool
DynamicAnalysis::insertNextAvailableIssueCycle(uint64_t NextAvailableCycle,
                                               unsigned ExecutionResource,
                                               unsigned NElementsVector,
                                               int IssuePort, bool isPrefetch)
{
  Tree < uint64_t > *Node = AvailableCyclesTree[ExecutionResource];
  unsigned NodeIssueOccupancy = 0;
  unsigned NodeWidthOccupancy = 0;
  unsigned NodeOccupancyPrefetch = 0;
  bool LevelGotFull = false;
  
#ifdef EFF_TBV
  getTreeChunk(NextAvailableCycle, ExecutionResource);
#else
  unsigned TreeChunk = getTreeChunk(NextAvailableCycle);
#endif
  
  // Update Instruction count
  if (InstructionsCountExtended[ExecutionResource] == 0)
    FirstIssue[ExecutionResource] = true;
  
  if (NElementsVector > 1) {
    InstructionsCountExtended[ExecutionResource] =
    InstructionsCountExtended[ExecutionResource] + NElementsVector;
    VectorInstructionsCountExtended[ExecutionResource]++;
  }else {
    InstructionsCountExtended[ExecutionResource]++;
    ScalarInstructionsCountExtended[ExecutionResource]++;
  }
  
  unsigned AccessWidth = AccessWidths[ExecutionResource];

  if (FirstIssue[ExecutionResource] == true) {
    FirstNonEmptyLevel[ExecutionResource] = NextAvailableCycle;
    FirstIssue[ExecutionResource] = false;
  }else {
    FirstNonEmptyLevel[ExecutionResource] = min (FirstNonEmptyLevel[ExecutionResource],
                                                 NextAvailableCycle);
  }
  
  InstructionsLastIssueCycle[ExecutionResource] =
 max(InstructionsLastIssueCycle[ExecutionResource], NextAvailableCycle);
  
  AvailableCyclesTree[ExecutionResource] =
  insert_node(NextAvailableCycle, AvailableCyclesTree[ExecutionResource]);
#ifdef SOURCE_CODE_ANALYSIS
  AvailableCyclesTree[ExecutionResource]->
  SourceCodeLinesOperationPair.push_back(std::make_pair (SourceCodeLine, ExecutionResource));
#endif
  
  if (IssuePort >= PORT_0)
    AvailableCyclesTree[ExecutionResource]->issuePorts.push_back(IssuePort);
  Node = AvailableCyclesTree[ExecutionResource];

  if (isPrefetch)
    Node->occupancyPrefetch++;
  else
    Node->issueOccupancy++;
  
  Node->widthOccupancy += getNodeWidthOccupancy(ExecutionResource, AccessWidth,
                                                NElementsVector);
  /* Copy these values because later on the Node is not the same anymore */
  NodeIssueOccupancy = Node->issueOccupancy;
  NodeWidthOccupancy = Node->widthOccupancy;
  NodeOccupancyPrefetch = Node->occupancyPrefetch;
  MaxOccupancy[ExecutionResource] =
  max(MaxOccupancy[ExecutionResource], NodeIssueOccupancy + NodeOccupancyPrefetch);

  // If ExecutionUnitsThroughput and ExecutionUnitsParallelIssue are INF,
  // the level never gets full.
  // Otherwise, a level gets full then:
  // 1. The number of parallel operations issued is equal to
  // ExecutionUnitParallelIssue, although the full bw is not utilized
  // 2. When width occupancy equals to Throughput*ParallelIssue
  
  LevelGotFull = getLevelFull(ExecutionResource, AccessWidth, NElementsVector,
                              NodeIssueOccupancy, NodeWidthOccupancy);
  
  if (LevelGotFull) {
    LevelGotFull = true;
    // Check whether next cycle is in full. because if it is, it should not be
    // inserted into AvailableCyclesTree.
    // Next cycle is not NexAvailableCycle+1, is NextAvailableCycle + 1/Throughput
    // Here is where the distinction betweeen execution resource and instruction
    // type is important.
    unsigned NextCycle  = getIssueCycleGranularity(ExecutionResource,
                                                   AccessWidth,
                                                   NElementsVector);
    
    AvailableCyclesTree[ExecutionResource] =
    delete_node(NextAvailableCycle, AvailableCyclesTree[ExecutionResource]);
    
    // Insert node in FullOccupancy
#ifdef EFF_TBV
    FullOccupancyCyclesTree[ExecutionResource].insert_node( NextAvailableCycle);
#else
    FullOccupancyCyclesTree[TreeChunk].insert_node(NextAvailableCycle, ExecutionResource);
#endif
    
#ifdef SOURCE_CODE_ANALYSIS
    //The corresponding node in AvailableCyclesTress has already been removed
    FullOccupancyCyclesTree[TreeChunk].insert_source_code_line (NextAvailableCycle,
                                                                SourceCodeLine,
                                                                ExecutionResource);
#endif
    
#ifdef EFF_TBV
    getTreeChunk(NextAvailableCycle + NextCycle, ExecutionResource);
#else
    TreeChunk = getTreeChunk(NextAvailableCycle + NextCycle);
#endif
    
#ifdef EFF_TBV
    if (!FullOccupancyCyclesTree[ExecutionResource].get_node((NextAvailableCycle
                                                              + NextCycle))) {
#else
      if ( !FullOccupancyCyclesTree[TreeChunk].get_node((NextAvailableCycle + NextCycle),
                                                        ExecutionResource)) {
#endif
        AvailableCyclesTree[ExecutionResource] =
        insert_node(NextAvailableCycle + NextCycle,
                     AvailableCyclesTree[ExecutionResource]);
        // In this case, although we are inserting a node into AvailableCycles,
        // we don't insert the source code line associated to the cycle because
        // it does not mean that an instruction has actually been
        // scheduled in NextAvailableCycle+NextCycle. In this case it just means
        // that this is the next available cycle. Actually, IssueOccupacy of
        // this new level should be zero.
            InstructionsLastIssueCycle[ExecutionResource] =
       max(InstructionsLastIssueCycle[ExecutionResource], NextAvailableCycle + NextCycle);
#ifdef EFF_TBV
      }
#else
    }
#endif
  }
  return LevelGotFull;
}


void DynamicAnalysis::increaseInstructionFetchCycle(bool EmptyBuffers)
{
#ifndef EFF_TBV
  unsigned TreeChunk = 0;
#endif
  bool OOOBufferFull = false;
  
  uint64_t OriginalInstructionFetchCycle = InstructionFetchCycle;
#ifdef DEBUG_GENERIC
  DEBUG (dbgs() << "_____________________ InstructionFetchCycle " <<
         InstructionFetchCycle << "_____________________\n");
#endif
  
  // Remove from Reservation Stations elements issued at fetch cycle
  if (ReservationStationSize > 0)
    removeFromReservationStation(InstructionFetchCycle);
  
  if (ReorderBufferSize > 0)
    removeFromReorderBuffer(InstructionFetchCycle);
  
  //Remove from Load, Store and Fill Line Buffers elements completed at issue cycle
  if(SmallBuffers){
    removeFromLoadBuffer(InstructionFetchCycle);
    removeFromDispatchToLoadBufferQueue(InstructionFetchCycle);
  }else
    removeFromLoadBufferTree(InstructionFetchCycle);
  
  removeFromStoreBuffer(InstructionFetchCycle);
  removeFromLineFillBuffer(InstructionFetchCycle);
  
  removeFromDispatchToStoreBufferQueue(InstructionFetchCycle);
  removeFromDispatchToLineFillBufferQueue(InstructionFetchCycle);
  // Insert into LB, SB and LFB the instructions from the dispatch queue.
  if(SmallBuffers)
    dispatchToLoadBuffer(InstructionFetchCycle);
  else
    dispatchToLoadBufferTree (InstructionFetchCycle);
  
  dispatchToStoreBuffer (InstructionFetchCycle);
  dispatchToLineFillBuffer (InstructionFetchCycle);
  
  // If even still removing element that should have been issued,
  // Reservation station is full
  if(ReservationStationIssueCycles.size() == (unsigned) ReservationStationSize
     && ReservationStationSize != 0){
    // Advance InstructionFetchCyle until min issue cycle
    OOOBufferFull = true;
    uint64_t CurrentInstructionFetchCycle = InstructionFetchCycle;
    InstructionFetchCycle = getMinIssueCycleReservationStation ();
    
    if (InstructionFetchCycle > CurrentInstructionFetchCycle + 1)
      FirstNonEmptyLevel[RS_STALL] =
    (FirstNonEmptyLevel[RS_STALL] == 0) ? CurrentInstructionFetchCycle + 1 :
    FirstNonEmptyLevel[RS_STALL];
    
    if(InstructionFetchCycle == CurrentInstructionFetchCycle)
      report_fatal_error("CHECK InstructionFetchCycle == \
                         CurrentInstructionFetchCycle");
    
    for (uint64_t i = CurrentInstructionFetchCycle+1; i < InstructionFetchCycle; i++) {
#ifdef EFF_TBV
      getTreeChunk(i,RS_STALL );
      FullOccupancyCyclesTree[RS_STALL].insert_node( i);
#else
      TreeChunk = getTreeChunk(i);
      FullOccupancyCyclesTree[TreeChunk].insert_node(i, RS_STALL);
#endif
      
#ifdef SOURCE_CODE_ANALYSIS
      FullOccupancyCyclesTree[TreeChunk].insert_source_code_line(i,
                                                                 SourceCodeLine,
                                                                 RS_STALL);
#endif
      InstructionsCountExtended[RS_STALL]++;
      InstructionsLastIssueCycle[RS_STALL] = i;
    }
  }
  
  if (ReorderBufferCompletionCycles.size() == ReorderBufferSize &&
      ReorderBufferSize > 0) {
    //Advance InstructionFetchCycle to the head of the buffer
    OOOBufferFull = true;
    uint64_t CurrentInstructionFetchCycle = InstructionFetchCycle;
    InstructionFetchCycle =max(InstructionFetchCycle,
                               ReorderBufferCompletionCycles.front ());
    if (InstructionFetchCycle > CurrentInstructionFetchCycle + 1) {
      FirstNonEmptyLevel[ROB_STALL] =
      (FirstNonEmptyLevel[ROB_STALL] == 0) ? CurrentInstructionFetchCycle + 1 :
      FirstNonEmptyLevel[ROB_STALL];
    }
    
    for (uint64_t i = CurrentInstructionFetchCycle+1 ;
         i < InstructionFetchCycle; i++) {
      // Get the node, if any, corresponding to this issue cycle.
#ifdef EFF_TBV
      getTreeChunk(i,ROB_STALL);
      FullOccupancyCyclesTree[ROB_STALL].insert_node(i);
#else
      TreeChunk = getTreeChunk(i);
      FullOccupancyCyclesTree[TreeChunk].insert_node(i, ROB_STALL);
#endif
      
#ifdef SOURCE_CODE_ANALYSIS
      FullOccupancyCyclesTree[TreeChunk].insert_source_code_line (i,
                                                                  SourceCodeLine,
                                                                  ROB_STALL);
#endif
      InstructionsCountExtended[ROB_STALL]++;
      InstructionsLastIssueCycle[ROB_STALL] = i;
    }
  }
  
  if (OOOBufferFull == true) {
    // Remove from Reservation Stations elements issued at fetch cycle
    removeFromReservationStation(InstructionFetchCycle);
    removeFromReorderBuffer(InstructionFetchCycle);
    //Remove from Load, Store and Fill Line Buffers elements completed at issue cycle
    if(SmallBuffers){
      removeFromLoadBuffer(InstructionFetchCycle);
      removeFromDispatchToLoadBufferQueue(InstructionFetchCycle);
    }else
      removeFromLoadBufferTree(InstructionFetchCycle);
    
    removeFromStoreBuffer(InstructionFetchCycle);
    removeFromLineFillBuffer(InstructionFetchCycle);
    
    removeFromDispatchToStoreBufferQueue(InstructionFetchCycle);
    removeFromDispatchToLineFillBufferQueue(InstructionFetchCycle);
    // Insert into LB, SB and LFB the instructions from the dispatch queue.
    if(SmallBuffers)
      dispatchToLoadBuffer(InstructionFetchCycle);
    else
      dispatchToLoadBufferTree (InstructionFetchCycle);
    
    dispatchToStoreBuffer (InstructionFetchCycle);
    dispatchToLineFillBuffer (InstructionFetchCycle);
  }
  // When we are at this point, either we have removed from RS or ROB the
  // instructions issued at this cycle, and they left some empty slots
  // so that the buffers are not full anymore, or we have advanced
  // InstructionFetchCycle to the cycle at which any of the buffers
  // gets empty. In this case, we also have to set Remaining instructions
  // to fetch to Fetchbandwidth, because we have modified fetch cycle
  // and we start fetching again.
  if (OOOBufferFull == true)
    RemainingInstructionsFetch = InstructionFetchBandwidth;
  
  if (EmptyBuffers == true)
    InstructionFetchCycle++;
  else{
    if (RemainingInstructionsFetch == 0 && InstructionFetchBandwidth != INF) {
      InstructionFetchCycle++;
      RemainingInstructionsFetch = InstructionFetchBandwidth;
    }
  }

  if (OriginalInstructionFetchCycle != InstructionFetchCycle) {
    uint64_t CyclesIncrease =
    (InstructionFetchCycle-OriginalInstructionFetchCycle);
    
    BuffersOccupancy[RS_STALL - RS_STALL] +=
    (ReservationStationIssueCycles.size() * CyclesIncrease);
    
    BuffersOccupancy[ROB_STALL - RS_STALL] +=
    (ReorderBufferCompletionCycles.size()* CyclesIncrease);
    
    if(SmallBuffers){
      BuffersOccupancy[LB_STALL-RS_STALL] +=
      (LoadBufferCompletionCycles.size()*CyclesIncrease);
      if(LoadBufferCompletionCycles.size() > LoadBufferSize)
        report_fatal_error("Buffer overflow");
      
    }else{
      BuffersOccupancy[LB_STALL - RS_STALL] +=
      (node_size(LoadBufferCompletionCyclesTree)*CyclesIncrease);
      if( node_size(LoadBufferCompletionCyclesTree) > LoadBufferSize){
        report_fatal_error("Buffer overflow");
      }
    }
    BuffersOccupancy[SB_STALL - RS_STALL] +=
    (StoreBufferCompletionCycles.size()*CyclesIncrease);
    BuffersOccupancy[LFB_STALL - RS_STALL] +=
    (LineFillBufferCompletionCycles.size()*CyclesIncrease);
    
    uint64_t PrevInstructionFetchCycle = InstructionFetchCycle - 1;
    if (DispatchToLineFillBufferQueue.empty() == false) {
      if (InstructionsCountExtended[LFB_STALL] == 0)
      FirstIssue[LFB_STALL] = true;
      if (FirstIssue[LFB_STALL] == true) {
        FirstNonEmptyLevel[LFB_STALL] = PrevInstructionFetchCycle;
        FirstIssue[LFB_STALL] = false;
      }
      InstructionsLastIssueCycle[LFB_STALL] = PrevInstructionFetchCycle;
      
#ifdef EFF_TBV
      getTreeChunk(PrevInstructionFetchCycle, LFB_STALL);
      FullOccupancyCyclesTree[LFB_STALL].insert_node(PrevInstructionFetchCycle);
#else
      TreeChunk = getTreeChunk(PrevInstructionFetchCycle);
      FullOccupancyCyclesTree[TreeChunk].insert_node(PrevInstructionFetchCycle,
                                                     LFB_STALL);
#endif
      // We do it when an instruction is inserted. Otherwise, SourceCodeLine
      // has the value of the last instruction analyzed from the instruction
      // fetch window, which might not be the instruction that was stalled.
      InstructionsCountExtended[LFB_STALL]++;
    }
    
    bool BufferNonEmpty=false;
    if(SmallBuffers){
      if (DispatchToLoadBufferQueue.empty() == false)
        BufferNonEmpty = true;
    }else{
      if (node_size(DispatchToLoadBufferQueueTree) != 0)
        BufferNonEmpty = true;
    }
    if(BufferNonEmpty){
      if (InstructionsCountExtended[LB_STALL] == 0)
        FirstIssue[LB_STALL] = true;
      if (FirstIssue[LB_STALL] == true) {
        FirstNonEmptyLevel[LB_STALL] = PrevInstructionFetchCycle;
        FirstIssue[LB_STALL] = false;
      }
      InstructionsLastIssueCycle[LB_STALL] = PrevInstructionFetchCycle;
#ifdef EFF_TBV
      getTreeChunk(PrevInstructionFetchCycle,LB_STALL);
      FullOccupancyCyclesTree[LB_STALL].insert_node(PrevInstructionFetchCycle);
#else
      TreeChunk = getTreeChunk(PrevInstructionFetchCycle);
      FullOccupancyCyclesTree[TreeChunk].insert_node(PrevInstructionFetchCycle,
                                                     LB_STALL);
#endif
      InstructionsCountExtended[LB_STALL]++;
    }
    if (DispatchToStoreBufferQueue.empty() == false) {
      if (InstructionsCountExtended[SB_STALL] == 0)
        FirstIssue[SB_STALL] = true;
      if (FirstIssue[SB_STALL] == true) {
        FirstNonEmptyLevel[SB_STALL] = PrevInstructionFetchCycle;
        FirstIssue[SB_STALL] = false;
      }
      InstructionsLastIssueCycle[SB_STALL] = PrevInstructionFetchCycle;
#ifdef EFF_TBV
      getTreeChunk(PrevInstructionFetchCycle, SB_STALL);
      FullOccupancyCyclesTree[SB_STALL].insert_node(PrevInstructionFetchCycle);
#else
      TreeChunk = getTreeChunk(PrevInstructionFetchCycle);
      FullOccupancyCyclesTree[TreeChunk].insert_node(PrevInstructionFetchCycle,
                                                     SB_STALL);
#endif
      InstructionsCountExtended[SB_STALL]++;
    }
  }
}


//===----------------------------------------------------------------------===//
//                Routines for Analysis of Data Reuse
//===----------------------------------------------------------------------===//
#ifdef REUSE_STACK
int DynamicAnalysis::registerStackReuseDistance(uint64_t address, bool WarmRun)
{
  int Distance = -1;
  std::deque<uint64_t>::iterator itPosition;
  unsigned PositionCounter = 0;
  
  if(RegisterFileSize == 0)
    return Distance;
  
  for (std::deque<uint64_t>::iterator it = ReuseStack.end()-1;
       it>= ReuseStack.begin(); --it){
    if (*it == address){
      Distance = PositionCounter;
      itPosition = it;
      break;
    }
    PositionCounter++;
  }
  // If element was not in ReuseStack,
  // - If size of ReuseStack is equal to RegisterFile, pop front and push back the new element
  // - Else, simply push_back
  if (Distance < 0){
    insertRegisterStack(address);
  }else{
    // Remove and put at the top (back) of the stack
    ReuseStack.erase(itPosition);
    ReuseStack.push_back(address);
  }

  return Distance;
}


void DynamicAnalysis::insertRegisterStack(int64_t address)
  {
  if(ReuseStack.size() == RegisterFileSize){
    if(!WarmCache)
    NRegisterSpillsStores++;
    ReuseStack.pop_front();
    ReuseStack.push_back(address);
  }else{
    ReuseStack.push_back(address);
  }
}
#endif


int
DynamicAnalysis::ReuseDistance(uint64_t Last, uint64_t Current, uint64_t address,
                                bool FromPrefetchReuseTree)
{
  int Distance = -1;
  
  int PrefetchReuseTreeDistance = 0;
  if (!(L1CacheSize == 0 && L2CacheSize == 0 && LLCCacheSize == 0) ) {
    // Otherwise, does not matter the distance, it is mem access
    int ReuseTreeDistance = reuseTreeSearchDelete (Last, address, false);
    if (SpatialPrefetcher == true) {
      bool IsInPrefetchReuseTree = false;
      // To know whether the data item was in PrefetchReuseTree or not,
      // we check whether the element has been removed from the tree
      // (i.e., the size of the tree).
      int PrefetchReuseTreeSizeBefore = PrefetchReuseTreeSize;
      PrefetchReuseTreeDistance = reuseTreeSearchDelete (Last, address, true);
      int PrefetchReuseTreeSizeAfter = PrefetchReuseTreeSize;
      
      if (PrefetchReuseTreeSizeAfter < PrefetchReuseTreeSizeBefore) {
        IsInPrefetchReuseTree = true;
      }
      if (IsInPrefetchReuseTree == false) {
        if (ReuseTreeDistance > 0 && (uint64_t) ReuseTreeDistance >= PrefetchLevel) {
          if (PrefetchReuseTreeDistance >= 0) {
            Distance = ReuseTreeDistance + PrefetchReuseTreeDistance;
          }else
          Distance = ReuseTreeDistance;	// In case PrefetchReuseTreeDistance returns -1
        }else {
          // If is not a prefetched node and ReuseTreeDistance = -1 or is
          // not a prefetched node and ReuseDistance < L2CacheSize
          Distance = ReuseTreeDistance;
        }
      }else {
        // If the data item is a prefetched data item (found in PrefetchReuseTree)
        Distance = PrefetchLevel;
        if (ReuseTreeDistance >= 0) {
          Distance += ReuseTreeDistance;
        }else {
          // The data item has only been prefetched. In that case, the distance is
          // the size of L2 plus the data items prefetched since the last access
        }
        if (PrefetchReuseTreeDistance >= 0)
          Distance += PrefetchReuseTreeDistance;
        else{
          report_fatal_error
          ("The data item is prefetched, PrefetchReuseTreeDistance >=0, but data\
           item does not seem to be in PrefetchReuseTree");
        }
      }
      // In the rest of the cases (ReuseTreeDistance > 0 && ReuseTreeDistance)
    }
    else{
      Distance = ReuseTreeDistance;
    }
#ifdef ROUND_REUSE_DISTANCE
    if (Distance >= 0)
      Distance = roundNextPowerOfTwo (Distance);
#endif
    // Get a pointer to the resulting tree
    if (FromPrefetchReuseTree == false) {
      ReuseTree = insert_node(Current, ReuseTree, address);
    }else {
      PrefetchReuseTree = insert_node(Current, PrefetchReuseTree, address);
      PrefetchReuseTreeSize++;
    }
  }else
    ReuseTree = insert_node(address, ReuseTree, address);

  return Distance;
}


// Return the distance of the closest node with key <= Current, i.e., all the
// nodes that have been prefetched between Last and Current.
// From the paper "Program Locality Analysis Using Reuse Distance", by Y. Zhong,
// X. Sheng and C. DIng, 2009
int
DynamicAnalysis::reuseTreeSearchDelete(uint64_t Original, uint64_t address,
                                        bool FromPrefetchReuseTree)
{
  Tree < uint64_t > *Node = NULL;
  int Distance = 0;

  if (FromPrefetchReuseTree == false)
    Node = ReuseTree;
  else
    Node = PrefetchReuseTree;
  
  //Once we find it, calculate the distance without deleting the node.
  if (Original == 0 || Node == NULL) {	// Did not find any node smaller
    Distance = -1;
  }else {
    while (true) {
      // This is the mechanism used in the original algorithm to delete the host
      // node,  decrementing the last_record attribute of the host node, and
      // Node->size = Node->size-1;
      if (Original < Node->key) {
        if (Node->right != NULL)
        Distance = Distance + Node->right->size;
        if (Node->left == NULL)
        break;
        
        Distance = Distance + 1 /*Node->last_record */ ;
        Node = Node->left;
      }else {
        if (Original > Node->key) {
          if (Node->right == NULL)
          break;
          Node = Node->right;
        }else {			// Last = Node->key, i.e., Node is the host node
          if (Node->right != NULL)
          Distance = Distance + Node->right->size;
          //increase by one so that we can calculate directly the hit rate
          // for a cache size multiple of powers of two.
          Distance = Distance + 1;
          if (Node->address == address && FromPrefetchReuseTree == false)
            ReuseTree = delete_node(Original, ReuseTree);
          else {
            if (Node->address == address && FromPrefetchReuseTree == true) {
              PrefetchReuseTree = delete_node(Original, PrefetchReuseTree);
              PrefetchReuseTreeSize--;
            }
          }
          break;
        }
      }
    }
  }
  return Distance;
}

  
void
DynamicAnalysis::updateRegisterReuseDistanceDistribution(int Distance)
{
  map < int, int >::iterator ReuseDistanceMapIt;
  
  ReuseDistanceMapIt = RegisterReuseDistanceDistribution.find (Distance+1);
  if (ReuseDistanceMapIt != RegisterReuseDistanceDistribution.end())
    ReuseDistanceMapIt->second = ReuseDistanceMapIt->second + 1;
  else
    RegisterReuseDistanceDistribution[Distance+1] = 1;	// First usage
}


void
DynamicAnalysis::updateReuseDistanceDistribution(int Distance,
                                                  uint64_t InstructionIssueCycle)
{
  map < int, int >::iterator ReuseDistanceMapIt;
  map < int, map < uint64_t, uint > >::iterator ReuseDistanceExtendedMapIt;
  map < uint64_t, uint >::iterator AuxMapIt;
  
#ifdef NORMAL_REUSE_DISTRIBUTION
  ReuseDistanceMapIt = ReuseDistanceDistribution.find (Distance);
  if (ReuseDistanceMapIt != ReuseDistanceDistribution.end())
    ReuseDistanceMapIt->second = ReuseDistanceMapIt->second + 1;
  else
    ReuseDistanceDistribution[Distance] = 1;	// First usage
#else
  ReuseDistanceExtendedMapIt = ReuseDistanceDistributionExtended.find (Distance);
  if (ReuseDistanceExtendedMapIt != ReuseDistanceDistributionExtended.end()) {
    AuxMapIt = (ReuseDistanceExtendedMapIt->second).find (InstructionIssueCycle);
    if (AuxMapIt != (ReuseDistanceExtendedMapIt->second).end())
      AuxMapIt->second = AuxMapIt->second + 1;
    else
    (ReuseDistanceExtendedMapIt->second)[InstructionIssueCycle] = 1;
  }
  else
    ReuseDistanceDistributionExtended[Distance][InstructionIssueCycle] = 1;
#endif
}


//===----------------------------------------------------------------------===//
//                  Some auxiliary routines
//===----------------------------------------------------------------------===//
// compute the next highest power of 2 of 32-bit v.
// Routine from Bit Twiddling Hacks, University of Standford.
unsigned int
DynamicAnalysis::roundNextPowerOfTwo(unsigned int v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  
  return v;
}


uint64_t
DynamicAnalysis::roundNextMultiple(uint64_t num, int factor)
{
  // This works because factor is always going to be a power of 2
  return (num + factor - 1) & ~(factor - 1);
}


//===----------------------------------------------------------------------===//
//        Routine for analysis of ILP
//===----------------------------------------------------------------------===//

void
DynamicAnalysis::printILPDistribuion(unsigned span)
{
  for(uint64_t cycle = 0; cycle < ParallelismDistribution.size(); cycle++)
    dbgs() << ParallelismDistribution[cycle][0] << " " <<
  ParallelismDistribution[cycle][1] << "\n";
}


//===---------------------------------------------------------------------===//
//               Routines for span analysis
//===---------------------------------------------------------------------===//

uint64_t
DynamicAnalysis::getLastIssueCycle(unsigned ExecutionResource, bool WithPrefetch)
{
  Tree < uint64_t > *NodeAvailable = NULL;
  bool isPrefetchType = false;
  unsigned IssueCycleGranularity = IssueCycleGranularities[ExecutionResource];
  uint64_t LastCycle = InstructionsLastIssueCycle[ExecutionResource];
  
  if (ExecutionResource <= NExecutionUnits) {
    AvailableCyclesTree[ExecutionResource] =
    splay(LastCycle, AvailableCyclesTree[ExecutionResource]);
    NodeAvailable = AvailableCyclesTree[ExecutionResource];
    
    if (isPrefetchType) {
#ifdef EFF_TBV
      if ((NodeAvailable != NULL && NodeAvailable->key == LastCycle &&
           NodeAvailable->occupancyPrefetch == 0) ||
          (FullOccupancyCyclesTree[ExecutionResource].get_node_nb (LastCycle))) {
#else
        int TreeChunk = LastCycle / SplitTreeRange;
        if ((NodeAvailable != NULL && NodeAvailable->key == LastCycle &&
             NodeAvailable->occupancyPrefetch == 0) ||
            (FullOccupancyCyclesTree[TreeChunk].get_node_nb(LastCycle,
                                                            ExecutionResource))){
#endif
          LastCycle = LastCycle - IssueCycleGranularity;

#ifdef EFF_TBV
        }
#else
      }
#endif
    }else {
      if (NodeAvailable != NULL && NodeAvailable->key == LastCycle &&
          NodeAvailable->issueOccupancy == 0) {
        LastCycle = LastCycle - IssueCycleGranularity;
      }
    }
  }
  return LastCycle;
}


unsigned
DynamicAnalysis::calculateIssueSpanFinal(vector < int >&ResourcesVector,
                                          bool onlyScalar)
{
  unsigned Span = 0;
  unsigned MaxLatency = 0;
  uint64_t First = 0;
  bool EmptyLevel = true;
  int NResources = ResourcesVector.size();
  uint64_t LastCycle = 0;
  uint64_t ResourceLastCycle = 0;
  unsigned MaxLatencyLevel = 0;
  unsigned ResourceType = 0;
  unsigned AccessWidth = 0;
  unsigned TmpLatency = 0;
  
  for (unsigned i = 0; i < NTotalResources; i++) {
    if (LastIssueCycleVector[i] > LastCycle)
      LastCycle = LastIssueCycleVector[i];
  }
  LastCycle += MaxLatencyResources;		// to be safe
  
  // Prepare a cache of values
  if (NResources == 1) {
    CISFCache[ResourcesVector[0]].resize(LastCycle, false);
  }else {
    dynamic_bitset <> BitMesh (LastCycle);
    for (int j = 0; j < NResources; j++) {
      // Should probably recurse and calculate this value just in case
      if (CISFCache[ResourcesVector[j]].size() == 0) {
        vector < int >tv;
        tv.push_back(ResourcesVector[j]);
        calculateIssueSpanFinal (tv);
      }
      BitMesh |= CISFCache[ResourcesVector[j]];
    }
    return BitMesh.count ();
  }
  
  //Determine first non-empty level and LastCycle
  for (int j = 0; j < NResources; j++) {
    ResourceType = ResourcesVector[j];
    if (InstructionsCountExtended[ResourceType] > 0) {
      AccessWidth = AccessWidths[ResourceType];
      if (ExecutionUnitsThroughput[ResourceType] == INF)
      TmpLatency = 1;
      else{
        if(onlyScalar)
        TmpLatency = getIssueCycleGranularity(ResourceType, AccessWidth, 1);
        else
        TmpLatency = getIssueCycleGranularity(ResourceType, AccessWidth,
                                              getNElementsAccess(ResourceType,
                                                                 AccessWidth,
                                                                 VectorWidth));
      }
      // This will be only executed the first time of a non-empty level
      if (EmptyLevel == true) {
        EmptyLevel = false;
        First = FirstNonEmptyLevel[ResourceType];
        MaxLatency = TmpLatency;
      }else {
        if (First == FirstNonEmptyLevel[ResourceType])
        MaxLatency =max(MaxLatency, TmpLatency);
        else {
          First = min (First, FirstNonEmptyLevel[ResourceType]);
          if (First == FirstNonEmptyLevel[ResourceType])
          MaxLatency = TmpLatency;
        }
      }
      ResourceLastCycle = LastIssueCycleVector[ResourceType];
      LastCycle =max(LastCycle, ResourceLastCycle);
    }
  }
  
  unsigned DominantLevel = First;
  if (NResources == 1 && MaxLatency > 0) {
    //unsigned nBits = 0;
    for (unsigned q = 0; q < MaxLatency; q++)
      CISFCache[ResourcesVector[0]][First + q] = 1;
  }
  
  if (EmptyLevel == false) {
    Span += MaxLatency;
    //Start from next level to first non-empty level
    for (unsigned i = First + 1; i <= LastCycle; i++) {
      //Determine MaxLatency of Level
      MaxLatencyLevel = 0;
      for (int j = 0; j < NResources; j++) {
        ResourceType = ResourcesVector[j];
        if (i <= LastIssueCycleVector[ResourceType]) {
          if (isEmptyLevelFinal (ResourceType, i) == false) {
            AccessWidth = AccessWidths[ResourceType];
            if (ExecutionUnitsThroughput[ResourceType] == INF)
              TmpLatency = 1;
            else{
              if(onlyScalar)
                TmpLatency =  getIssueCycleGranularity(ResourceType,
                                                       AccessWidth, 1);
              else
                TmpLatency =  getIssueCycleGranularity(ResourceType, AccessWidth,
                                                     getNElementsAccess(ResourceType,
                                                                        AccessWidth,
                                                                        VectorWidth));
            }
            MaxLatencyLevel =max(MaxLatencyLevel, TmpLatency);
          }
        }
      }
      //That is, only if there are instructions scheduled in this cycle
      if (MaxLatencyLevel != 0) {
        if (i <= DominantLevel + MaxLatency - 1) {
          if (i + MaxLatencyLevel > DominantLevel + MaxLatency &&
              MaxLatencyLevel != 0) {
            Span += ((i + MaxLatencyLevel) - max((DominantLevel + MaxLatency),
                                                 (unsigned) 1));
            DominantLevel = i;
            MaxLatency = MaxLatencyLevel;
          }
        }else {
          Span += MaxLatencyLevel;
          DominantLevel = i;
          MaxLatency = MaxLatencyLevel;
        }
      }
      
      if (NResources == 1 && MaxLatencyLevel > 0) {
        for (unsigned q = 0; q < MaxLatencyLevel; q++) {
          CISFCache[ResourcesVector[0]][i + q] = 1;
        }
      }
    }
  }
  assert (CISFCache[ResourcesVector[0]].count () == Span);
  return Span;
}


uint64_t DynamicAnalysis::calculateSpanFinal(int ResourceType)
{
  uint64_t Span = 0;
  //If there are instructions of this type....
  if (InstructionsCountExtended[ResourceType] > 0) {
    uint64_t Latency = ExecutionUnitsLatency[ResourceType];
    uint64_t First = FirstNonEmptyLevel[ResourceType];
    uint64_t DominantLevel = First;
    uint64_t LastCycle = LastIssueCycleVector[ResourceType];
    
    Span += Latency;
    
    //Start from next level to first non-emtpy level
    for (unsigned i = First + 1; i <= LastCycle; i += 1) {
      //Check whether there is instruction scheduled in this cycle!
      if (isEmptyLevelFinal(ResourceType, i) == false) {
        if (DominantLevel + Latency != 0 && i <= DominantLevel + Latency - 1) {
          if (i + Latency > DominantLevel + Latency && Latency != 0) {
            Span += ((i + Latency) -max((DominantLevel + Latency), (uint64_t) 1));
            DominantLevel = i;
          }
        }else {
          Span += Latency;
          DominantLevel = i;
        }
      }
    }
  }
  return Span;
}


bool
DynamicAnalysis::isEmptyLevelFinal(unsigned ExecutionResource, uint64_t Level)
{
  if (ExecutionResource <= NExecutionUnits) {
    if (ACTFinal.get_node_ACT (Level, ExecutionResource))
      return false;
  }
  
#ifdef EFF_TBV
  return !FullOccupancyCyclesTree[ExecutionResource].get_node(Level);
#else
  int TreeChunk = getTreeChunk(Level);
  return !FullOccupancyCyclesTree[TreeChunk].get_node(Level, ExecutionResource);
#endif
}


unsigned
DynamicAnalysis::calculateLatencyOnlySpanFinal(unsigned i)
{
  CLSFCache[i].resize(LastIssueCycleFinal + MaxLatencyResources);
  CLSFCache[i] |= CGSFCache[i];
  CLSFCache[i] ^= CISFCache[i];
  return CLSFCache[i].count();
}


unsigned
DynamicAnalysis::getGroupSpanFinal(vector < int >&ResourcesVector)
{
  dynamic_bitset <> BitMesh (LastIssueCycleFinal + MaxLatencyResources);
  for (size_t j = 0; j < ResourcesVector.size(); j++) {
    // Should probably recurse and calculate this value just in case
    if (CGSFCache[ResourcesVector[j]].size() != 0)
      BitMesh |= CGSFCache[ResourcesVector[j]];
  }
  return BitMesh.count ();
}


unsigned
DynamicAnalysis::getGroupOverlapCyclesFinal(vector < int >&ResourcesVector)
{
  dynamic_bitset <> BitMesh (LastIssueCycleFinal + MaxLatencyResources);
  for (size_t j = 0; j < ResourcesVector.size(); j++) {
    // Should probably recurse and calculate this value just in case
    if (CGSFCache[ResourcesVector[j]].size() != 0 &&
        CGSFCache[ResourcesVector[j]].count () != 0) {
      if (BitMesh.count() == 0)
        BitMesh ^= CGSFCache[ResourcesVector[j]];
      else {
        BitMesh &= CGSFCache[ResourcesVector[j]];
        if(BitMesh.count() == 0)
        return 0;
      }
    }
  }
  return BitMesh.count ();
}


// Calculate the total span of resources, and then do an AND with the
// span of the corresponding resource.
unsigned
DynamicAnalysis::getOneToAllOverlapCyclesFinal(vector < int >&ResourcesVector)
{
  dynamic_bitset <> BitMesh (LastIssueCycleFinal + MaxLatencyResources);
  
  for (size_t j = 1; j < ResourcesVector.size(); j++) {
    if (CGSFCache[ResourcesVector[j]].size() != 0 &&
        CGSFCache[ResourcesVector[j]].count () != 0)
    BitMesh |= CGSFCache[ResourcesVector[j]];
  }
  //We assume the first resoruce is the target resource, that is, the resource we
  // want to calculate the overlap with all others
  BitMesh &= CGSFCache[ResourcesVector[0]];
  
  return BitMesh.count ();
}


// Calculate the total span of resources, and then do an AND with the
// span of the corresponding resource.
unsigned
DynamicAnalysis::getOneToAllOverlapCyclesFinal (vector < int >&ResourcesVector,
                                                bool Issue)
{
  dynamic_bitset <> BitMesh (LastIssueCycleFinal + MaxLatencyResources);
  
  for (size_t j = 1; j < ResourcesVector.size(); j++) {
    if (CGSFCache[ResourcesVector[j]].size() != 0 &&
        CGSFCache[ResourcesVector[j]].count () != 0)
    BitMesh |= CGSFCache[ResourcesVector[j]];
  }
  if (Issue == true)
    BitMesh &= CISFCache[ResourcesVector[0]];
  else
    BitMesh &= CLSFCache[ResourcesVector[0]];
  
  return BitMesh.count ();
}


unsigned
DynamicAnalysis::calculateGroupSpanFinal(vector < int >&ResourcesVector)
{
  unsigned Span = 0;
  unsigned MaxLatency = 0;
  uint64_t First = 0;
  bool EmptyLevel = true;
  bool IsGap = false;
  int NResources = ResourcesVector.size();
  uint64_t LastCycle = 0;
  uint64_t ResourceLastCycle = 0;
  unsigned MaxLatencyLevel = 0;
  unsigned ResourceType = 0;
  unsigned AccessWidth = 0;
  unsigned SpanIncrease = 0;
  
  for (unsigned i = 0; i < NTotalResources; i++) {
    if (LastIssueCycleVector[i] > LastCycle)
    LastCycle = LastIssueCycleVector[i];
  }
  LastCycle += MaxLatencyResources;		// to be safe
  
  // Prepare a cache of values
  if (NResources == 1) {
    CGSFCache[ResourcesVector[0]].resize(LastCycle, false);
  }else{
    dynamic_bitset <> BitMesh (LastCycle);
    
    for (int j = 0; j < NResources; j++) {
      // Should probably recurse and calculate this value just in case
      if (CGSFCache[ResourcesVector[j]].size() == 0) {
        vector < int >tv;
        tv.push_back(ResourcesVector[j]);
        calculateGroupSpanFinal(tv);
      }
      BitMesh |= CGSFCache[ResourcesVector[j]];
    }
    return BitMesh.count ();
  }
  
  LastCycle = 0;
  
  //Determine first non-empty level and LastCycle
  for (int j = 0; j < NResources; j++) {
    ResourceType = ResourcesVector[j];
    if (InstructionsCountExtended[ResourceType] > 0) {
      AccessWidth = AccessWidths[ResourceType];
      // This will be only executed the first time of a non-empty level
      if (EmptyLevel == true) {
        EmptyLevel = false;
        First = FirstNonEmptyLevel[ResourceType];
        if (ExecutionUnitsThroughput[ResourceType] == INF) {
          MaxLatency = ExecutionUnitsLatency[ResourceType];
        }else{
          MaxLatency =
          max(ExecutionUnitsLatency[ResourceType],
              (unsigned)ceil(AccessWidth/ExecutionUnitsThroughput[ResourceType]));
        }
      }else{
        if (First == FirstNonEmptyLevel[ResourceType]) {
          if (ExecutionUnitsThroughput[ResourceType] == INF) {
            MaxLatency =max(MaxLatency, ExecutionUnitsLatency[ResourceType]);
          }
          else
            MaxLatency =
            max(MaxLatency,
              max(ExecutionUnitsLatency[ResourceType],
                  (unsigned)ceil(AccessWidth / ExecutionUnitsThroughput[ResourceType])));
        }else{
          First = min (First, FirstNonEmptyLevel[ResourceType]);
          if (First == FirstNonEmptyLevel[ResourceType]) {
            if (ExecutionUnitsThroughput[ResourceType] == INF)
              MaxLatency = ExecutionUnitsLatency[ResourceType];
            else
              MaxLatency =
              max(ExecutionUnitsLatency[ResourceType],
                (unsigned)ceil(AccessWidth / ExecutionUnitsThroughput[ResourceType]));
          }
        }
      }
      ResourceLastCycle = LastIssueCycleVector[ResourceType];
      LastCycle =max(LastCycle, ResourceLastCycle);
      
#ifdef SOURCE_CODE_ANALYSIS
      if (NResources == 1 && ResourceType < NExecutionUnits) {
        unsigned AccessWidth = AccessWidths[ResourceType];
        unsigned IssueCycleGranularity =
        getIssueCycleGranularity(ResourceType, AccessWidth,
                                 getNElementsAccess(ResourceType, AccessWidth,
                                                    VectorWidth));
        if (AvailableCyclesTree[ResourceType] != NULL) {
          AvailableCyclesTree[ResourceType] =
          splay(First, AvailableCyclesTree[ResourceType]);
          if (First == AvailableCyclesTree[ResourceType]->key) {
            if (AvailableCyclesTree[ResourceType]->issueOccupancy != 0 )
              IsInAvailableCyclesTree = true;
          }
        }
        collectSourceCodeLineStatistics (ResourceType, First, MaxLatency,
                                         MaxLatency - IssueCycleGranularity,
                                         IssueCycleGranularity,
                                         IsInAvailableCyclesTree);
      }
      if (NResources == 1 && ResourceType >= RS_STALL)
        collectSourceCodeLineStatistics(ResourceType, First, MaxLatency,
                                      MaxLatency - 1, 1, false);
#endif
    }
  }
  unsigned DominantLevel = First;
  if (NResources == 1 && MaxLatency > 0) {
    for (unsigned q = 0; q < MaxLatency; q++)
      CGSFCache[ResourcesVector[0]][First + q] = 1;
  }
  if (EmptyLevel == false) {
    Span += MaxLatency;
    for (uint64_t i = First + 1; i <= LastCycle; i++) {
      // For sure there is at least resource for which this level is not empty.
      // Determine MaxLatency of Level
      MaxLatencyLevel = 0;
      for (int j = 0; j < NResources; j++) {
        ResourceType = ResourcesVector[j];
        if (i <= LastIssueCycleVector[ResourceType]) {
          if (isEmptyLevelFinal(ResourceType, i) == false) {
            IsGap = false;
            AccessWidth = AccessWidths[ResourceType];
            if (ExecutionUnitsThroughput[ResourceType] == INF) {
              MaxLatencyLevel = max(MaxLatencyLevel,
                                    ExecutionUnitsLatency[ResourceType]);
            }else{
              MaxLatencyLevel =
              max(MaxLatencyLevel,
                  max(ExecutionUnitsLatency[ResourceType],
                      (unsigned)ceil(AccessWidth /
                                     ExecutionUnitsThroughput[ResourceType])));
            }
          }
        }
      }
      //That is, only if there are instructions scheduled in this cycle
      if (MaxLatencyLevel != 0) {
        // Add the first condition because if Latency=0 is allowed, it can happen
        // that DominantLevel+MaxLatency-1 is a negative number, so the loop
        // is entered incorrectly.
        if (DominantLevel + MaxLatency != 0 && i <= DominantLevel + MaxLatency - 1) {
          if (i + MaxLatencyLevel > DominantLevel + MaxLatency &&
              MaxLatencyLevel != 0) {
            SpanIncrease = ((i + MaxLatencyLevel) -
                            max((DominantLevel + MaxLatency), (unsigned) 1));
            Span += SpanIncrease;
            DominantLevel = i;
            MaxLatency = MaxLatencyLevel;
          }
        }else {
          SpanIncrease = MaxLatencyLevel;
          Span += MaxLatencyLevel;
          DominantLevel = i;
          MaxLatency = MaxLatencyLevel;
        }
        
#ifdef SOURCE_CODE_ANALYSIS
        ResourceType = ResourcesVector[0];
        if (NResources == 1 && (ResourceType < NExecutionUnits ||
                                ResourceType >= RS_STALL)) {
          unsigned AccessWidth = AccessWidths[ResourceType];
          unsigned IssueCycleGranularity =
          getIssueCycleGranularity(ResourceType, AccessWidth,
                                   getNElementsAccess(ResourceType, AccessWidth,
                                                      VectorWidth));
          
          if (AvailableCyclesTree[ResourceType] != NULL) {
            AvailableCyclesTree[ResourceType] =
            splay(i, AvailableCyclesTree[ResourceType]);
            if (i == AvailableCyclesTree[ResourceType]->key) {
              if (AvailableCyclesTree[ResourceType]->issueOccupancy != 0) {
                IsInAvailableCyclesTree = true;
              }
            }
          }
          collectSourceCodeLineStatistics(ResourceType, i, MaxLatencyLevel,
                                          SpanIncrease, IssueCycleGranularity,
                                          IsInAvailableCyclesTree);
        }
#endif
      }else{
        if (i > DominantLevel + MaxLatency - 1) {
          if (NResources == 1 && IsGap == false) {
            SpanGaps[ResourceType]++;
            IsGap = true;
          }
        }
      }
      if (NResources == 1 && MaxLatencyLevel > 0) {
        for (unsigned q = 0; q < MaxLatencyLevel; q++) {
          CGSFCache[ResourcesVector[0]][i + q] = 1;
        }
      }
    }
  }
  // Delta should be 0
  unsigned delta = Span - CGSFCache[ResourcesVector[0]].count ();
  if (delta != 0)
    report_fatal_error("Error calculating span\n");
  
  return Span;
}


// Tree is unbalanced, switch from recursive to iterative method
void
DynamicAnalysis::computeAvailableTreeFinalHelper(uint p, Tree < uint64_t > *t,
                                                 uint d)
{
  uint64_t lastKey = 0;
  while (true) {
    if (t->left != NULL) {
      t->left->prev = t;
      t = t->left;
      continue;
    }
    // insert element
    if (t->key >= lastKey) {
      ACTNode *n = new ACTNode;
      n->key = t->key;
      n->issueOccupancy = t->issueOccupancy;
      n->widthOccupancy = t->widthOccupancy;
      n->occupancyPrefetch = t->occupancyPrefetch;
      n->address = t->address;
      ACTFinal.push_back(n, p);
      lastKey = t->key;
    }
    if (t->right != NULL) {
      t->right->prev = t;
      t = t->right;
      continue;
    }
    if (t->prev != NULL) {
      Tree < uint64_t > *old = t;
      t = t->prev;
      if (t->left == old)
        t->left = NULL;
      if (t->right == old)
        t->right = NULL;
      delete old;
      continue;
    }else
      break;
  }
}


void
DynamicAnalysis::computeAvailableTreeFinal()
{
  uint p = 0;
  for (auto it = AvailableCyclesTree.begin(),
       et = AvailableCyclesTree.end(); it != et; ++it) {
    if ((*it) != NULL) {
      computeAvailableTreeFinalHelper(p, *it, 0);
      *it = NULL;
    }
    p++;
  }
}


//===----------------------------------------------------------------------===//
//                      Routine for source code analysis
//===----------------------------------------------------------------------===//

#ifdef SOURCE_CODE_ANALYSIS
void
DynamicAnalysis::collectSourceCodeLineStatistics(uint64_t ResourceType,
                                                  uint64_t Cycle,
                                                  uint64_t MaxLatencyLevel,
                                                  uint64_t SpanIncrease,
                                                  uint64_t IssueCycleGranularity,
                                                  bool IsInAvailableCyclesTree)
{
#ifdef EFF_TBV
  getTreeChunk(Cycle, ResourceType);
#else
  unsigned TreeChunk = getTreeChunk(Cycle);
#endif
  uint64_t Line = 0;
  unsigned Resource = 0;
  
  if (FullOccupancyCyclesTree[TreeChunk].get_node(Cycle, ResourceType)) {
    vector < pair < unsigned, unsigned >>SourceCodeLinesOperationPair =
				FullOccupancyCyclesTree[TreeChunk].get_source_code_lines (Cycle);

    for (std::vector < pair < unsigned, unsigned >>::iterator it =
         SourceCodeLinesOperationPair.begin();
         it != SourceCodeLinesOperationPair.end(); ++it) {
      Line = (*it).first;
      Resource = (*it).second;
      
      // SourceCodeLineInfo[Line] contains a set of the cycles associated to this line.
      // We want all the cycles associated to a line for the fraction of the total
      // span this line contributes to
      
      if (ResourceType == Resource) {
        vector < uint64_t > emptyVector;
        set < uint64_t > emptySet;
        map < uint64_t, set < uint64_t > >::iterator it_SourceCodeLineInfo =
        SourceCodeLineInfo.find(Line);
        
        if (it_SourceCodeLineInfo == SourceCodeLineInfo.end()){
          SourceCodeLineInfo[Line] = emptySet;
          SourceCodeLineInfoBreakdown[Line] = emptyVector;
          for (int z = 0; z <= 47; z++) {
            SourceCodeLineInfoBreakdown[Line].push_back(0);
          }
        }
        for (unsigned k = Cycle; k < Cycle + MaxLatencyLevel; k++) {
          SourceCodeLineInfo[Line].insert (k);
        }
        
        unsigned overlapCycles = MaxLatencyLevel - SpanIncrease;
        unsigned removeLatencyCycles = 0;
        if(overlapCycles < IssueCycleGranularity)
          removeLatencyCycles = overlapCycles;
        else
          removeLatencyCycles = IssueCycleGranularity;
        
        // Issue cycle granularity
        for(unsigned i = 0; i< IssueCycleGranularity; i++){
          SourceCodeLineInfoBreakdown[Line]
          [getPositionSourceCodeLineInfoVector(ResourceType)]++;
        }
        
        for(unsigned i = 0; i< removeLatencyCycles; i++){
          SourceCodeLineInfoBreakdown[Line]
          [getPositionSourceCodeLineInfoVector (ResourceType) + 1]--;
        }
        
        for (unsigned k = 0; k < SpanIncrease; k++) {
          SourceCodeLineInfoBreakdown[Line]
          [getPositionSourceCodeLineInfoVector(ResourceType) + 1]++;
        }
      }
    }
  }
  
  if (IsInAvailableCyclesTree == true &&
      AvailableCyclesTree[ResourceType] != NULL &&
      AvailableCyclesTree[ResourceType]->key == Cycle) {
    
    // For every line in the source code
    for (auto it = AvailableCyclesTree[ResourceType]->SourceCodeLinesOperationPair.begin();
         it != AvailableCyclesTree[ResourceType]->SourceCodeLinesOperationPair.end(); ++it) {
      Line = (*it).first;
      
      Resource = ResourceType;
      
      vector < uint64_t > emptyVector;
      set < uint64_t > emptySet;
      
      map < uint64_t, set < uint64_t > >::iterator it_SourceCodeLineInfo =
      SourceCodeLineInfo.find(Line);
      
      if (it_SourceCodeLineInfo == SourceCodeLineInfo.end()){
        SourceCodeLineInfo[Line] = emptySet;
        SourceCodeLineInfoBreakdown[Line] = emptyVector;
        for (int z = 0; z <= 47; z++) {
          SourceCodeLineInfoBreakdown[Line].push_back(0);
        }
      }
      for (unsigned k = Cycle; k < Cycle + MaxLatencyLevel; k++) {
        SourceCodeLineInfo[Line].insert (k);
      }
      
      unsigned overlapCycles = MaxLatencyLevel - SpanIncrease;
      unsigned removeLatencyCycles = 0;
      if(overlapCycles < IssueCycleGranularity)
      removeLatencyCycles = overlapCycles;
      else
      removeLatencyCycles = IssueCycleGranularity;
      
      // Issue cycle granularity
      for(unsigned i = 0; i< IssueCycleGranularity; i++)
        SourceCodeLineInfoBreakdown[Line]
      [getPositionSourceCodeLineInfoVector (ResourceType)]++;

      for(unsigned i = 0; i< removeLatencyCycles; i++){
        SourceCodeLineInfoBreakdown[Line]
        [getPositionSourceCodeLineInfoVector (ResourceType) + 1]--;
        
      }
  
      for (unsigned k = 0; k < SpanIncrease; k++) {
        SourceCodeLineInfoBreakdown[Line]
        [getPositionSourceCodeLineInfoVector (ResourceType) + 1]++;
      }
    }
  }
}


unsigned
DynamicAnalysis::getPositionSourceCodeLineInfoVector (uint64_t Resource)
{
  switch (Resource) {
    case FP32_ADDER:
    return 0;
    break;
    case FP64_ADDER:
    return 2;
    break;
    case FP32_MULTIPLIER:
    return 4;
    case FP64_MULTIPLIER:
    return 6;
    break;
    case FP32_FMADDER:
    return 8;
    case FP64_FMADDER:
    return 10;
    break;
    
    case FP32_DIVIDER:
    return 12;
    break;
    case FP64_DIVIDER:
    return 14;
    break;
    case FP32_SHUFFLE_UNIT:
    return 16;
    break;
    case FP64_SHUFFLE_UNIT:
    return 18;
    break;
    case FP32_BLEND_UNIT:
    return 20;
    break;
    case FP64_BLEND_UNIT:
    return 22;
    break;
    case REGISTER_LOAD_CHANNEL:
    return 24;
    break;
    
    case L1_LOAD_CHANNEL:
    return 26;
    break;
    case L1_STORE_CHANNEL:
    return 28;
    break;
    case L2_LOAD_CHANNEL:
    return 30;
    break;
    
    case L3_LOAD_CHANNEL:
    return 32;
    break;
    
    case MEM_LOAD_CHANNEL:
    return 34;
    break;
    
    case RS_STALL:
    return 36;
    break;
    case ROB_STALL:
    return 37;
    break;
    case LB_STALL:
    return 38;
    break;
    case SB_STALL:
    return 39;
    break;
    case LFB_STALL:
    return 40;
    break;
    
    case ADDRESS_GENERATION_UNIT:
    return 41;
    break;
    case PORT_0:
    return 42;
    break;
    case PORT_1:
    return 43;
    break;
    case PORT_2:
    return 44;
    break;
    case PORT_3:
    return 45;
    break;
    case PORT_4:
    return 46;
    break;
    case PORT_5:
    return 47;
    break;
    
    default:
    report_fatal_error("Resource " + getResourceName(Resource) + " unknown \
                       while retrieving source code line information.");
    break;
  }
}
#endif


//===----------------------------------------------------------------------===//
//                      OoO Buffers routines
//===----------------------------------------------------------------------===//
uint64_t
DynamicAnalysis::getMinIssueCycleReservationStation()
{
  vector < uint64_t >::iterator it;
  uint64_t  MinIssueCycle = ReservationStationIssueCycles.front ();
  for (it = ReservationStationIssueCycles.begin();
       it != ReservationStationIssueCycles.end(); ++it)
    MinIssueCycle = min (MinIssueCycle, *it);
  
  return MinIssueCycle;
}


uint64_t
DynamicAnalysis::getMinCompletionCycleLoadBuffer()
{
  vector < uint64_t >::iterator it;
  uint64_t  MinCompletionCycle = LoadBufferCompletionCycles.front();
  for (it = LoadBufferCompletionCycles.begin();
       it != LoadBufferCompletionCycles.end(); ++it) {
    MinCompletionCycle = min (MinCompletionCycle, *it);
  }
  return MinCompletionCycle;
}


uint64_t
DynamicAnalysis::getMinCompletionCycleLoadBufferTree()
{
  return MinLoadBuffer;
}

  
uint64_t DynamicAnalysis::getMinCompletionCycleStoreBuffer()
{
  vector < uint64_t >::iterator it;
  
  uint64_t  MinCompletionCycle = StoreBufferCompletionCycles.front();
  for (it = StoreBufferCompletionCycles.begin();
       it != StoreBufferCompletionCycles.end(); ++it) {
    MinCompletionCycle = min (MinCompletionCycle, *it);
  }
  return MinCompletionCycle;
}
  

uint64_t DynamicAnalysis::getMinCompletionCycleLineFillBuffer()
{
  vector < uint64_t >::iterator it;
  
  uint64_t MinCompletionCycle = LineFillBufferCompletionCycles.front();
  for (it = LineFillBufferCompletionCycles.begin();
       it != LineFillBufferCompletionCycles.end(); ++it) {
    MinCompletionCycle = min (MinCompletionCycle, *it);
  }
  return MinCompletionCycle;
}

  
void
DynamicAnalysis::removeFromReservationStation(uint64_t Cycle)
{
  LessThanOrEqualValuePred Predicate = { Cycle };
  ReservationStationIssueCycles.
  erase (std::remove_if (ReservationStationIssueCycles.begin(),
                         ReservationStationIssueCycles.end(), Predicate),
         ReservationStationIssueCycles.end());
}


void
DynamicAnalysis::removeFromReorderBuffer(uint64_t Cycle)
{
  while (!ReorderBufferCompletionCycles.empty()
         && ReorderBufferCompletionCycles.front () <= Cycle)
    ReorderBufferCompletionCycles.pop_front ();
}


void
DynamicAnalysis::removeFromLoadBuffer(uint64_t Cycle)
{
  LessThanOrEqualValuePred Predicate = { Cycle };
  LoadBufferCompletionCycles.
  erase (std::remove_if (LoadBufferCompletionCycles.begin(),
                         LoadBufferCompletionCycles.end(), Predicate),
         LoadBufferCompletionCycles.end());
}

  
void
DynamicAnalysis::removeFromLoadBufferTree(uint64_t Cycle)
{
  bool CycleFound = true;
  while (LoadBufferCompletionCyclesTree != NULL && (CycleFound == true ||
                                                    Cycle >= MinLoadBuffer)) {

    LoadBufferCompletionCyclesTree = splay (Cycle, LoadBufferCompletionCyclesTree);
    
    if (LoadBufferCompletionCyclesTree->key == Cycle) {	// If Cycle found
      LoadBufferCompletionCyclesTree->left = NULL;
      
      LoadBufferCompletionCyclesTree =
      delete_node(Cycle, LoadBufferCompletionCyclesTree);
      if (Cycle >= MinLoadBuffer && LoadBufferCompletionCyclesTree != NULL)
        MinLoadBuffer = min (LoadBufferCompletionCyclesTree);
      if(LoadBufferCompletionCyclesTree==NULL)
        break;
    }else {
      if(Cycle >= MinLoadBuffer)
        Cycle--;
      else
        CycleFound = false;
    }
    CycleFound = false;
  }
}
  

void
DynamicAnalysis::removeFromStoreBuffer(uint64_t Cycle)
{
  LessThanOrEqualValuePred Predicate = { Cycle };
  StoreBufferCompletionCycles.
  erase (std::remove_if (StoreBufferCompletionCycles.begin(),
                         StoreBufferCompletionCycles.end(), Predicate),
         StoreBufferCompletionCycles.end());
}


void
DynamicAnalysis::removeFromLineFillBuffer(uint64_t Cycle)
{
  LessThanOrEqualValuePred Predicate = { Cycle };
  LineFillBufferCompletionCycles.
  erase(std::remove_if(LineFillBufferCompletionCycles.begin(),
                       LineFillBufferCompletionCycles.end(), Predicate),
        LineFillBufferCompletionCycles.end());
}


void
DynamicAnalysis::removeFromDispatchToLoadBufferQueue (uint64_t Cycle)
{
  StructMemberLessThanOrEqualThanValuePred Predicate = { Cycle };
  DispatchToLoadBufferQueue.
  erase (std::remove_if (DispatchToLoadBufferQueue.begin(),
                         DispatchToLoadBufferQueue.end(), Predicate),
         DispatchToLoadBufferQueue.end());
}


void
DynamicAnalysis::removeFromDispatchToStoreBufferQueue(uint64_t Cycle)
{
  StructMemberLessThanOrEqualThanValuePred Predicate = { Cycle };
  DispatchToStoreBufferQueue.
  erase (std::remove_if (DispatchToStoreBufferQueue.begin(),
                         DispatchToStoreBufferQueue.end(), Predicate),
         DispatchToStoreBufferQueue.end());
}


void
DynamicAnalysis::removeFromDispatchToLineFillBufferQueue(uint64_t Cycle)
{
  StructMemberLessThanOrEqualThanValuePred Predicate = { Cycle };
  DispatchToLineFillBufferQueue.
  erase (std::remove_if (DispatchToLineFillBufferQueue.begin(),
                         DispatchToLineFillBufferQueue.end(), Predicate),
         DispatchToLineFillBufferQueue.end());
}


void
DynamicAnalysis::dispatchToLoadBuffer(uint64_t Cycle)
{
  vector < InstructionDispatchInfo >::iterator it =
  DispatchToLoadBufferQueue.begin();
  for (; it != DispatchToLoadBufferQueue.end();) {
    if ((*it).IssueCycle == InstructionFetchCycle) {
      //Erase returns the next valid iterator => insert in LoadBuffer before it
      // is removed
      LoadBufferCompletionCycles.push_back((*it).CompletionCycle);
      it = DispatchToLoadBufferQueue.erase (it);
    }else
      ++it;
  }
}


void
DynamicAnalysis::inOrder(uint64_t i, ComplexTree < uint64_t > *n)
{
  bool condition = false;
  bool StopChecking = false;
  
  if (n == NULL)
    return;
  else
    inOrder (i, n->left);
 
  for(std::vector<uint64_t>::iterator it = n->IssueCycles.begin();
      it != n->IssueCycles.end(); ++it) {
    if (*it <= i && *it != 0) {
      condition = true;
      break;
    }
  }
  if(condition==true){
    for(std::vector<uint64_t>::iterator it = n->IssueCycles.begin();
        (!StopChecking && it != n->IssueCycles.end()); ++it) {
      if (*it <= i && *it != 0) {
        // Insert only if completion cycle is not smaller than i
        if(n->key > i){
          if (node_size(LoadBufferCompletionCyclesTree) == 0)
            MinLoadBuffer = n->key;
          else
            MinLoadBuffer = min (MinLoadBuffer, n->key);
          LoadBufferCompletionCyclesTree =
          insert_node(n->key, LoadBufferCompletionCyclesTree);
        }
        if(n->IssueCycles.size()==1)
          StopChecking = true;
        DispatchToLoadBufferQueueTreeCyclesToRemove.
        push_back(std::make_pair(n->key,*it));
      }
    }
  }
  if(n!=NULL)
    inOrder(i, n->right);
}


void
DynamicAnalysis::dispatchToLoadBufferTree(uint64_t Cycle)
{
  DispatchToLoadBufferQueueTreeCyclesToRemove.clear();
  inOrder (Cycle, DispatchToLoadBufferQueueTree);
  for(unsigned int i = 0; i< DispatchToLoadBufferQueueTreeCyclesToRemove.size();i++){
    DispatchToLoadBufferQueueTree =
    delete_node(DispatchToLoadBufferQueueTreeCyclesToRemove[i].first,
                DispatchToLoadBufferQueueTreeCyclesToRemove[i].second,
                DispatchToLoadBufferQueueTree);
    
  }
}


void
DynamicAnalysis::dispatchToStoreBuffer(uint64_t Cycle)
{
  vector < InstructionDispatchInfo >::iterator it =
  DispatchToStoreBufferQueue.begin();
  for (; it != DispatchToStoreBufferQueue.end();) {
    if ((*it).IssueCycle == InstructionFetchCycle) {
      StoreBufferCompletionCycles.push_back((*it).CompletionCycle);
      it = DispatchToStoreBufferQueue.erase (it);
    }else
      ++it;
  }
}


void
DynamicAnalysis::dispatchToLineFillBuffer(uint64_t Cycle)
{
  vector < InstructionDispatchInfo >::iterator it =
  DispatchToLineFillBufferQueue.begin();
  for (; it != DispatchToLineFillBufferQueue.end();) {
    
    if ((*it).IssueCycle == InstructionFetchCycle) {
      LineFillBufferCompletionCycles.push_back((*it).CompletionCycle);
      it = DispatchToLineFillBufferQueue.erase (it);
    }else
      ++it;
  }
}


uint64_t DynamicAnalysis::findIssueCycleWhenLineFillBufferIsFull()
{
  size_t  BufferSize = DispatchToLineFillBufferQueue.size();
  
  if (BufferSize == 0)
    return getMinCompletionCycleLineFillBuffer ();
  else{
    if (BufferSize >= (unsigned) LineFillBufferSize) {
      // Iterate from end-LineFillBufferSize
      uint64_t
      EarliestCompletion = DispatchToLineFillBufferQueue.back ().CompletionCycle;
      for (vector < InstructionDispatchInfo >::iterator it =
           DispatchToLineFillBufferQueue.end() - 1;
           it >= DispatchToLineFillBufferQueue.end() - LineFillBufferSize; --it) {
        if ((*it).CompletionCycle < EarliestCompletion)
          EarliestCompletion = (*it).CompletionCycle;
      }
      return EarliestCompletion;
    }else {
      sort (LineFillBufferCompletionCycles.begin(),
            LineFillBufferCompletionCycles.end());
      return LineFillBufferCompletionCycles[BufferSize];
    }
  }
}


uint64_t DynamicAnalysis::findIssueCycleWhenLoadBufferIsFull()
{
  size_t  BufferSize = DispatchToLoadBufferQueue.size();
  
  if (BufferSize == 0) {
    return getMinCompletionCycleLoadBuffer ();
  }else {
    // Iterate through the DispathToLoadBufferQueue and get the
    // largest dispatch cycle. The new load cannot be dispatched
    // untill all previous in Dispatch Queue have been dispatched.
    uint64_t EarliestDispatchCycle = 0;
    for (vector < InstructionDispatchInfo >::iterator it =
         DispatchToLoadBufferQueue.begin();
         it != DispatchToLoadBufferQueue.end(); ++it) {
      EarliestDispatchCycle =max(EarliestDispatchCycle, (*it).IssueCycle);
    }
    //Traverse LB and count how many elements are there *smaller than or equal*
    // EarliestDispathCycle
    unsigned  counter = 0;
    for (vector < uint64_t >::iterator it = LoadBufferCompletionCycles.begin();
         it != LoadBufferCompletionCycles.end();
         ++it) {
      if ((*it) <= EarliestDispatchCycle)
        counter++;
    }
    uint64_t IssueCycle = 0;
    // This means that in LB, there are more loads that terminate before or in
    // my dispatch cycle -> IssueCycle is Earliest
    if (counter > BufferSize) {
      IssueCycle = EarliestDispatchCycle;
    }else {
      if (counter == BufferSize|| (counter < BufferSize &&
                                   counter==LoadBufferCompletionCycles.size() &&
                                   BufferSize > LoadBufferSize) ) {
        // Iterate through both, DispatchBufferQueue and LB to count how many
        // elements are completed in a cycle *larger than* EarliestDispatchCycle
        unsigned CompletedAfterCounter = 0;
        for (vector < InstructionDispatchInfo >::iterator it =
             DispatchToLoadBufferQueue.begin();
             it != DispatchToLoadBufferQueue.end(); ++it) {
          if ((*it).CompletionCycle > EarliestDispatchCycle)
          CompletedAfterCounter++;
        }
        for (vector < uint64_t >::iterator it = LoadBufferCompletionCycles.begin();
             it != LoadBufferCompletionCycles.end();
             ++it) {
          if ((*it) > EarliestDispatchCycle)
          CompletedAfterCounter++;
        }
        if(CompletedAfterCounter < LoadBufferSize){
          IssueCycle = EarliestDispatchCycle;
        }else{
          // Iterate through both, DispatchBufferQueue and LB to determine the
          // smallest completion cycle which is larger than EarliestDispatchCycle.
          // Initialize with the Completion cycle of the last element of the
          // DispatchToLoadBufferQueue
          IssueCycle = DispatchToLoadBufferQueue.back ().CompletionCycle;
          for (vector < InstructionDispatchInfo >::iterator it =
               DispatchToLoadBufferQueue.begin();
               it != DispatchToLoadBufferQueue.end(); ++it) {
            if ((*it).CompletionCycle > EarliestDispatchCycle)
              IssueCycle = min (IssueCycle, (*it).CompletionCycle);
          }
          // We have to also iterate over the completion cycles of the LB even
          // if there are more elements in the DispatchQueue than the size
          // of the LB. Because it can happen than all the elements of the
          // DispatchQueue are complemente even before than an element in the
          // LB which is waiting very long for a resource.
          // We could, nevertheless, simplify it. We can keep the max and
          // the min completion cycle always. If the max completion cycle
          // is smaller than the EarliestDispatchCycle, then it is not necessary
          // to iterate over the LB.
          for (vector < uint64_t >::iterator it = LoadBufferCompletionCycles.begin();
               it != LoadBufferCompletionCycles.end();
               ++it) {
            if ((*it) > EarliestDispatchCycle)
              IssueCycle = min (IssueCycle, *it);
          }
        }
      }else
        report_fatal_error("Error in Dispatch to Load Buffer Queue");
    }
    return IssueCycle;
  }
}


uint64_t DynamicAnalysis::findIssueCycleWhenLoadBufferTreeIsFull ()
{
  size_t BufferSize = node_size(DispatchToLoadBufferQueueTree);
  if (BufferSize == 0){
    return getMinCompletionCycleLoadBufferTree ();
  }else {
    uint64_t EarliestDispatchCycle = 0;
    uint64_t SlotsLoadBufferCompleteBeforeEarliestDispatch = 0;
    uint64_t SlotsDispatchCompleteBeforeEarliest = 0;
    uint64_t SlotsDispatchCompleteAfterEarliest = 0;
    uint64_t TotalCyclesCompleteAfter = 0;
    // Iterate through the DispathToLoadBufferQueue and get the
    // largest dispatch cycle. The new load cannot be dispatched
    // untill all previous in Dispatch Queue have been dispatched.
    //  We keep a variable, so no need to iterate
    EarliestDispatchCycle = MaxDispatchToLoadBufferQueueTree;
    
    //Traverse LB and count how many elements are there *smaller than or equal*
    // EarliestDispathCycle
    LoadBufferCompletionCyclesTree =
    splay(EarliestDispatchCycle, LoadBufferCompletionCyclesTree);
    
    // When we splay, the following can happen:
    // key == Earliest: account for the size of the left node, if any, and
    // the duplicates of key
    // key > Earliest: account for the size of the left node, if any.
    // key < Earliest: account for the size of the left node, if any, plus the
    // duplicated.
    // If node->left is not NULL, is the size of left. If, moreover,
    // node->key <= EarliestComplettionCycle, then add also the duplicates of
    // this node.
    if (LoadBufferCompletionCyclesTree->left != NULL)
      SlotsLoadBufferCompleteBeforeEarliestDispatch =
    node_size(LoadBufferCompletionCyclesTree->left);
    if(LoadBufferCompletionCyclesTree->key <= EarliestDispatchCycle)
      SlotsLoadBufferCompleteBeforeEarliestDispatch +=
    LoadBufferCompletionCyclesTree->duplicates;
    
    TotalCyclesCompleteAfter = LoadBufferSize -
                                SlotsLoadBufferCompleteBeforeEarliestDispatch;
    
    // Traverse DispatchToLoadBufferQueuteTree and count how many
    // complete after my EarliestDispatchCycle, and how many earlier
    DispatchToLoadBufferQueueTree =
    splay (EarliestDispatchCycle, DispatchToLoadBufferQueueTree);
    
    if(DispatchToLoadBufferQueueTree->left!=NULL)
      SlotsDispatchCompleteBeforeEarliest =
      node_size(DispatchToLoadBufferQueueTree->left);
    if(DispatchToLoadBufferQueueTree->right!=NULL)
      SlotsDispatchCompleteAfterEarliest =
      node_size(DispatchToLoadBufferQueueTree->right);
    
    if(DispatchToLoadBufferQueueTree->key <= EarliestDispatchCycle)
      SlotsDispatchCompleteBeforeEarliest +=
      DispatchToLoadBufferQueueTree->IssueCycles.size();
    else
      SlotsDispatchCompleteAfterEarliest+=
      DispatchToLoadBufferQueueTree->IssueCycles.size();
    
    TotalCyclesCompleteAfter+=SlotsDispatchCompleteAfterEarliest;
    
    uint64_t IssueCycle = 0;
    // This means that in LB, there are more loads that terminate before or in
    // my dispatch cycle -> IssueCycle is Earliest
    if(TotalCyclesCompleteAfter < LoadBufferSize)
      IssueCycle = EarliestDispatchCycle;
    else{
      ComplexTree < uint64_t > *Node = DispatchToLoadBufferQueueTree;
      //PrintdispatchToLoadBufferTree();
      while (true) {
        // This is the mechanism used in the original algorithm to delete the host
        // node,  decrementing the last_record attribute of the host node, and
        // the size attribute of all parents nodes.
        // Node->size = Node->size-1;
        if (EarliestDispatchCycle + 1 < Node->key) {
          if (Node->left == NULL)
            break;
          if (Node->left->key < EarliestDispatchCycle + 1) {
            break;
          }
          Node = Node->left;
        }else {
          if (EarliestDispatchCycle + 1 > Node->key) {
            if (Node->right == NULL)
              break;
            Node = Node->right;
          }else{		// Last = Node->key, i.e., Node is the host node
            break;
          }
        }
      }
      IssueCycle = Node->key;
      //Get the closest larger than or equal to EarliestaDispatchCycle
      SimpleTree < uint64_t > *TmpNode = LoadBufferCompletionCyclesTree;
      while (true) {
        // This is the mechanism used in the original algorithm to delete the host
        // node,  decrementing the last_record attribute of the host node, and
        // the size attribute of all parents nodes.
        // Node->size = Node->size-1;
        if (EarliestDispatchCycle + 1 < TmpNode->key) {
          if (TmpNode->left == NULL)
            break;
          if (TmpNode->left->key < EarliestDispatchCycle + 1) {
            break;
          }
          TmpNode = TmpNode->left;
        }else {
          if (EarliestDispatchCycle + 1 > TmpNode->key) {
            if (TmpNode->right == NULL)
              break;
            TmpNode = TmpNode->right;
          }
          else {		// Last = Node->key, i.e., Node is the host node
            break;
          }
        }
      }
      if (TmpNode->key >= EarliestDispatchCycle + 1) {
        IssueCycle = min (TmpNode->key, IssueCycle);
      }
    }
    return IssueCycle;
  }
}

  
uint64_t DynamicAnalysis::findIssueCycleWhenStoreBufferIsFull()
{
  size_t BufferSize = DispatchToStoreBufferQueue.size();
  if (BufferSize == 0)
    return getMinCompletionCycleStoreBuffer ();
  else{
    if (BufferSize >= (unsigned) StoreBufferSize) {
      uint64_t EarliestCompletion =
      DispatchToStoreBufferQueue.back ().CompletionCycle;
      for (vector < InstructionDispatchInfo >::iterator it =
           DispatchToStoreBufferQueue.end() - 1;
           it >= DispatchToStoreBufferQueue.end() - StoreBufferSize; --it) {
        if ((*it).CompletionCycle < EarliestCompletion)
          EarliestCompletion = (*it).CompletionCycle;
      }
      return EarliestCompletion;
    }else {
      sort (StoreBufferCompletionCycles.begin(), StoreBufferCompletionCycles.end());
      return StoreBufferCompletionCycles[BufferSize];
    }
  }
}


//===----------------------------------------------------------------------===//
//          Main method that schedules a node in the DAG
//===----------------------------------------------------------------------===//

// Handling instructions dependences with def-use chains.
// Whenever there is a def, and we know the issue cycle (IssueCycle) of the def,
// update all the uses of that definition with IssueCycle+1.
// The issue cycle of an instruction is hence the max of the issue cycles of its
// operands, but the issue cycle of its operands does not have to be determined,
// already contains the right value because they are uses of a previous definition.

void
DynamicAnalysis::analyzeInstruction (Instruction & I, unsigned OpCode,
                                     uint64_t addr, unsigned Line,
                                     bool forceAnalyze, unsigned VectorWidth,
                                     unsigned valueRep, bool lastValue,
                                     bool firstValue, bool isSpill)
{
  int k = 0;
  int Distance = -1;
  int RegisterStackDistance = -1;
  int NextCacheLineExtendedInstructionType;
  int InstructionType;
  
  CacheLineInfo Info;
  uint64_t CacheLine = 0;
  uint64_t MemoryAddress = 0;
  uint64_t NextCacheLine;
  uint64_t NextCacheLineIssueCycle;
  uint64_t InstructionIssueCycle, OriginalInstructionIssueCycle;
  uint64_t InstructionIssueFetchCycle = 0,
  
  InstructionIssueLoadBufferAvailable = 0,
  InstructionIssueLineFillBufferAvailable = 0,
  InstructionIssueStoreBufferAvailable = 0,
  InstructionIssueAGUAvailable = 0,
  InstructionIssueInOrderExecution = 0,
  InstructionIssueDataDeps = 0,
  InstructionIssueCacheLineAvailable = 0,
  InstructionIssueMemoryModel = 0,
  InstructionIssueStoreAGUAvailable = 0,
  InstructionIssueLoadAGUAvailable = 0,
  InstructionIssueThroughputAvailable = 0;
  
  uint Latency = 0;
  uint LatencyPrefetch = 0;
  
  //Aux vars for handling arguments of a call instruction
  CallSite CS;
  Function *F;
  std::vector < Value * >ArgVals;
  unsigned NumArgs;
  
  unsigned ExecutionResource = 0;
  
  unsigned NElementsVector = 1;
  bool IsVectorInstruction = false;
  
  bool isLoad = true;
  bool isRegisterSpill = false;
  
  unsigned Port = 0;
  
  // Reset IssuePorts
  IssuePorts = vector < unsigned >();
  
  vector < uint64_t > emptyVector;
  
  PointerToMemory instructionPTM;
  InstructionValue instValue;
  int64_t valueInstance;
  PointerToMemoryInstance instructionPTMI;
  PointerToMemory associatedPTM;
  PointerToMemoryInstance associatedPTMI;
  
  unsigned PrvFP32DivLat = 0;
  unsigned PrvFP64DivLat = 0;
  double PrvFP32DivTh = 0.0;
  double PrvFP64DivTh = 0.0;
  
  if(isSpill || (forceAnalyze && (OpCode == Instruction::Load ||
                                  OpCode == Instruction::Store)))
    InstructionType = 1;
  else{
    if (forceAnalyze && (OpCode == Instruction::FDiv ||
                         OpCode == Instruction::FAdd)){
      InstructionType = 0;
    }else
    InstructionType = getInstructionType (I);
  }
  
  unsigned ExtendedInstructionType = InstructionType;

  
  if(forceAnalyze && (OpCode == Instruction::FDiv) &&
     Microarchitecture.compare("ARM-CORTEX-A9") == 0){
	   if (CallInst *CI = dyn_cast<CallInst> (&I)){
       Function * f = CI->getCalledFunction();
       if (f->getName().find("exp") != string::npos) {
         PrvFP32DivLat = ExecutionUnitsLatency[FP32_DIVIDER];
         PrvFP64DivLat = ExecutionUnitsLatency[FP64_DIVIDER];
         PrvFP32DivTh = ExecutionUnitsThroughput[FP32_DIVIDER];
         PrvFP64DivTh = ExecutionUnitsThroughput[FP64_DIVIDER];
         ExecutionUnitsLatency[FP32_DIVIDER] = 17; // TODO: determine this value
         ExecutionUnitsLatency[FP64_DIVIDER] = 162;
         ExecutionUnitsThroughput[FP32_DIVIDER] = 0.0769;// TODO: determine this value
         ExecutionUnitsThroughput[FP64_DIVIDER] = 0.0061728;
       }else{
         PrvFP32DivLat = ExecutionUnitsLatency[FP32_DIVIDER];
         PrvFP64DivLat = ExecutionUnitsLatency[FP64_DIVIDER];
         PrvFP32DivTh = ExecutionUnitsThroughput[FP32_DIVIDER];
         PrvFP64DivTh = ExecutionUnitsThroughput[FP64_DIVIDER];
         ExecutionUnitsLatency[FP32_DIVIDER] = 17;
         ExecutionUnitsLatency[FP64_DIVIDER] = 32;
         ExecutionUnitsThroughput[FP32_DIVIDER] = 0.0769;
         ExecutionUnitsThroughput[FP64_DIVIDER] = 0.0357;
         
       }
     }
  }
  //=============== WARM CACHE ANALYSIS - RECORD ONLY MEMORY ACCESSES =========//
  if (WarmCache && rep == 0) {
    if ((InstructionType >= 0 || forceAnalyze == true ||
         OpCode ==Instruction::GetElementPtr ||
         OpCode ==Instruction::BitCast || OpCode == Instruction::Alloca)) {
      
#ifdef VALUE_ANALYSIS
      
      // =======================================================================
      // Create PointerToMemory of the instruction
      // =======================================================================
      instructionPTM = {&I, NULL, NULL, NULL, NULL, NULL};
      // =======================================================================
      // Create InstructionValue to get the instance of the instruction
      // =======================================================================
      instValue = {&I, valueRep};
      valueInstance = getInstructionValueInstance(instValue);
      // =======================================================================
      // Create PointerToMemoryInstance
      // =======================================================================
      instructionPTMI = {instructionPTM, valueRep, valueInstance};
#endif
      
      switch (OpCode) {
#ifdef VALUE_ANALYSIS
        case Instruction::GetElementPtr:{
          // GEP is only involved in the computation of addresses.
          // The first operand of a GEP is the pointer through which the GEP
          // instruction starts.
          // The same is true whether the first operand is an argument,
          // allocated memory, or a global variable.
          if(dyn_cast<PointerType>(I.getOperand(0)->getType())){
            unsigned NOperands = I.getNumOperands();
            if (NOperands > 6){
             dbgs() << I << " (" << &I << ")\n";
              report_fatal_error("GetelementPtr instruction with more than 6 \
                                 operands -> Need to redefine PointerToMemory");
            }
            if(NOperands == 1)
              associatedPTM = {I.getOperand(0), NULL, NULL, NULL, NULL, NULL};
            else if (NOperands == 2)
              associatedPTM = {I.getOperand(0), I.getOperand(1),
              NULL, NULL, NULL, NULL};
            else if (NOperands == 3)
              associatedPTM = {I.getOperand(0), I.getOperand(1), I.getOperand(2),
              NULL, NULL, NULL};
            else if (NOperands == 4)
              associatedPTM = {I.getOperand(0), I.getOperand(1), I.getOperand(2),
              I.getOperand(3), NULL, NULL};
            else if (NOperands == 5)
              associatedPTM = {I.getOperand(0), I.getOperand(1), I.getOperand(2),
              I.getOperand(3), I.getOperand(4), NULL};
            else if (NOperands == 6)
              associatedPTM = {I.getOperand(0), I.getOperand(1), I.getOperand(2),
                I.getOperand(3), I.getOperand(4),I.getOperand(5) };
          }else
            report_fatal_error("The first operand of a GEP instructions must \
                               always be a pointer");
      
          // Insert into the global vector of pointers to memory and return a
          // pointer to the pointer to memory in the global vector.
          associatedPTMI = {associatedPTM, 0, valueInstance};
          bool insertUse = insertUsesOfPointerToMemory(&I, associatedPTMI);
          if (insertUse)
            increaseInstructionValueInstance({&I, valueRep});
        }
        break;
  
        case Instruction::BitCast:{
          associatedPTM = {I.getOperand(0), NULL, NULL, NULL, NULL, NULL};
          associatedPTMI = {associatedPTM, 0, valueInstance};
          bool insertUse = insertUsesOfPointerToMemory(&I, associatedPTMI);
          if(insertUse)
          increaseInstructionValueInstance({&I, valueRep});
        }
        break;
        
        case Instruction::Alloca:{
          associatedPTM = {&I, NULL, NULL, NULL, NULL, NULL};
          associatedPTMI = {associatedPTM, 0, valueInstance};
          bool insertUse = insertUsesOfPointerToMemory(&I, associatedPTMI);
          printPointerToMemoryInstance(associatedPTMI);
          if(insertUse)
            increaseInstructionValueInstance({&I, valueRep});
        }
        break;
#endif
        case Instruction::Load:{
          MemoryAddress = addr;
#ifdef VALUE_ANALYSIS
          // ===================================================================
          // 1.  Get Memory Address (passed as a reference) and PointerToMemory
          //====================================================================
          if(isSpill){
            bm_type::right_iterator address_iter =
            PointerToMemoryInstanceAddressBiMap.right.find(MemoryAddress);
            if (address_iter == PointerToMemoryInstanceAddressBiMap.right.end())
              report_fatal_error("Any spill load should have an associated \
                                 pointer to memory and address.");
            else
              associatedPTMI = address_iter->second;
          }else{
            associatedPTMI = managePointerToMemory(instructionPTMI, I, valueRep,
                                                   valueInstance, MemoryAddress,
                                                   OpCode, !rep, forceAnalyze);
          }
          // ===================================================================
          // 2.  Check if is in register. A spill load should never be a
          // register, otherwise would have not been spilled
          //====================================================================
          if(RegisterFileSize > 0){
            RegisterStackDistance = registerStackReuseDistance(associatedPTMI,
                                                               I, !rep, isSpill);
            if(RegisterStackDistance < 0){
              if(lastValue)
                insertRegisterStack(associatedPTMI,  I, valueRep, !rep);
            }
            // If it was in the stack, in RegisterStackReuseDistance it has been
            // moved to the top
            if(isSpill){
              if(RegisterStackDistance >= 0){
               dbgs() << I << " (" << &I << ")\n";
                report_fatal_error("A spill load should not be in register");
              }
            }
          }
          // ===================================================================
          // 3.  If it was not in the register, insert the cache line into
          // the reuse tree
          //====================================================================
          // If it is an intrinsic load, we not only insert into the register
          // stack the value loaded from memory, but also the intermediate result
          
          // ===================================================================
          // 4.  Increase instruction value instance
          //====================================================================
          // Only if is not spill
          if(!isSpill){
            printInstructionValue(instValue);
            increaseInstructionValueInstance(instValue);
          }
#endif
          // ===================================================================
          // 5.  If it was not in the register, insert the cache line into the
          // reuse tree
          //====================================================================
          CacheLine = MemoryAddress >> BitsPerCacheLine;
          Info = getCacheLineInfo(CacheLine);
          // If not in the stack
          if (RegisterStackDistance < 0){
            Distance = ReuseDistance(Info.LastAccess, TotalInstructions, CacheLine);
            Info.LastAccess = TotalInstructions;
            insertCacheLineLastAccess(CacheLine, Info.LastAccess);
          }
        }
        break;
        case Instruction::Store:{
          MemoryAddress = addr;
#ifdef VALUE_ANALYSIS
          // ===================================================================
          // 1.  Get Memory Address and PointerToMemory
          //====================================================================
          if(isSpill){
            bm_type::right_iterator address_iter =
            PointerToMemoryInstanceAddressBiMap.right.find(MemoryAddress);
            if (address_iter == PointerToMemoryInstanceAddressBiMap.right.end())
              report_fatal_error("Any spill store should have an associated\
                                 pointer to memory and address.");
            else
              associatedPTMI = address_iter->second;
          }else{
            associatedPTMI = managePointerToMemory(instructionPTMI, I, valueRep,
                                                   valueInstance, MemoryAddress,
                                                   OpCode, !rep, forceAnalyze);
          }
          // ===================================================================
          // 2.  Check if the operand is in the stack. If not, emit spill load.
          //====================================================================
          // If the store is a spill, we don't care about the operands in the
          // registers.
          
          if(RegisterFileSize > 0){
            if(!isSpill && lastValue){
              int64_t operandPosition = 0;
              if (dyn_cast < StoreInst > (&I)) {
                operandPosition = 0;
              }else if (CallInst *CI = dyn_cast<CallInst> (&I)){
                Function * f = CI->getCalledFunction();
                operandPosition = getStoreOperandPositionIntrinsic(f->getName());
              }else{
                report_fatal_error("Store operation not found\n");
              }
              // This function takes care of triggering a load if necessary
              insertOperandsInRegisterStack(operandPosition, I, valueRep, !rep,
                                            SourceCodeLine);
            }
            // =================================================================
            // 3.  Check if the pointer to memory associated to the store is
            // in register.
            //==================================================================
#ifdef STORES_IN_REGISTER
            if(!isSpill && lastValue){
              RegisterStackDistance = registerStackReuseDistance(associatedPTMI,
                                                                 I, !rep, isSpill);
              // For memory operations, what we store in the stack is an associated
              // pointer to memory, not the value itself, to track near
              // loads/stores to the same address.
              if(RegisterStackDistance < 0)
                insertRegisterStack(associatedPTMI,  I, valueRep, !rep);
            }
#else
            RegisterStackDistance = -1; // Do not put on the stack and store.
            if(!isSpill && lastValue){
              increaseNUses(associatedPTMI);
            }
#endif
          }
          // ===================================================================
          // 4.  Increase instruction value instance
          //====================================================================
          // Only if is not spill
          if(!isSpill)
            increaseInstructionValueInstance(instValue);
#endif
          // ===================================================================
          // 5.  If it was not in the register, insert the cache line into the
          // reuse tree
          //====================================================================
          CacheLine = MemoryAddress >> BitsPerCacheLine;
          Info = getCacheLineInfo (CacheLine);
          // If not in the stack
          if (RegisterStackDistance < 0){
            Distance = ReuseDistance (Info.LastAccess, TotalInstructions, CacheLine);
            Info.LastAccess = TotalInstructions;
            insertCacheLineLastAccess(CacheLine, Info.LastAccess);
          }
        }
        break;
        
        default:
        {
#ifdef VALUE_ANALYSIS
          // ===================================================================
          // 2.  Check that operands are in the stack. If not, trigger the
          // corresponding load
          //====================================================================
          // if forceAnalyze, the instruction I is not the corresponding instruction.
          // Hence, the operands might not correspond to the operands of a real I.
          if(RegisterFileSize > 0){
            if(firstValue){
              if(forceAnalyze == false){
                unsigned NOperands = I.getNumOperands();
                for (unsigned i = 0; i < NOperands; i++)
                  insertOperandsInRegisterStack(i, I, valueRep, !rep,
                                                SourceCodeLine);
              }else{
                // If a forceAnalyze instruction, and not a load/store.
                // Check if the operands of the first rep are in the stack.
                if (CallInst *CI = dyn_cast<CallInst> (&I)){
                  Function * f = CI->getCalledFunction();
                  vector<int64_t> positions;
                  getOperandsPositionsIntrinsic(f->getName(), positions, valueRep);
                  unsigned NOperands = positions.size();
                  if(NOperands > 0){
                    for (unsigned i = 0; i < NOperands; i++)
                      insertOperandsInRegisterStack(positions.at(i), I, valueRep,
                                                    !rep, SourceCodeLine);
                  }
                }
              }
            }
            // =================================================================
            // 3.  Insert the intermediate result in the stack.
            //==================================================================
            // Create an associated PointerToMemory
            if(lastValue)
              insertIntermediateResultInRegisterStack(instructionPTMI, I,
                                                    valueRep, !rep, isSpill);
          }
          // ===================================================================
          // 4.  Increase instruction value instance
          //====================================================================
          increaseInstructionValueInstance(instValue);
#endif
        }
      }
      // ============================ SPATIAL PREFETCHER =======================
      if (SpatialPrefetcher && (OpCode == Instruction::Load ||
                                OpCode == Instruction::Store)
          && (ExtendedInstructionType > PrefetchDispatch &&
              !(ExecutionUnit[ExtendedInstructionType] == PrefetchLevel))){
      
        NextCacheLine = CacheLine + 1;
        //Get reuse distance of NextCacheLine
        Info = getCacheLineInfo (NextCacheLine);
        Distance = ReuseDistance (Info.LastAccess, TotalInstructions,
                                  NextCacheLine, true);
        NextCacheLineExtendedInstructionType =
            getMemoryInstructionType (Distance, MemoryAddress, isLoad);
        
        ExecutionResource = ExecutionUnit[NextCacheLineExtendedInstructionType];
        // Only bring data from memory to the die, not for example, from LLC to L2
        if (ExecutionResource > PrefetchTarget &&
            ExecutionResource >= PrefetchDestination) {
          Info.LastAccess = TotalInstructions;
          insertCacheLineLastAccess(NextCacheLine, Info.LastAccess);
        }
      }
    }
    //====================== END OF WARM CACHE ANALYSIS  =====================//
  }else {
    TotalInstructions++;
#ifdef VALUE_ANALYSIS
    instValue = {&I, valueRep};
    valueInstance = getInstructionValueInstance(instValue);
    instructionPTM = {&I, NULL, NULL, NULL, NULL, NULL};
    instructionPTMI = {instructionPTM, valueRep, valueInstance};
    if(OpCode ==Instruction::GetElementPtr || OpCode ==Instruction::BitCast ||
       OpCode == Instruction::Alloca){
      increaseInstructionValueInstance(instValue);
    }
#endif
    if (InstructionType >= 0 || forceAnalyze == true) {
      DEBUG (dbgs() << I << " (" << &I << ")\n");
      // Determine instruction width
      int NumOperands = I.getNumOperands ();
      if (forceAnalyze){
        // At the end I have decided to pass it as an argument directly instead
        // of obtaining the type through the operands because it depends
        // a lot on which kind of instruction is. So since we distinguish
        // across all intrinsics in Execution.cpp, it is easier to
        // decide there the operand position (or, directly, the number of vector elements).
        if(VectorCode){
          NElementsVector = VectorWidth;
          IsVectorInstruction = true;
        }else
          IsVectorInstruction = false;
      }else{
        if(isSpill){
          if(VectorCode){
            NElementsVector = VectorWidth;
            IsVectorInstruction = true;
          }else
            IsVectorInstruction = false;
        }else{
          if (NumOperands > 0) {
            int OperandPosition = (OpCode == Instruction::Store) ? 1 : 0;
            Type *Ty = I.getOperand (OperandPosition)->getType ();
            // If load/store, the operand is a pointer
            if (PointerType * PT = dyn_cast < PointerType > (Ty)) {
              if (PT->getElementType ()->getTypeID () == Type::VectorTyID) {
                IsVectorInstruction = true;
                NElementsVector = PT->getElementType ()->getVectorNumElements ();
              }
            }
            // If arithmetic instruction, we can get the vector type directly
            if (Ty->getTypeID () == Type::VectorTyID) {
              IsVectorInstruction = true;
              NElementsVector = Ty->getVectorNumElements ();
            }
          }
        }
      }
      
      if (IsVectorInstruction) {
        if(!VectorCode)
          report_fatal_error("Vector code found - Need to execute with \
                             -vector-code flag");
      }
      
      if (InstructionType >= 0 || forceAnalyze == true) {
#ifdef SOURCE_CODE_ANALYSIS
        // Get line number
        if (MDNode *N = I.getMetadata("dbg")) {  // Here I is an LLVM instruction
          DILocation Loc(N);                      // DILocation is in DebugInfo.h
          SourceCodeLine = Loc.getLineNumber();
        }else{
          // Cannot get metadata of the instruction
          report_fatal_error("Cannot get metadata of the instruction. \
                             Source code analysis requires the application to \
                             be compiled with -g flag");
        }
#endif
        //========== == Update Fetch Cycle, remove insts from buffers =========//
        // EVERY INSTRUCTION IN THE RESERVATION STATION IS ALSO IN THE REORDER BUFFER
        if (RemainingInstructionsFetch == 0 ||
            (ReorderBufferCompletionCycles.size() == (unsigned) ReorderBufferSize &&
             ReorderBufferSize != 0) || (ReservationStationIssueCycles.size() ==
                                         (unsigned) ReservationStationSize &&
                                         ReservationStationSize != 0))
          increaseInstructionFetchCycle();
      }
    }
    //==================== Handle special cases ===============================//
    switch (OpCode) {
      // Dependences through PHI nodes
      case Instruction::Br:
      case Instruction::IndirectBr:
      case Instruction::Switch:
      {
        InstructionIssueCycle =max(max (InstructionFetchCycle, BasicBlockBarrier),
                                   getInstructionValueIssueCycle (&I));
        //Iterate over the uses of the generated value
        for (User * U:I.users ())
          insertInstructionValueIssueCycle(U, InstructionIssueCycle + 1  );
      }
      
      break;
      case Instruction::PHI:{
        PHINode * PN = dyn_cast < PHINode > (&I);
        unsigned incomingValues = PN->getNumIncomingValues();
        if(incomingValues==0)
          report_fatal_error("PHI node with no incoming predecessors");
        InstructionIssueCycle =max(max (InstructionFetchCycle, BasicBlockBarrier),
                                   getInstructionValueIssueCycle (&I));
        for (unsigned i = 0; i < incomingValues; i++){
          BasicBlock * incomingBlock  = PN->getIncomingBlock(i);
          if(incomingBlock == PrevBB && dyn_cast<Constant> (PN->getIncomingValue(i))){
            InstructionIssueCycle =max(InstructionFetchCycle, BasicBlockBarrier);
          }
        }
#ifndef INTERPRETER
        // Iterate through the uses of the PHI node
        for (Value::use_iterator ui = I.use_begin(), ie = I.use_end();
             ui != ie; ++i) {
          Use &U = *ui;
          auto *i = cast<Instruction>(U.getUser());
          
          if (dyn_cast < PHINode > (*i))
            insertInstructionValueIssueCycle(*i, InstructionIssueCycle, true);
          else
            insertInstructionValueIssueCycle(*i, InstructionIssueCycle);
        }
#endif
      }
      break;
      // Dependences through the arguments of a method call
      case Instruction::Call:
        CS = CallSite (&I);
        F = CS.getCalledFunction ();
        // Loop over the arguments of the called function --- From Execution.cpp
        NumArgs = CS.arg_size();
        ArgVals.reserve (NumArgs);
        for (CallSite::arg_iterator i = CS.arg_begin(), e = CS.arg_end();
             i != e; ++i) {
          Value *V = *i;
          ArgVals.push_back(V);
        }
        InstructionIssueCycle =max(max (InstructionFetchCycle, BasicBlockBarrier),
                                   getInstructionValueIssueCycle (&I));
        break;
      
      //-------------------- Memory Dependences -------------------------------//
      case Instruction::Load:
        if (InstructionType >= 0 || forceAnalyze == true) {
          MemoryAddress = addr;
#ifdef VALUE_ANALYSIS
          set<PointerToMemoryInstance> operandsPointerToMemoryInstance;
          // ===================================================================
          // 1.  Get Memory Address and PointerToMemory
          //====================================================================
          if(isSpill){
            bm_type::right_iterator address_iter =
            PointerToMemoryInstanceAddressBiMap.right.find(MemoryAddress);
            if (address_iter == PointerToMemoryInstanceAddressBiMap.right.end())
              report_fatal_error("Any load spill should have an associated address.");
            else
              instructionPTMI = address_iter->second;
            associatedPTMI = instructionPTMI;
          }else{
            PointerToMemoryInstanceMapIterator it  =
            PointerToMemoryInstanceMap.find(instructionPTMI);
            if(it == PointerToMemoryInstanceMap.end()){
              printPointerToMemoryInstance(instructionPTMI);
              DEBUG(dbgs() << "\n");
              report_fatal_error("In analysis run every instructionPTMI should\
                                 have an associatedPTMI");
            }else
              associatedPTMI = it->second;
            
            bm_type::left_iterator associatedPTMIAddress_iter =
            PointerToMemoryInstanceAddressBiMap.left.find(associatedPTMI);
            if(associatedPTMIAddress_iter ==
               PointerToMemoryInstanceAddressBiMap.left.end())
              report_fatal_error("In analysis run every instructionPTMI should \
                                 have an associated memory address");
            else
              MemoryAddress = associatedPTMIAddress_iter->second;
          }
          if(RegisterFileSize > 0){
            // =================================================================
            // 2.  Check if is in register. A spill load should never be a
            // register, otherwise would have not been spilled
            //==================================================================
            RegisterStackDistance = registerStackReuseDistance(associatedPTMI, I,
                                                               !rep, isSpill);
            if(RegisterStackDistance < 0){
              if(lastValue)
                insertRegisterStack(associatedPTMI, I, valueRep, !rep);
            }else
              updateRegisterReuseDistanceDistribution(RegisterStackDistance);
            
            if(isSpill){
              if(RegisterStackDistance >= 0)
                report_fatal_error("A spill load should not be in register");
            }
            operandsPointerToMemoryInstance.insert(associatedPTMI);
            
            // =================================================================
            // 3.  If it is a forceAnalyze, insert also the intermediate result
            // into the stack
            //==================================================================
            // If it is an intrinsic load, we not only insert into the register
            // stack the value loaded from memory, but also the intermediate result.

            // =================================================================
            // 4. Remove operands/intermediate result from the stack if they are
            // not going to be used again
            // =================================================================
            
            if(!isSpill && lastValue ){
              set<PointerToMemoryInstance>::iterator it;
              for(it = operandsPointerToMemoryInstance.begin();
                  it != operandsPointerToMemoryInstance.end(); it++){
                if(PointerToMemoryInstanceNUsesMap[(*it)] == 0)
                  removeRegisterStack((*it));
              }
              if(lastValue){
                // Remove also the intermediate value
                if(PointerToMemoryInstanceNUsesMap[instructionPTMI] == 0)
                  removeRegisterStack(instructionPTMI);
              }
            }
            if(RegisterStackDistance >= 0 && firstValue){
              if(PointerToMemoryInstanceNUsesMap[associatedPTMI] == 0)
                removeRegisterStack(associatedPTMI);
            }
          }
          // ===================================================================
          // 5.  Increase instruction value instance
          //====================================================================
          // Only if is not spill
          if(!isSpill){
            increaseInstructionValueInstance(instValue);
            insertInstructionValueName(&I);
          }
#endif
          // ===================================================================
          // 6.  If it was not in the register, insert the cache line into the
          // reuse tree
          //====================================================================
          CacheLine = MemoryAddress >> BitsPerCacheLine;
          Info = getCacheLineInfo (CacheLine);
#ifdef DEBUG_MEMORY_TRACES
          DEBUG (dbgs() << "MemoryAddress " << MemoryAddress << "\n");
          DEBUG (dbgs() << "CacheLine " << CacheLine << "\n");
#endif
          // If not in the stack
          if (RegisterStackDistance < 0)
            Distance = ReuseDistance(Info.LastAccess, TotalInstructions,
                                      CacheLine);
          // If we load from L1 or other levels a variable that was allocated in
          // the stack, then that was a register spill.
          if (dyn_cast < AllocaInst > (I.getOperand(0))) {
            if (RegisterStackDistance< 0){
              NRegisterSpillsLoads++;
              isRegisterSpill = true;
            }
          }
          
          // ===================================================================
          // 7.  Define instruction type, execution resource and latency
          // depending on the reuse distance
          //====================================================================
          // Get the new instruction type depending on the reuse distance
          ExtendedInstructionType = getExtendedInstructionType(I, Instruction::Load,
                                                               Distance,
                                                               RegisterStackDistance);
          ExecutionResource = ExecutionUnit[ExtendedInstructionType];
          Latency = ExecutionUnitsLatency[ExecutionResource];
          
          // ===================================================================
          // 8.  Update instruction count
          //====================================================================
          if(ExtendedInstructionType != REGISTER_LOAD_NODE){
            if (IsVectorInstruction) {
              InstructionsCount[InstructionType] = InstructionsCount[InstructionType] +
              getNElementsAccess(ExecutionResource, AccessWidths[ExecutionResource],
                                 NElementsVector);
            }else
              InstructionsCount[InstructionType]++;
            }
          // ===================================================================
          // 9.  Minimum instruction issue cycle based on fetch cycle
          //====================================================================
          InstructionIssueFetchCycle = InstructionFetchCycle;
          
          // ===================================================================
          // 10.  Minimum instruction issue cycle based on when was the cache
          // line loaded
          //====================================================================
          if (ExtendedInstructionType >= L1_LOAD_NODE){
            InstructionIssueCacheLineAvailable = Info.IssueCycle;
            insertCacheLineLastAccess(CacheLine, TotalInstructions);
          }
          
          // ===================================================================
          // 11.  Minimum instruction issue cycle based on buffers availability
          //====================================================================
          if(ExtendedInstructionType != REGISTER_LOAD_NODE){
            //Calculate issue cycle depending on buffer Occupancy.
            if (LoadBufferSize > 0) {
              bool BufferFull=false;
              if(SmallBuffers){
                if (LoadBufferCompletionCycles.size() == LoadBufferSize)
                  BufferFull = true;
              }else{
                if (node_size(LoadBufferCompletionCyclesTree) == LoadBufferSize)
                  BufferFull = true;
              }
              
              if(BufferFull){
                if(SmallBuffers)
                  InstructionIssueLoadBufferAvailable =
                  findIssueCycleWhenLoadBufferIsFull();
                else
                  InstructionIssueLoadBufferAvailable =
                  findIssueCycleWhenLoadBufferTreeIsFull ();
                
                // If, moreover, the instruction has to go to the LineFillBuffer...
                if (ExtendedInstructionType >= L2_LOAD_NODE &&
                    LineFillBufferSize > 0) {
                  if (LineFillBufferCompletionCycles.size() ==
                      (unsigned) LineFillBufferSize) {
                    InstructionIssueLineFillBufferAvailable =
                    findIssueCycleWhenLineFillBufferIsFull ();
                  }
                }
              }
              else {		// If the Load Buffer is not fulll...
                if (ExtendedInstructionType >= L2_LOAD_NODE &&
                    LineFillBufferSize > 0) {	// If it has to go to the LFS...
                  if (LineFillBufferCompletionCycles.size() == LineFillBufferSize
                      || !DispatchToLineFillBufferQueue.empty()) {
                    InstructionIssueLineFillBufferAvailable =
                    findIssueCycleWhenLineFillBufferIsFull ();
                  } // Else, there is space on both
                  // Do nothing -> Instruction Issue Cycle is not affected by LB
                  // or LFB Occupancy
                } // Else, it does not have to go to LFB...
                // Do nothing (insert into LoadBuffer later, after knowing
                // IssueCycle depending on BW availability)
              }
            }
          }
          // If there is Load buffer, the instruction can be dispatched as soon as
          // the buffer is available. Otherwise, both the AGU and the execution
          // resource must be available
          
          // ===================================================================
          // 12.  Minimum instruction issue cycle based on data dependencies
          //====================================================================
          if(!isSpill)
            InstructionIssueDataDeps = getMemoryAddressIssueCycle(MemoryAddress);
          else
            InstructionIssueDataDeps = InstructionIssueFetchCycle;
          // ===================================================================
          // 13.  Minimum instruction issue cycle based on memory model
          //====================================================================
          // New for memory model
          if (ExtendedInstructionType != REGISTER_LOAD_NODE  && x86MemoryModel)
            InstructionIssueMemoryModel = LastLoadIssueCycle;

          if ( ExtendedInstructionType != REGISTER_LOAD_NODE && ARMMemoryModel) {
            // in ARM Memory model, loads cannot bypass a stores.
            if(!(ExtendedInstructionType== MEM_LOAD_NODE &&
                 cacheLineRecentlyAccessed(CacheLine)))
              InstructionIssueMemoryModel = max(LastStoreIssueCycle,
                                                LastLoadIssueCycle);
          }
          InstructionIssueCycle =
          max(max(max(max(max(max(InstructionIssueFetchCycle,
                                  InstructionIssueInOrderExecution),
                              InstructionIssueLoadBufferAvailable),
                          InstructionIssueLineFillBufferAvailable),
                      InstructionIssueDataDeps),
                  InstructionIssueCacheLineAvailable),
              InstructionIssueMemoryModel);
          // ===================================================================
          // 14.  Minimum instruction issue cycle based on resource availability
          //====================================================================
          if ((ExtendedInstructionType != REGISTER_LOAD_NODE) && ConstraintAGUs) {
            // Once all previous constraints have been satisfied, check AGU
            // availability, if any
            //First, check in dedicated AGUs.
            if (NLoadAGUs > 0) {
              InstructionIssueLoadAGUAvailable =
              findNextAvailableIssueCycle(InstructionIssueCycle,
                                          LOAD_ADDRESS_GENERATION_UNIT);
            }
            // Check in shared (loads/stores) AGUs if any, and if there is no
            // available in dedicated AGU
            if (!(NLoadAGUs > 0 &&
                  InstructionIssueLoadAGUAvailable == InstructionIssueCycle) &&
                NAGUs > 0) {
              InstructionIssueAGUAvailable =
              findNextAvailableIssueCycle(InstructionIssueCycle,
                                          ADDRESS_GENERATION_UNIT);
            }
            
            // Insert but check that there are AGUs.
            if (NLoadAGUs > 0 &&
                InstructionIssueLoadAGUAvailable >= InstructionIssueAGUAvailable){
              insertNextAvailableIssueCycle(InstructionIssueLoadAGUAvailable,
                                            LOAD_ADDRESS_GENERATION_UNIT);
            }else{
              if (NAGUs > 0) {
                insertNextAvailableIssueCycle(InstructionIssueAGUAvailable,
                                              ADDRESS_GENERATION_UNIT);
              }
            }
            
            //Store specific AGU
            if (NLoadAGUs > 0) {
              InstructionIssueCycle =
              max(InstructionIssueCycle, min(InstructionIssueAGUAvailable,
                                            InstructionIssueLoadAGUAvailable));
            }else
              InstructionIssueCycle =max(InstructionIssueCycle,
                                         InstructionIssueAGUAvailable);

          }else
            InstructionIssueAGUAvailable = InstructionIssueCycle;
          
          Port = 0;
          if (ConstraintPorts) {
            // There must be available cycle in both, the dispatch port
            // and the resource
            InstructionIssueThroughputAvailable =
            findNextAvailableIssueCyclePortAndThroughtput(InstructionIssueCycle,
                                                          ExtendedInstructionType,
                                                          getNElementsAccess(ExecutionResource,
                                                                             AccessWidths[ExecutionResource], NElementsVector));
          }else {
            InstructionIssueThroughputAvailable =
            findNextAvailableIssueCycle(InstructionIssueCycle, ExecutionResource,
                                        getNElementsAccess(ExecutionResource,
                                                           AccessWidths[ExecutionResource],
                                                           NElementsVector));
            
            if (ConstraintPorts && DispatchPort[ExtendedInstructionType].size() > 0)
              insertNextAvailableIssueCycle(InstructionIssueThroughputAvailable,
                                            ExecutionResource,
                                            getNElementsAccess(ExecutionResource,
                                                               AccessWidths[ExecutionResource],
                                                               NElementsVector),
                                          DispatchPort[ExtendedInstructionType][Port]);
            else
              insertNextAvailableIssueCycle(InstructionIssueThroughputAvailable,
                                            ExecutionResource,
                                            getNElementsAccess(ExecutionResource,
                                                               AccessWidths[ExecutionResource],
                                                               NElementsVector));
          }
          
          InstructionIssueCycle =max(InstructionIssueCycle,
                                     InstructionIssueThroughputAvailable);

          DEBUG (dbgs() << "======== Instruction Issue Cycle (fetch cycle)" <<
                 InstructionIssueFetchCycle << "========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (LB availability)" <<
                 InstructionIssueLoadBufferAvailable <<
                 "========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (LFB availability)" << InstructionIssueLineFillBufferAvailable <<
                 "========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (cache line available)" << InstructionIssueCacheLineAvailable <<
                 "========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (data deps)" <<
                 InstructionIssueDataDeps << "========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (memory model Principle) "
                 << InstructionIssueMemoryModel <<
                 "========\n");
          if(InOrderExecution)
          DEBUG (dbgs() << "======== Instruction Issue Cycle (in order execution) " <<
                 InstructionIssueInOrderExecution <<
                 "========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (AGU Availability)" <<
                 InstructionIssueAGUAvailable <<
                 "========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (Throughput Availability)" << InstructionIssueThroughputAvailable
                 << "========\n");
          DEBUG (dbgs() << "__________________Instruction Issue Cycle " <<
                 InstructionIssueCycle << "__________________\n");
          
          LastInstructionIssueCycle = max(LastInstructionIssueCycle,
                                          InstructionIssueCycle);
          insertCacheLineHistory(CacheLine);
          
          if(RegisterStackDistance < 0 )
            updateReuseDistanceDistribution(Distance, InstructionIssueCycle);
          
          if (InstructionIssueCacheLineAvailable > InstructionIssueCycle)
            InstructionIssueCycle = InstructionIssueCacheLineAvailable;
        }
        break;
      
      // The Store can execute as soon as the value being stored is calculated
      case Instruction::Store:
        if (InstructionType >= 0 || forceAnalyze == true) {
          MemoryAddress = addr;
#ifdef VALUE_ANALYSIS
          set<PointerToMemoryInstance> operandsPointerToMemoryInstance;
          // ===================================================================
          // 1.  Get Memory Address and PointerToMemory
          //====================================================================
          if(isSpill){
            bm_type::right_iterator address_iter =
            PointerToMemoryInstanceAddressBiMap.right.find(MemoryAddress);
            if (address_iter == PointerToMemoryInstanceAddressBiMap.right.end()){
              report_fatal_error("Any store spill should have an associated\
                                 address.");
            }else
              instructionPTMI = address_iter->second;
            associatedPTMI = instructionPTMI;
          }else{
            PointerToMemoryInstanceMapIterator it  =
            PointerToMemoryInstanceMap.find(instructionPTMI);
            if(it == PointerToMemoryInstanceMap.end())
              report_fatal_error("In analysis run every instructionPTMI should \
                                 have an associatedPTMI");
            else
              associatedPTMI = it->second;
            
            bm_type::left_iterator associatedPTMIAddress_iter =
            PointerToMemoryInstanceAddressBiMap.left.find(associatedPTMI);
            if (associatedPTMIAddress_iter ==
               PointerToMemoryInstanceAddressBiMap.left.end())
              report_fatal_error("In analysis run every instructionPTMI should\
                                 have an associated memory address");
            else
              MemoryAddress = associatedPTMIAddress_iter->second;
          }

          if(RegisterFileSize > 0){
            // =================================================================
            // 2.  Check if the operand is in the stack. If not, emit spill load.
            //==================================================================
            // If the store is a spill, we don't care about the operands in the
            // registers.
            if(!isSpill && lastValue){
              unsigned operandPosition = 0;
              if (dyn_cast < StoreInst > (&I))
                operandPosition = 0;
              else if (CallInst *CI = dyn_cast<CallInst> (&I)){
                Function * f = CI->getCalledFunction();
                operandPosition = getStoreOperandPositionIntrinsic(f->getName());
              }else
                report_fatal_error("Store operation not found\n");
              
              // This function takes care of triggering a load if necessary
              operandsPointerToMemoryInstance.
              insert(insertOperandsInRegisterStack(operandPosition, I,
                                                   valueRep, !rep,
                                                   SourceCodeLine));
              
            }
            // =================================================================
            // 3.  Check if the pointer to memory associated to the store is in
            // register.
            //==================================================================
#ifdef STORES_IN_REGISTER
            if(!isSpill && lastValue){
              RegisterStackDistance = registerStackReuseDistance(associatedPTMI,
                                                                 I, !rep, isSpill);
              // For memory operations, what we store in the stack is an
              // associated pointer to memory, not the value itself, to track
              // near loads/stores to the same address.
              if(RegisterStackDistance < 0)
                insertRegisterStack(associatedPTMI, I, valueRep,!rep);
              else
                updateRegisterReuseDistanceDistribution(RegisterStackDistance);
            }
#else
            RegisterStackDistance = -1; // Do not put on the stack and store.
            if(!isSpill && lastValue){
              decreaseNUses(associatedPTMI);
              // If after decreasing nUses becomes zero, remove from the stack
              // even if STORES_IN_REGISTER is defined.
              if(PointerToMemoryInstanceNUsesMap[associatedPTMI] == 0){
                // We have to check anyway if it is in the stack or not, because
                // if total number of uses is zero we may have not inserted it.
                // Use the function CheckRegisterStackReuseDistance because
                // it does not increase/decrease uses, it simply checks presence
                // in the stack
                if(checkRegisterStackReuseDistance(associatedPTMI) >= 0)
                  removeRegisterStack(associatedPTMI);
              }
            }
#endif
            // =================================================================
            // 4. Remove operands/intermediate result from the stack if they are
            // not going to be used again
            // =================================================================
            if(!isSpill){
              set<PointerToMemoryInstance>::iterator it;
              for(it = operandsPointerToMemoryInstance.begin();
                  it != operandsPointerToMemoryInstance.end(); it++){
                PointerToMemoryInstanceNUsesMapIterator nUsesit =
                PointerToMemoryInstanceNUsesMap.find((*it));
                if (nUsesit != PointerToMemoryInstanceNUsesMap.end()){
                  if(PointerToMemoryInstanceNUsesMap[(*it)] == 0){
                    removeRegisterStack((*it));
                  }
                }
              }
#ifdef STORES_IN_REGISTER
              if(lastValue){
                // Remove also the intermediate value
                PointerToMemoryInstanceNUsesMapIterator nUsesit =
                PointerToMemoryInstanceNUsesMap.find(associatedPTMI);
                if (nUsesit != PointerToMemoryInstanceNUsesMap.end()){
                  if(PointerToMemoryInstanceNUsesMap[associatedPTMI] == 0)
                    removeRegisterStack(associatedPTMI);
                }else{
                  printPointerToMemoryInstance(associatedPTMI);
                  report_fatal_error("An entry should exist for the associated \
                                     PTMI in PointerToMemoryInstanceNUsesMap");
                }
              }
#endif
            }
          }
          // ===================================================================
          // 5.  Increase instruction value instance
          //====================================================================
          // Only if is not spill
          if(!isSpill)
            increaseInstructionValueInstance(instValue);
#endif
          // ===================================================================
          // 6.  If it was not in the register, insert the cache line into the
          // reuse tree
          //====================================================================
          CacheLine = MemoryAddress >> BitsPerCacheLine;
          Info = getCacheLineInfo (CacheLine);
          // If not in the stack
          if (RegisterStackDistance < 0)
            Distance = ReuseDistance (Info.LastAccess, TotalInstructions,
                                      CacheLine);
          if(!isSpill){
            // Check whether the value is stored in a stack variable
            if (dyn_cast < AllocaInst > (I.getOperand(1))) {
              // It is a store in the register file for sure, so although
              // RegisterStackReuseDistance returns -1, it has to be inserted
              // in the register reuse stack and is a store to register. This is
              // different from a load that is not in register. If a load is not
              // in the register file, it is inserted into the register file,
              // but RegisterStackDistance is -1 because it is a L1 load
              RegisterStackDistance = 0;
            }
          }
          // ===================================================================
          // 7.  Define instruction type, execution resource and latency
          // depending on the reuse distance
          //====================================================================
          isLoad = false;
          ExtendedInstructionType =
          getExtendedInstructionType (I, Instruction::Store, Distance,
                                      RegisterStackDistance );
          ExecutionResource = ExecutionUnit[ExtendedInstructionType];
          Latency = ExecutionUnitsLatency[ExecutionResource];

          // ===================================================================
          // 8.  Update instruction count
          //====================================================================
          if(ExtendedInstructionType != REGISTER_STORE_NODE){
            if (IsVectorInstruction) {
              InstructionsCount[InstructionType] =
              InstructionsCount[InstructionType] +
              getNElementsAccess(ExecutionResource,AccessWidths[ExecutionResource],
                                 NElementsVector);
            }else
              InstructionsCount[InstructionType]++;
          }

          // ===================================================================
          // 9.  Minimum instruction issue cycle based on fetch cycle
          //====================================================================
          InstructionIssueFetchCycle = InstructionFetchCycle;
          
          // ===================================================================
          // 10.  Minimum instruction issue cycle based on cache line
          //====================================================================
          if (ExtendedInstructionType >= L1_STORE_NODE)
            InstructionIssueCacheLineAvailable = Info.IssueCycle;

          // ===================================================================
          // 11.  Minimum instruction issue cycle based on buffers availability
          //====================================================================
          if(ExtendedInstructionType != REGISTER_STORE_NODE){
            //Calculate issue cycle depending on buffer Occupancy.
            if (StoreBufferSize > 0) {
              // If the store buffer is full
              if (StoreBufferCompletionCycles.size() == StoreBufferSize) {
                InstructionIssueStoreBufferAvailable =
                findIssueCycleWhenStoreBufferIsFull();
                // If, moreover, the instruction has to go to the LineFillBuffer...
                if (ExtendedInstructionType >= L2_LOAD_NODE &&
                    LineFillBufferSize > 0) {
                  if (LineFillBufferCompletionCycles.size() ==
                      (unsigned) LineFillBufferSize) {
                    InstructionIssueLineFillBufferAvailable =
                    findIssueCycleWhenLineFillBufferIsFull();
                  }
                }
              }else {		// If the Store Buffer is not full...
                if (ExtendedInstructionType >= L2_LOAD_NODE &&
                    LineFillBufferSize > 0) {	// If it has to go to the LFS...
                  if (LineFillBufferCompletionCycles.size() == LineFillBufferSize ||
                      !DispatchToLineFillBufferQueue.empty()) {
                    InstructionIssueLineFillBufferAvailable =
                    findIssueCycleWhenLineFillBufferIsFull();
                  }
                }
              }
            }
          }
          // ===================================================================
          // 12.  Minimum instruction issue cycle based on data dependencies
          //====================================================================
          if(!isSpill)
            InstructionIssueDataDeps = getInstructionValueIssueCycle (&I);
          else
            InstructionIssueDataDeps = InstructionIssueFetchCycle;
          
          // ===================================================================
          // 13.  Minimum instruction issue cycle based on memory model
          //====================================================================
          if ( ExtendedInstructionType != REGISTER_STORE_NODE && x86MemoryModel) {
            // Writes are not reordered with other writes
            InstructionIssueMemoryModel = LastStoreIssueCycle;
            // Writes are not reordered with earlier reads
            // The memory-ordering model ensures that a store by a processor may
            // not occur before a previous load by the same processor.
            InstructionIssueMemoryModel =max(InstructionIssueMemoryModel,
                                             LastLoadIssueCycle);
          }
          // in ARM Memory model, stores cannot bypass a stores.
          if ( ExtendedInstructionType != REGISTER_STORE_NODE && ARMMemoryModel)
            InstructionIssueMemoryModel = LastLoadIssueCycle;
          
          InstructionIssueCycle = max(max(max(max (max(InstructionIssueFetchCycle,
                                                       InstructionIssueInOrderExecution),
                                                   InstructionIssueStoreBufferAvailable),
                                              InstructionIssueDataDeps),
                                          InstructionIssueCacheLineAvailable),
                                      InstructionIssueMemoryModel);
          
          // ===================================================================
          // 14.  Minimum instruction issue cycle based on resource availability
          //====================================================================
          if ((ExtendedInstructionType!= REGISTER_STORE_NODE) && ConstraintAGUs) {
            // Once all previous constraints have been satisfied, check AGU
            // availability, if any
            //First, check in dedicated AGUs.
            if (NStoreAGUs > 0) {
              InstructionIssueStoreAGUAvailable =
              findNextAvailableIssueCycle(InstructionIssueCycle,
                                          STORE_ADDRESS_GENERATION_UNIT);
            }
            // Check in shared (loads/stores) AGUs if any, and if there is no
            // available in dedicated AGU
            if (!(NStoreAGUs > 0 &&
                  InstructionIssueStoreAGUAvailable == InstructionIssueCycle) &&
                NAGUs > 0) {
              InstructionIssueAGUAvailable =
              findNextAvailableIssueCycle(InstructionIssueCycle,
                                          ADDRESS_GENERATION_UNIT);
            }
            
            // Insert but check that there are AGUs.
            if (NStoreAGUs > 0 &&
                InstructionIssueStoreAGUAvailable >= InstructionIssueAGUAvailable){
              insertNextAvailableIssueCycle(InstructionIssueStoreAGUAvailable,
                                            STORE_ADDRESS_GENERATION_UNIT);
            }else{
              if (NAGUs > 0) {
                insertNextAvailableIssueCycle(InstructionIssueAGUAvailable,
                                              ADDRESS_GENERATION_UNIT);
              }
            }
            
            //Store specific AGU
            if (NStoreAGUs > 0) {
              InstructionIssueCycle =
             max(InstructionIssueCycle, min(InstructionIssueAGUAvailable,
                                             InstructionIssueStoreAGUAvailable));
            }else
              InstructionIssueCycle =max(InstructionIssueCycle,
                                         InstructionIssueAGUAvailable);
          }else
            InstructionIssueAGUAvailable = InstructionIssueCycle;
          
          // If there is a store buffer, the dispatch cycle might be different from
          // the issue (execution) cycle.
          if (ConstraintPorts) {
            InstructionIssueThroughputAvailable =
            findNextAvailableIssueCyclePortAndThroughtput(InstructionIssueCycle,
                                                          ExtendedInstructionType,
                                                          getNElementsAccess(ExecutionResource,
                                                                             AccessWidths[ExecutionResource], NElementsVector));
          }else {
            InstructionIssueThroughputAvailable =
            findNextAvailableIssueCycle(InstructionIssueCycle, ExecutionResource,
                                        getNElementsAccess(ExecutionResource,
                                                           AccessWidths[ExecutionResource],
                                                           NElementsVector));
            
            if (DispatchPort[ExtendedInstructionType].size() > 0)
              insertNextAvailableIssueCycle(InstructionIssueThroughputAvailable,
                                            ExecutionResource,
                                            getNElementsAccess(ExecutionResource,
                                                               AccessWidths[ExecutionResource],
                                                               NElementsVector),
                                          DispatchPort[ExtendedInstructionType][Port]);
            else
              insertNextAvailableIssueCycle(InstructionIssueThroughputAvailable,
                                            ExecutionResource,
                                            getNElementsAccess(ExecutionResource,
                                                               AccessWidths[ExecutionResource],
                                                               NElementsVector));
          }
          InstructionIssueCycle = max(InstructionIssueCycle,
                                     InstructionIssueThroughputAvailable);
          
          DEBUG (dbgs() << "======== Instruction Issue Cycle (fetch cycle) " <<
                 InstructionIssueFetchCycle << " ========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (SB availability) " << InstructionIssueStoreBufferAvailable <<
                 " ========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (cache line available) "
                 << InstructionIssueCacheLineAvailable <<" ========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (data deps)" <<
                 InstructionIssueDataDeps << " ========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (memory model \
                 Principles 2 and 3) " << InstructionIssueMemoryModel << " ========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (AGU Availability) "
                 << InstructionIssueAGUAvailable << "========\n");
          DEBUG (dbgs() << "======== Instruction Issue Cycle (Throughput Availability) "
                 << InstructionIssueThroughputAvailable << "========\n");
          DEBUG (dbgs() << "__________________Instruction Issue Cycle " <<
                 InstructionIssueCycle << "__________________\n");

          LastInstructionIssueCycle = max(InstructionIssueCycle,LastInstructionIssueCycle);

          // When a cache line is written does not impact when in can be loaded again.
          if(RegisterStackDistance < 0){
            updateReuseDistanceDistribution (Distance, InstructionIssueCycle);
            insertCacheLineLastAccess(CacheLine, TotalInstructions);
          }
        }
        break;

      case Instruction::Ret:
      // Determine the uses of the returned value outside the funcion
      //(i.e., the uses of the calling function)
      // Check http://llvm.org/docs/ProgrammersManual.html for a lot
      // of info about how iterate through functions, bbs, etc.
      F = I.getParent ()->getParent ();
      InstructionIssueCycle = max(max (InstructionFetchCycle, BasicBlockBarrier),
                                 getInstructionValueIssueCycle (&I));

      for (User * U:F->users ()) {
        for (User * UI:U->users ())
          insertInstructionValueIssueCycle(UI, InstructionIssueCycle);
      }
      
      break;
      
      //-------------------------General case------------------------------//
      default:
      if (InstructionType == 0 || InstructionType == 2 || forceAnalyze == true){

#ifdef VALUE_ANALYSIS
        if(RegisterFileSize > 0){
          // ===================================================================
          // 2.  Check that operands are in the stack. If not, trigger the
          // corresponding load
          //====================================================================
          // if forceAnalyze, the instruction I is not the corresponding instruction.
          // Hence, the operands might not correspond to the operands of a real I.
          set<PointerToMemoryInstance> operandsPointerToMemoryInstance;
          if(firstValue){
            PointerToMemoryInstance operandPTMI;
            vector<int64_t> positions;
            if(forceAnalyze == false){
              unsigned NOperands = I.getNumOperands();
              for (unsigned i = 0; i < NOperands; i++)
                positions.push_back(i);
            }else{
              // If a forceAnalyze instruction, and not a load/store.
              // Check if the operands of the first rep are in the stack.
              if (CallInst *CI = dyn_cast<CallInst> (&I)){
                Function * f = CI->getCalledFunction();
                getOperandsPositionsIntrinsic(f->getName(), positions, valueRep);
              }
            }
            unsigned NOperands = positions.size();
            PointerToMemoryInstance emptyPTMI =
            {{NULL,NULL,NULL,NULL, NULL, NULL},0,-1};
            // Here we don't have to check if NOperands is 0, because if it is
            // zero the for loop is never executed.
            for (unsigned i = 0; i < NOperands; i++){
              //	if(positions.at(i)>= 0){
              operandPTMI = insertOperandsInRegisterStack(positions.at(i), I,
                                                          valueRep, !rep,
                                                          SourceCodeLine);
              // Operand is -1 if operand is a constant.
              if(!(operandPTMI == emptyPTMI))
                operandsPointerToMemoryInstance.insert(operandPTMI);
            }
          }
          // ===================================================================
          // 3.  Insert the intermediate result in the stack.
          //====================================================================
          // Create an associated PointerToMemory
          if(lastValue)
            insertIntermediateResultInRegisterStack(instructionPTMI, I, valueRep,
                                                  !rep, isSpill);

          // ===================================================================
          // 4. Remove operands/intermediate result from the stack if they are
          // not going to be used again
          // ===================================================================
          set<PointerToMemoryInstance>::iterator it;
          for(it = operandsPointerToMemoryInstance.begin();
              it != operandsPointerToMemoryInstance.end(); it++){
            if(PointerToMemoryInstanceNUsesMap[(*it)] == 0)
              removeRegisterStack((*it));
          }
          if(lastValue){
            // Remove also the intermediate value
            if(PointerToMemoryInstanceNUsesMap[instructionPTMI] == 0)
              removeRegisterStack(instructionPTMI);
          }
        }
        
        // =====================================================================
        // 5.  Increase Instruction Value Instance
        //======================================================================
        // Here it is never a spill
        increaseInstructionValueInstance({&I, valueRep});
#endif
        // =====================================================================
        // 5.  Define instruction type, execution resource and latency depending
        // on the reuse distance
        //======================================================================
        ExtendedInstructionType = getExtendedInstructionType (I, OpCode);
        Latency = ExecutionUnitsLatency[ExtendedInstructionType];
  
        // =====================================================================
        // 6.  Update instruction count
        //======================================================================
        if(InstructionType == 0){
          if (IsVectorInstruction) {
            InstructionsCount[InstructionType] = InstructionsCount[InstructionType] +
            getNElementsAccess(ExecutionResource, AccessWidths[ExecutionResource],
                               NElementsVector);
          }
          else
            InstructionsCount[InstructionType]++;

          if (InstructionIssueCycle > OriginalInstructionIssueCycle)
            NInstructionsStalled[ExtendedInstructionType]++;
        }
        
        // =====================================================================
        // 7.  Issue cycle based on fetch cycle and data dependencies
        //======================================================================
        OriginalInstructionIssueCycle = getInstructionValueIssueCycle (&I);
        if(InOrderExecution)
          InstructionIssueInOrderExecution = LastInstructionIssueCycle;
        InstructionIssueCycle = max(max(max(InstructionFetchCycle,
                                           InstructionIssueInOrderExecution),
                                       BasicBlockBarrier),
                                   OriginalInstructionIssueCycle);
        
        // =====================================================================
        // 8.  Issue cycle based on resource availability
        //======================================================================
   
        InstructionIssueThroughputAvailable =
        findNextAvailableIssueCyclePortAndThroughtput(InstructionIssueCycle,
                                                      ExtendedInstructionType,
                                                      getNElementsAccess(ExecutionResource,
                                                                         AccessWidths[ExecutionResource],NElementsVector));
        
        InstructionIssueCycle =max(InstructionIssueCycle,
                                   InstructionIssueThroughputAvailable);
        
        DEBUG (dbgs() << "========Original Instruction Issue Cycle (data deps)" <<
               OriginalInstructionIssueCycle << "========\n");
        DEBUG (dbgs() << "========Original Instruction Issue Cycle (fetch cycle) " <<
               InstructionFetchCycle <<
               "========\n");
        DEBUG (dbgs() << "__________________Instruction Issue Cycle " <<
               InstructionIssueCycle << "__________________\n");
        
        LastInstructionIssueCycle = max(LastInstructionIssueCycle,
                                        InstructionIssueCycle);
      }
      break;
    }
    
    if (InstructionType >= 0 || forceAnalyze == true) {
#ifdef SOURCE_CODE_ANALYSIS
      SourceCodeLineOperations[SourceCodeLine].
      insert(ExecutionUnit[ExtendedInstructionType]);
#endif
      
      uint64_t NewInstructionIssueCycle = InstructionIssueCycle;
      if (x86MemoryModel || ARMMemoryModel) {
        // Accesses to registers are excluded from the memory model
        if (OpCode == Instruction::Load &&
            ExtendedInstructionType >= L1_LOAD_NODE) {
          LastLoadIssueCycle = NewInstructionIssueCycle;
        }else {
          if (OpCode == Instruction::Store &&
              ExtendedInstructionType >= L1_STORE_NODE)
            LastStoreIssueCycle = NewInstructionIssueCycle;
        }
      }
      
      // A load can execute as soon as all its operands are available, i.e., all
      // the values that are being loaded. If the same value is loaded again,
      // without  having been used in between (Read After Read dependence), then
      // the next load can only be issued after the first one has finished.
      // This only applies to memory accesses > L1. If we access a cache line at
      // cycle  X which is in L3, e.g., it has a latency of 30. The next  time
      // this cache line is accessed it is in L1, but it is inconsistent to
      // assume that it can be  loaded also at cycle X and have a latency of 4 cycles.
      ExecutionResource = ExecutionUnit[ExtendedInstructionType];
      if (OpCode == Instruction::Load && RARDependences) {
        if(RegisterFileSize != 0 && ExtendedInstructionType == L1_LOAD_NODE){
          insertMemoryAddressIssueCycle(MemoryAddress,
                                        NewInstructionIssueCycle + Latency);
        }else{
          if(ExtendedInstructionType > L1_LOAD_NODE
             && ExecutionUnitsLatency[ExecutionResource] >
             ExecutionUnitsLatency[L1_LOAD_CHANNEL]){
            Info = getCacheLineInfo (CacheLine);
            Info.IssueCycle = NewInstructionIssueCycle + Latency;
            insertCacheLineInfo(CacheLine, Info);
            insertMemoryAddressIssueCycle(MemoryAddress,
                                          NewInstructionIssueCycle + Latency);
          }
        }
      }
      if (OpCode == Instruction::Store){
        if(RegisterFileSize!=0){
          insertMemoryAddressIssueCycle(MemoryAddress, NewInstructionIssueCycle);
        }else{
          if( ExtendedInstructionType > L1_STORE_NODE
             && ExecutionUnitsLatency[ExecutionResource] >
             ExecutionUnitsLatency[L1_LOAD_CHANNEL]){
            Info = getCacheLineInfo (CacheLine);
            Info.IssueCycle = NewInstructionIssueCycle + Latency;
            insertCacheLineInfo(CacheLine, Info);
            insertMemoryAddressIssueCycle(MemoryAddress, NewInstructionIssueCycle);
          }
        }
      }
      
      // =========================== SPATIAL PREFETCHER ========================
      if (SpatialPrefetcher && (OpCode == Instruction::Load ||
                                OpCode == Instruction::Store) &&
          ExtendedInstructionType > PrefetchDispatch &&
          !(ExecutionUnit[ExtendedInstructionType] == PrefetchLevel)) {
          NextCacheLine = CacheLine + 1;
        
        //Get reuse distance of NextCacheLine
        Info = getCacheLineInfo(NextCacheLine);
        Distance = ReuseDistance(Info.LastAccess, TotalInstructions,
                                 NextCacheLine, true);
        NextCacheLineExtendedInstructionType =
        getMemoryInstructionType(Distance, MemoryAddress,isLoad);
        ExecutionResource = ExecutionUnit[NextCacheLineExtendedInstructionType];
        LatencyPrefetch = ExecutionUnitsLatency[ExecutionResource] -
        ExecutionUnitsLatency[PrefetchDestination];

        // Prefetch every time there is a miss (not necessarily an access to
        // memory), but only if the prefetched data is in memory.
        if (ExecutionResource > PrefetchTarget &&
            ExecutionResource >= PrefetchDestination) {
          InstructionsCountExtended[NextCacheLineExtendedInstructionType]++;
          if (IsVectorInstruction) {
            InstructionsCount[InstructionType] = InstructionsCount[InstructionType] +
            getNElementsAccess(ExecutionResource, AccessWidths[ExecutionResource],
                               NElementsVector);
          }else
            report_fatal_error("ERROR with prefetcher");

          // UpdateReuseDistribution
          NextCacheLineIssueCycle = findNextAvailableIssueCycle(NewInstructionIssueCycle,
                                                                ExecutionResource);
          
          updateReuseDistanceDistribution(Distance, NextCacheLineIssueCycle);
          insertNextAvailableIssueCycle(NextCacheLineIssueCycle,
                                        ExecutionResource, 1, 0, true);
          Info.IssueCycle = NextCacheLineIssueCycle + LatencyPrefetch;
          Info.LastAccess = TotalInstructions;
          insertCacheLineInfo(NextCacheLine, Info);
        }
      }
      
      //Iterate over the uses of the generated value (except for GetElementPtr)
      if (OpCode != Instruction::GetElementPtr && !isSpill ) {
#if LLVM_VERSION_MAJOR<4
#ifdef PRINT_DEPENDENCIES
        if (valueInstance < 0)
          dbgs()<<  &I << ".0" << " ";
        else
          dbgs()<<  &I << "." << valueInstance-1 << " ";
#endif
        for (Value::use_iterator i = I.use_begin(), ie = I.use_end(); i != ie; ++i) {
          
#ifdef PRINT_DEPENDENCIES
          // If the use is a PHI node:
          // Print "PHINODE" followed by the PHINODE
          // The print N_USES followed by the uses
          // We will process later this information
          InstructionValue instValueUse = {*i, valueRep};
          int64_t valueUseInstance = getInstructionValueInstance(instValueUse);
          if(dyn_cast <PHINode> (*i)){
            dbgs() << "PHINODE ";
            if (valueUseInstance < 0)
              dbgs()<<  *i << ".0" << " ";
            else
              dbgs()<<  *i << "." << valueUseInstance << " ";
            unsigned nUsesPhiNode = 0;
            for (Value::use_iterator ii = (*i)->use_begin(), iie = (*i)->use_end();
                 ii != iie; ++ii)
              nUsesPhiNode++;
            for (Value::use_iterator ii = (*i)->use_begin(), iie = (*i)->use_end();
                 ii != iie; ++ii) {
              InstructionValue instValuePHIUse = {*ii, valueRep};
              int64_t valuePHIUseInstance = getInstructionValueInstance(instValuePHIUse);
              if (valuePHIUseInstance< 0)
                dbgs()<<  *ii << ".0" << " ";
              else
                dbgs()<<  *ii << "." << valuePHIUseInstance << " ";
            }
          }else{
            if (valueUseInstance < 0)
              dbgs()<<  *i << ".0" << " ";
            else
              dbgs()<<  *i << "." << valueUseInstance << " ";
          }
#endif
          if (dyn_cast < PHINode > (*i))
            insertInstructionValueIssueCycle(*i,
                                             NewInstructionIssueCycle + Latency,
                                             true);

          else
            insertInstructionValueIssueCycle(*i,
                                             NewInstructionIssueCycle + Latency);
          
          if (dyn_cast < CallInst > (*i)) {
            CS = CallSite (*i);
            F = CS.getCalledFunction ();
            // Loop over the arguments of the called function
            NumArgs = CS.arg_size();
            ArgVals.reserve (NumArgs);
            for (CallSite::arg_iterator j = CS.arg_begin(), e = CS.arg_end();
                 j != e; ++j) {
              Value *V = *j;
              ArgVals.push_back(V);
            }
            
            // Make sure it is an LLVM-well-defined function
            if (static_cast < Function * >(F)) {
              for (Function::arg_iterator AI = F->arg_begin(), E = F->arg_end();
                   AI != E; ++AI, ++k) {
                // Iterate through the uses of the argument and check which one
                // is equal to current instruction
                if (ArgVals[k] == &I) {
                  for (Value::use_iterator vi = (*AI).use_begin(),
                       vie = (*AI).use_end();  vi != vie; ++vi) {
                    insertInstructionValueIssueCycle(*vi,
                                                     NewInstructionIssueCycle + Latency);
                  }
                }
              }
            }
          }
        }
#ifdef PRINT_DEPENDENCIES
       dbgs() <<"\n";
#endif
#else
        for (User * U:I.users ()) {
          if (dyn_cast < PHINode > (U))
            insertInstructionValueIssueCycle(U,
                                             NewInstructionIssueCycle + Latency,
                                             true);
          else
            insertInstructionValueIssueCycle(U, NewInstructionIssueCycle + Latency);
          
          if (dyn_cast < CallInst > (U)) {
            CS = CallSite (U);
            F = CS.getCalledFunction ();
            // Loop over the arguments of the called function
            NumArgs = CS.arg_size();
            ArgVals.reserve (NumArgs);
            for (CallSite::arg_iterator j = CS.arg_begin(), e = CS.arg_end();
                 j != e; ++j) {
              Value *V = *j;
              ArgVals.push_back(V);
            }
            // Make sure it is an LLVM-well-defined funciton
            if (static_cast < Function * >(F)) {
              for (Function::arg_iterator AI = F->arg_begin(), E = F->arg_end();
                   AI != E; ++AI, ++k) {
                if (ArgVals[k] == &I) {
                  for (Value::use_iterator vi = (*AI).use_begin(),
                       vie = (*AI).use_end(); vi != vie; ++vi) {
                    insertInstructionValueIssueCycle(*vi,
                                                     NewInstructionIssueCycle + Latency);
                  }
                }
              }
            }
          }
        }
#endif
      }
      
      if (forceAnalyze == true && !isSpill) {
        if (OpCode == Instruction::Store)
          insertInstructionValueIssueCycle(&I, NewInstructionIssueCycle);
        else {
          // If it is not a store, we force uses only if it is not lastValue.
          if(!lastValue){
            insertInstructionValueIssueCycle(&I, NewInstructionIssueCycle + Latency);
          }
        }
      }
      
      //===================== Update Parallelism Distribution ================//
#ifdef ILP_DISTRIBUTION
      if (InstructionType >= 0) {	// Consider only selected instructions types
        if (ParallelismDistribution.empty()) {
          for (uint i = 0; i < Latency; i++) {
            vector < unsigned >v;
            ParallelismDistribution.push_back(v);
            // TODO: Instead of 2, parameter with number of types of instructions
            for (unsigned j = 0; j < 2; j++)
              ParallelismDistribution[InstructionIssueCycle + i].push_back(0);
          }
        }
        else {
          uint DistributionSize = ParallelismDistribution.size();
          if (DistributionSize <= InstructionIssueCycle + Latency - 1) {
            uint extraSlots =max(InstructionIssueCycle + Latency - DistributionSize,
                                 (uint64_t) 1);
            for (uint i = 0; i < extraSlots; i++) {
              vector < unsigned >v;
              ParallelismDistribution.push_back(v);
              for (uint j = 0; j < 2; j++)
                ParallelismDistribution[DistributionSize + i].push_back(0);
            }
          }
        }
        for(uint i = 0; i < Latency; i++)
        ParallelismDistribution[InstructionIssueCycle + i][InstructionType]++;
      }
#endif
      
      //When InstructionFetchBandwidth is INF, remaining instructions to fetch
      // is -1, but still load and stores must be inserted into the OOO buffers
      if(ExtendedInstructionType != REGISTER_LOAD_NODE &&
         ExtendedInstructionType != REGISTER_STORE_NODE  && isRegisterSpill==false){
        if (RemainingInstructionsFetch > 0 || RemainingInstructionsFetch == INF) {
          if (RemainingInstructionsFetch > 0)
            RemainingInstructionsFetch--;
          uint64_t CycleInsertReservationStation = 0;
          if (OpCode == Instruction::Load) {
            // If LB is not full, they go directly to the LB and to the RS
            // If LB is INF, this comparison is false. But still
            // we need to check wether RS is INF
            
            bool BufferFull=false;
            if(SmallBuffers){
              if (LoadBufferCompletionCycles.size() == LoadBufferSize)
                BufferFull = true;
            }else{
              if (node_size(LoadBufferCompletionCyclesTree) == LoadBufferSize)
                BufferFull = true;
            }
            if (LoadBufferSize > 0 && BufferFull) {
              // Put in the reservation station, but only if RS exists
              if(SmallBuffers)
                CycleInsertReservationStation = findIssueCycleWhenLoadBufferIsFull();
              else
                CycleInsertReservationStation =
              findIssueCycleWhenLoadBufferTreeIsFull ();
              
              if(InstructionIssueLoadBufferAvailable != CycleInsertReservationStation)
                report_fatal_error("InstructionIssueLoadBufferAvailable != \
                                   CycleInsertReservationStation");
              ReservationStationIssueCycles.push_back(CycleInsertReservationStation);
              
              //Put in the DispatchToLoadBufferQueue
              if(SmallBuffers){
                InstructionDispatchInfo DispathInfo;
                DispathInfo.IssueCycle = CycleInsertReservationStation;
                DispathInfo.CompletionCycle = NewInstructionIssueCycle+Latency;
                DispatchToLoadBufferQueue.push_back(DispathInfo);
                
              }else{
                if (DispatchToLoadBufferQueueTree == NULL) {
                  MaxDispatchToLoadBufferQueueTree = CycleInsertReservationStation;
                }else {
                  MaxDispatchToLoadBufferQueueTree = max(MaxDispatchToLoadBufferQueueTree, CycleInsertReservationStation);
                }
                DispatchToLoadBufferQueueTree =
                insert_node(NewInstructionIssueCycle + Latency,
                            MaxDispatchToLoadBufferQueueTree,
                            DispatchToLoadBufferQueueTree);
              }
              
#ifdef SOURCE_CODE_ANALYSIS
              SourceCodeLineOperations[SourceCodeLine].insert (LB_STALL);
              unsigned TreeChunk = 0;
              for (uint64_t i = InstructionFetchCycle;
                   i < CycleInsertReservationStation; i++) {
                TreeChunk = getTreeChunk(i);
                FullOccupancyCyclesTree[TreeChunk].
                insert_source_code_line (i, SourceCodeLine, LB_STALL);
              }
#endif
              // If, moreover, the instruction has to go to the LineFillBuffer...
              if (ExtendedInstructionType >= L2_LOAD_NODE && LineFillBufferSize > 0) {
                if (LineFillBufferCompletionCycles.size() == LineFillBufferSize ||
                    !DispatchToLineFillBufferQueue.empty()) {
                  InstructionDispatchInfo DispathInfo;
                  DispathInfo.IssueCycle = findIssueCycleWhenLineFillBufferIsFull ();
                  DispathInfo.CompletionCycle = NewInstructionIssueCycle + Latency;
                  DispatchToLineFillBufferQueue.push_back(DispathInfo);
                  
#ifdef SOURCE_CODE_ANALYSIS
                  SourceCodeLineOperations[SourceCodeLine].insert (LFB_STALL);
                  TreeChunk = 0;
                  for (uint64_t i = InstructionFetchCycle;
                       i < CycleInsertReservationStation; i++) {
                    TreeChunk = getTreeChunk(i);
                    SourceCodeLineOperations[SourceCodeLine].insert (LFB_STALL);
                    FullOccupancyCyclesTree[TreeChunk].
                    insert_source_code_line (i, SourceCodeLine,LFB_STALL);
                  }
#endif
                }else	// There is space on both
                  LineFillBufferCompletionCycles.push_back(NewInstructionIssueCycle +
                                                           Latency);
              }
            }else{
              bool BufferFull=true;
              if(SmallBuffers){
                if (LoadBufferCompletionCycles.size() != LoadBufferSize)
                  BufferFull = false;
              }else{
                if (node_size(LoadBufferCompletionCyclesTree) != LoadBufferSize)
                  BufferFull = false;
              }
              //If LB is not full
              if (LoadBufferSize > 0 && BufferFull==false) {
                //Insert into LB
                if(SmallBuffers)
                  LoadBufferCompletionCycles.
                push_back(NewInstructionIssueCycle+Latency);
                else{
                  if (node_size(LoadBufferCompletionCyclesTree) == 0) {
                    MinLoadBuffer = NewInstructionIssueCycle + Latency;
                  }else {
                    MinLoadBuffer = min (MinLoadBuffer,
                                         NewInstructionIssueCycle + Latency);
                  }
                  LoadBufferCompletionCyclesTree =
                  insert_node(NewInstructionIssueCycle + Latency,
                              LoadBufferCompletionCyclesTree);
                }
                if (ExtendedInstructionType >= L2_LOAD_NODE &&
                    LineFillBufferSize != 0) {
                  if (LineFillBufferCompletionCycles.size() == LineFillBufferSize
                      || !DispatchToLineFillBufferQueue.empty()) {
                    InstructionDispatchInfo DispathInfo;
                    DispathInfo.IssueCycle = findIssueCycleWhenLineFillBufferIsFull();
                    DispathInfo.CompletionCycle = NewInstructionIssueCycle + Latency;
                    DispatchToLineFillBufferQueue.push_back(DispathInfo);
                  }else {		// There is space on both
                    LineFillBufferCompletionCycles.push_back(NewInstructionIssueCycle +
                                                             Latency);
                  }
                }
              } else {
                //If LB is zero.... Insert into into RS, if it exists
                if (ReservationStationSize > 0) {
                  CycleInsertReservationStation = NewInstructionIssueCycle;
                  ReservationStationIssueCycles.
                  push_back(CycleInsertReservationStation);
                }
              }
            }
          }else{
            if (OpCode == Instruction::Store) {
              if (StoreBufferCompletionCycles.size() == StoreBufferSize &&
                  StoreBufferSize > 0) {
                CycleInsertReservationStation = findIssueCycleWhenStoreBufferIsFull ();
                ReservationStationIssueCycles.push_back(CycleInsertReservationStation);
                InstructionDispatchInfo DispathInfo;
                DispathInfo.IssueCycle = findIssueCycleWhenStoreBufferIsFull ();
                DispathInfo.CompletionCycle = NewInstructionIssueCycle + Latency;
                DispatchToStoreBufferQueue.push_back(DispathInfo);
                
#ifdef SOURCE_CODE_ANALYSIS
                SourceCodeLineOperations[SourceCodeLine].insert (SB_STALL);
                unsigned TreeChunk = 0;
                for (uint64_t i = InstructionFetchCycle;
                     i < CycleInsertReservationStation; i++) {
                  TreeChunk = getTreeChunk(i);
                FullOccupancyCyclesTree[TreeChunk].
                  insert_source_code_line(i,  SourceCodeLine, SB_STALL);
                }
#endif
              }else {		// If it is not full
                if (StoreBufferCompletionCycles.size() != StoreBufferSize &&
                    StoreBufferSize > 0) {
                  StoreBufferCompletionCycles.
                  push_back(NewInstructionIssueCycle + Latency);
                }else {
                  if (ReservationStationSize > 0) {
                    CycleInsertReservationStation = NewInstructionIssueCycle;
                    ReservationStationIssueCycles.
                    push_back(CycleInsertReservationStation);
                  }
                }
              }
            }else {
              // Not load nor store -> Insert into RS if its size is > -1
              if (ReservationStationSize > 0) {
                CycleInsertReservationStation = NewInstructionIssueCycle;
                ReservationStationIssueCycles.push_back(CycleInsertReservationStation);
              }
            }
          }
          
          if (InOrderExecution)
            ReorderBufferCompletionCycles.push_back(NewInstructionIssueCycle+Latency);
          else {
            if (ReorderBufferSize > 0)
              ReorderBufferCompletionCycles.push_back(NewInstructionIssueCycle + Latency);
          }
          if(isSpill){
            //  Update Fetch Cycle, remove insts from buffers
            // EVERY INSTRUCTION IN THE RESERVATION STATION IS ALSO IN THE
            // REORDER BUFFER
            if (RemainingInstructionsFetch == 0 ||
                (ReorderBufferCompletionCycles.size() == (unsigned)ReorderBufferSize &&
                 ReorderBufferSize != 0)
                || (ReservationStationIssueCycles.size() ==
                    (unsigned) ReservationStationSize &&
                    ReservationStationSize != 0)) {
              increaseInstructionFetchCycle();
            }
          }
        }
      }
    }
  }
  if (forceAnalyze && (OpCode == Instruction::FDiv) &&
      Microarchitecture.compare("ARM-CORTEX-A9") == 0){
    ExecutionUnitsLatency[FP32_DIVIDER] = PrvFP32DivLat;
    ExecutionUnitsLatency[FP64_DIVIDER] = PrvFP64DivLat;
    ExecutionUnitsThroughput[FP32_DIVIDER] = PrvFP32DivTh;
    ExecutionUnitsThroughput[FP64_DIVIDER] = PrvFP64DivTh;
  }

#ifdef EFF_TBV
}
#else
}
#endif


//===----------------------------------------------------------------------===//
//                  Routines for printing statistics
//===----------------------------------------------------------------------===//

void
DynamicAnalysis::dumpList(std::list< double > const & l, string const & filename)
{
  ofstream outfile;
  outfile.open(filename.c_str(), ios_base::out);
  std::list< double >::const_iterator it = l.begin();
  outfile << *it;
  for (++it; it != l.end(); ++it)
  outfile << " " << *it;
  
  outfile.close();
}


void
DynamicAnalysis::printHeaderStat (string Header)
{
 dbgs() << "//===------------------------------------------------------===//\n";
 dbgs() << "//               " << Header << "                               \n";
 dbgs() << "//===------------------------------------------------------===//\n";
}


void
DynamicAnalysis::finishAnalysisContechSimplified ()
{
  unsigned long long TotalSpan = 0;
  uint64_t TotalStallSpan = 0;
  float Performance = 0;
  
  vector < int >allCompResources;
  vector < int >compResources;
  vector < int >movResources;
  vector < int >memResources;
  
  vector < uint64_t > ResourcesSpan (NExecutionUnits + NPorts + NAGUs +
                                     NLoadAGUs + NStoreAGUs + NBuffers);
 
  vector < uint64_t > ResourcesTotalStallSpanVector (NExecutionUnits + NPorts +
                                                     NAGUs + NLoadAGUs +
                                                     NStoreAGUs + NBuffers);
  vector < vector < uint64_t > >ResourcesResourcesNoStallSpanVector
  (NExecutionUnits, vector < uint64_t > (NExecutionUnits));
  vector < vector < uint64_t > >
  ResourcesResourcesSpanVector(NExecutionUnits, vector < uint64_t > (NExecutionUnits));
  
  vector < vector < uint64_t > >
  ResourcesStallSpanVector (NExecutionUnits, vector < uint64_t > (NExecutionUnits));
  
  vector < vector < uint64_t > >
  StallStallSpanVector (NBuffers, vector < uint64_t > (NBuffers));
  
  vector < vector < uint64_t > >
  ResourcesIssueStallSpanVector (NExecutionUnits,vector < uint64_t > (NBuffers));
  
  list< double > cycleList, flopList, vectorizationEfficiency,
  vectorizationGapEfficiency, vectorizationVickyEfficiency,
  vectorizationEfficiencyPortsIntel,  vectorizationEfficiencyPortsIntelMem,  vectorizationEfficiencyPortsIntelMemPerType;
  
#ifdef PRINT_DEPENDENCIES
  printInstructionValueNames();
#endif
  
  bool BufferNonEmpty=false;
  if(SmallBuffers){
    if (LoadBufferCompletionCycles.size() != 0)
      BufferNonEmpty = true;
  }else{
    if (node_size(LoadBufferCompletionCyclesTree) != 0)
      BufferNonEmpty = true;
  }
  
  // Increase FetchCycle until all buffers are empty
  while (ReservationStationIssueCycles.size() != 0
         || ReorderBufferCompletionCycles.size() != 0
         || BufferNonEmpty || StoreBufferCompletionCycles.size() != 0
         || LineFillBufferCompletionCycles.size() != 0) {
    // In IncreaseInstructionFetchCycle(), InstructionFetchCycle only increases
    // when RS or ROB are full. But in this case, they may not get full, but we
    // just want to empty them.
    // We don't increase fetch cycle here anymore because it is increased in the
    // function IncreaseInstructionFetchCycle() by setting the argument to true
    //  InstructionFetchCycle++;
    increaseInstructionFetchCycle(true);
    
    BufferNonEmpty=false;
    //Update Buffer Size
    if(SmallBuffers){
      if (LoadBufferCompletionCycles.size() != 0)
        BufferNonEmpty = true;
    }else{
      if (node_size(LoadBufferCompletionCyclesTree) != 0)
        BufferNonEmpty = true;
    }
  }
  
  for (unsigned i = 0; i < NArithmeticNodes+NMovNodes; i++)
    allCompResources.push_back(i);
  
  for (unsigned i = 0; i < NArithmeticNodes; i++)
    compResources.push_back(i);
  
  unsigned nArithmeticInstructionCount = InstructionsCountExtended[FP32_ADD_NODE] +
  InstructionsCountExtended[FP32_MUL_NODE] +
  2*InstructionsCountExtended[FP32_FMA_NODE] +
  InstructionsCountExtended[FP32_DIV_NODE]+
  InstructionsCountExtended[FP64_ADD_NODE] +
  InstructionsCountExtended[FP64_MUL_NODE] +
  2*InstructionsCountExtended[FP64_FMA_NODE] +
  InstructionsCountExtended[FP64_DIV_NODE];
  
  for (unsigned i = NArithmeticNodes; i < NArithmeticNodes + NMovNodes; i++)
    movResources.push_back(i);
  
  for (unsigned i = NArithmeticNodes + NMovNodes;
       i < NArithmeticNodes + NMovNodes + NMemNodes; i++)
    memResources.push_back(i);
  
  for (unsigned j = 0; j < NExecutionUnits + NAGUs + NPorts + NBuffers; j++) {
    LastIssueCycleVector.push_back(getLastIssueCycle(j));
    // If all buffers sizes are infinity, or a buffer does not get full, we don't
    // increment in the previous while loop InstructionFetchCycle. So this check
    // only makes sense when RS or ROB have limited size(because they hold
    // all type of instructions until they are issued)
    if (InstructionFetchCycle != 0 &&
        LastIssueCycleVector[j] > InstructionFetchCycle &&
        ReservationStationIssueCycles.size() != 0 &&
        ReorderBufferCompletionCycles.size() != 0) {
      report_fatal_error("LastIssueCycle > InstructionFetchCycle\n");
    }
  }
  
  computeAvailableTreeFinal ();
  
  for (unsigned i = 0; i < NTotalResources; i++) {
    vector < int >tv;
    if (LastIssueCycleVector[i] > LastIssueCycleFinal)
      LastIssueCycleFinal = LastIssueCycleVector[i];
    
    if (InstructionsCountExtended[i] != 0) {
      tv.push_back(i);
      ResourcesSpan[i] = calculateGroupSpanFinal(tv);
      if (ExecutionUnitsLatency[i]!=0)
        IssueSpan[i] = calculateIssueSpanFinal (tv);
      else
        IssueSpan[i] = 0;
    }
  }
  
  for (unsigned i = 0; i < NTotalResources; i++) {
    if (InstructionsCountExtended[i] != 0 && ExecutionUnitsLatency[i]!=0)
      LatencyOnlySpan[i] = calculateLatencyOnlySpanFinal(i);
  }
  
  // Calculate total span and resources span with stalls
  unsigned long long InstructionLatency = 0;
  uint64_t LastCycle = 0;
  
  for (unsigned j = 0; j < NExecutionUnits; j++) {
    // If there are instructions of this type
    if (InstructionsCountExtended[j] > 0) {
      InstructionLatency = ExecutionUnitsLatency[j];
      LastCycle = LastIssueCycleVector[j];
      TotalSpan =max(LastCycle + InstructionLatency, TotalSpan);
    }
  }
  
  {
    for (unsigned i = 0; i < NExecutionUnits; i++) {
      vector < int >tv;
      if (InstructionsCountExtended[i] != 0) {
        tv.push_back(i);
        for (uint j = RS_STALL; j <= LFB_STALL; j++) {
          if (InstructionsCountExtended[j] != 0)
            tv.push_back(j);
        }
        ResourcesTotalStallSpanVector[i] = getGroupSpanFinal(tv);
      }
    }
  }
  
  //====================== Report only performance ==========================//
  if (ReportOnlyPerformance) {
    dbgs() << "TOTAL FLOPS" << "\t" << nArithmeticInstructionCount <<
    "\t\t" << calculateGroupSpanFinal(compResources) << " \n";
    dbgs() << "TOTAL MOPS" << "\t" << InstructionsCount[1] << "\t\t" <<
    calculateGroupSpanFinal(memResources) << " \n";
    dbgs() << "TOTAL" << "\t\t" << InstructionsCount[0] + InstructionsCount[1] <<
    "\t\t" << TotalSpan << " \n";
    Performance = (float) nArithmeticInstructionCount / ((float) TotalSpan);
    fprintf (stderr, "PERFORMANCE %1.3f\n", Performance);
    return;
  }
  
  //====================== Print ILP distribution ============================//
#ifdef ILP_DISTRIBUTION
  printHeaderStat ("ILP distribution");
  printILPDistribuion(TotalSpan);
#endif
  
  //=================== Reuse Distance Distriburion ==========================//
  printHeaderStat ("Register Reuse Distance distribution");
  map < int, int >::iterator ReuseDistanceMapIt;
  for (ReuseDistanceMapIt = RegisterReuseDistanceDistribution.begin();
       ReuseDistanceMapIt != RegisterReuseDistanceDistribution.end();
       ++ReuseDistanceMapIt) {
    dbgs() << ReuseDistanceMapIt->first << " " <<
    ReuseDistanceMapIt->second << "\n";
  }
  
  printHeaderStat ("Reuse Distance distribution (x axis: \
                   numer of distinct cache lines)");
  for (ReuseDistanceMapIt = ReuseDistanceDistribution.begin();
       ReuseDistanceMapIt != ReuseDistanceDistribution.end();
       ++ReuseDistanceMapIt)
   dbgs() << ReuseDistanceMapIt->first << " " << ReuseDistanceMapIt->second << "\n";
  
 dbgs() << "DATA_SET_SIZE\t" << node_size(ReuseTree) << "\n";
  
  //==================== Print resource statistics ===========================//
  printHeaderStat ("Statistics");
  dbgs() << "RESOURCE\tN_OPS_ISSUED\tSPAN\t\tISSUE-SPAN\tSTALL-SPAN\t\t \
  MAX_OCCUPANCY\n";
  
  for (unsigned j = 0; j < NExecutionUnits; j++) {
    {
      if (j >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
         ((j < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
          ((j%2 == 0 && FloatPrecision == 0) || (j %2 != 0 && FloatPrecision)))){
       dbgs() << getResourceName(j) << "\t\t" <<
        InstructionsCountExtended[j] << "\t\t" << ResourcesSpan[j] << "\t\t" <<
           IssueSpan[j] << "\t\t" <<
        ResourcesTotalStallSpanVector[j] << "\t\t" <<MaxOccupancy[j] << " \n";
        if (ScalarInstructionsCountExtended[j] +
            VectorInstructionsCountExtended[j]*VectorWidth !=
            InstructionsCountExtended[j]){
          dbgs() << "ScalarInstructionsCountExtended[j] " <<
          ScalarInstructionsCountExtended[j] << "\n";
          dbgs() << "VectorInstructionsCountExtended[j] " <<
          VectorInstructionsCountExtended[j] << "\n";
          report_fatal_error("SCALAR+VECTOR*VectorWidth != TOTAL");
        }
      }
    }
  }
  
  //==================== Calculate vectorization efficiencies ================//
  unsigned int opCountScalar = 0;
  unsigned int opCountVectorized = 0;
  double cyclesCountScalar = 0;
  double cyclesCountVectorized = 0;
  
  // Don't need opCount because it is the same as InstructionsCountExtended
  vector<double> cyclesPerPortIntelScalar;
  vector<double> cyclesPerPortIntelVector;
  
  for(unsigned int i = 0; i< NPorts; i++){
    cyclesPerPortIntelScalar.push_back(0);
    cyclesPerPortIntelVector.push_back(0);
  }
  
  // Arithmetic instructions
  for (unsigned j = 0; j < NArithmeticExecutionUnits; j++) {
    if ((j % 2 == 0 && FloatPrecision == 0) || (j % 2 != 0 && FloatPrecision)) {
      if (InstructionsCountExtended[j] > 0) {
        opCountScalar += InstructionsCountExtended[j];
        cyclesCountScalar += (double)InstructionsCountExtended[j]/
        (getEffectiveThroughput(j, AccessWidths[j],VectorWidth)/VectorWidth);
        opCountVectorized += VectorInstructionsCountExtended[j];
        cyclesCountVectorized += (double)InstructionsCountExtended[j]/
        getEffectiveThroughput(j, AccessWidths[j],VectorWidth);
        if (ScalarInstructionsCountExtended[j] +
            VectorInstructionsCountExtended[j] * VectorWidth !=
            InstructionsCountExtended[j])
        report_fatal_error("SCALAR+VECTOR*VectorWidth != TOTAL");
      }
    }
  }
  
  // Shuffle/blend instructions
  // -1: do not count the mov/bool
  for (unsigned j = NArithmeticExecutionUnits;
       j < NMovExecutionUnits+ NArithmeticExecutionUnits-2; j++) {
    if ((j % 2 == 0 && !FloatPrecision) || (j % 2 != 0 && FloatPrecision)) {
      if (InstructionsCountExtended[j] > 0) {
        opCountVectorized += VectorInstructionsCountExtended[j];
        cyclesCountVectorized += (double)InstructionsCountExtended[j]/
        getEffectiveThroughput(j, AccessWidths[j],VectorWidth);
      }
    }
  }
 
  double eff = (double)opCountScalar/(double)opCountVectorized;
  vectorizationEfficiency.push_back(eff);
  dumpList(vectorizationEfficiency, OutputDir + "/vect_efficiency.txt");
  
  //========================= Print stall cycles =============================//
  printHeaderStat ("Stall Cycles");
  dbgs() << "RESOURCE\tN_STALL_CYCLES\t\tAVERAGE_OCCUPANCY\t\t \
  FRACTION_OCCUPANCY\n";
  
  for (int j = RS_STALL; j <= LFB_STALL; j++) {
    if (TotalSpan == 0) {
      dbgs() << getResourceName(j) << "\t\t" << ResourcesSpan[j] << "\t\t" <<
      INF << "\n";
    } else {
      double AverageOccupancy = BuffersOccupancy[j -RS_STALL] /(double)TotalSpan;
      double FractionOccupancy = 0.0;
      dbgs() << getResourceName(j) << "\t\t" << ResourcesSpan[j] << "\t\t";
      fprintf (stderr, " %1.3f\t\t", AverageOccupancy);
      switch(j){
        case RS_STALL:
        if (ReservationStationSize!= 0)
        FractionOccupancy =  AverageOccupancy*100/ReservationStationSize;
        break;
        case ROB_STALL:
        if (ReorderBufferSize != 0)
        FractionOccupancy =  AverageOccupancy*100/ReorderBufferSize;
        break;
        case LB_STALL:
        if (LoadBufferSize != 0)
        FractionOccupancy =  AverageOccupancy*100/LoadBufferSize;
        break;
        case SB_STALL:
        if(StoreBufferSize!= 0)
        FractionOccupancy =  AverageOccupancy*100/StoreBufferSize;
        break;
        case LFB_STALL:
        if (LineFillBufferSize!= 0)
        FractionOccupancy =  AverageOccupancy*100/LineFillBufferSize;
        break;
        default:
        report_fatal_error("Buffer not recognized");
      }
      fprintf (stderr, " %1.3f\n", FractionOccupancy);
    }
  }

  printHeaderStat ("Span Only Stalls");
  {
    vector < int > tv;
    for (unsigned i = RS_STALL; i <= LFB_STALL; i++) {
      if (InstructionsCountExtended[i] > 0) {
        // This TotalStallSpan is just in case there are only stalls from
        //one buffer
        TotalStallSpan = ResourcesSpan[i];
        tv.push_back(i);
      }
    }
    if (tv.empty() == true)
      TotalStallSpan = 0;
    else
      TotalStallSpan = getGroupSpanFinal(tv);
  }
  dbgs() << TotalStallSpan << "\n";
  
  //======================= Print port Occupancy =============================//
  if(ConstraintPorts){
    printHeaderStat ("Port occupancy");
    dbgs() << "PORT\t\tDISPATCH CYCLES\n";
   {
     for (unsigned j = 0; j <NPorts; j++) {
       dbgs() << getResourceName(PORT_0+j) << "\t\t" <<
       ResourcesSpan[PORT_0+j] << "\n";
      }
    }
  }
  
  //======================== Resource-Stall Span =============================//
  printHeaderStat ("Resource-Stall Span");
  dbgs() << "RESOURCE";
  for (int j = RS_STALL; j <= LFB_STALL; j++)
    dbgs() << "\t" << getResourceName(j);
  dbgs() << "\n";
  {
    for (unsigned i = 0; i < NExecutionUnits; i++) {
      if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
         ((i < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
          ((i%2 == 0 && !FloatPrecision) || (i %2 != 0 && FloatPrecision)))){
       dbgs() << getResourceName(i) << "\t\t";
        for (uint j = RS_STALL; j <= LFB_STALL; j++) {
          vector < int >tv;
          if (InstructionsCountExtended[i] != 0 &&
              InstructionsCountExtended[j] != 0) {
            tv.push_back(i);
            tv.push_back(j);
            ResourcesStallSpanVector[i][j - RS_STALL] = getGroupSpanFinal(tv);
          }
          else {
            if (InstructionsCountExtended[i] == 0) {
              ResourcesStallSpanVector[i][j - RS_STALL] =
              InstructionsCountExtended[j];
            }else {
              if (InstructionsCountExtended[j] == 0) {
                ResourcesStallSpanVector[i][j - RS_STALL] = ResourcesSpan[i];
              }
            }
          }
         dbgs() << ResourcesStallSpanVector[i][j - RS_STALL] << "\t";
        }
       dbgs() << "\n";
      }
    }
  }
  
  //==================== Resource-Stall Overlap =============================//
#ifdef PRINT_OVERLAPS
  uint64_t Total;
  uint64_t T1, T2, OverlapCycles;
  
  printHeaderStat ("Resource-Stall Overlap (0-1)");
  dbgs() << "RESOURCE";
  for (unsigned j = RS_STALL; j <= LFB_STALL; j++)
    dbgs() << "\t" << getResourceName(j);
  
 dbgs() << "\n";
  
  float OverlapPercetage;
  for (unsigned i = 0; i < NExecutionUnits; i++) {
    if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
       ((i < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
        ((i%2 == 0 && !FloatPrecision) || (i %2 != 0 && FloatPrecision)))){
     dbgs() << getResourceName(i) << "\t\t";
      for (uint j = RS_STALL; j <= LFB_STALL; j++) {
        if (InstructionsCountExtended[i] != 0 && InstructionsCountExtended[j] !=
            0 && ResourcesSpan[i] != 0
            && ResourcesSpan[j] != 0) {
          Total = ResourcesStallSpanVector[i][j - RS_STALL];
          // When latency is zero, ResourcesSpan is zero. However, IssueSpan
          // might not be zero.
          T1 = ResourcesSpan[i];
          T2 = ResourcesSpan[j];
          assert (Total <= T1 + T2);
          OverlapCycles = T1 + T2 - Total;
          OverlapPercetage = (float) OverlapCycles / (float (min (T1, T2)));
          if (OverlapPercetage > 1.0)
            report_fatal_error("Overlap > 1.0 R-S overlap (0-1)");
        }
        else
          OverlapPercetage = 0;
        fprintf (stderr, " %1.3f ", OverlapPercetage);
      }
     dbgs() << "\n";
    }
  }
  
  //==================== ResourceIssue-Stall Span ============================//
  printHeaderStat ("ResourceIssue-Stall Span");
  dbgs() << "RESOURCE";
  for (unsigned j = RS_STALL; j <= LFB_STALL; j++)
    dbgs() << "\t" << getResourceName(j);
  dbgs() << "\n";
  {
    for (unsigned i = 0; i < NExecutionUnits; i++) {
      if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
         ((i < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
          ((i%2 == 0 && !FloatPrecision) || (i %2 != 0 && FloatPrecision)))){
       dbgs() << getResourceName(i) << "\t\t";
        for (uint j = RS_STALL; j <= LFB_STALL; j++) {
          dynamic_bitset <> BitMesh (LastIssueCycleFinal + MaxLatencyResources);
          if (InstructionsCountExtended[i] != 0 &&
              InstructionsCountExtended[j] != 0 &&
              ExecutionUnitsLatency[i]!= 0 && ExecutionUnitsLatency[j]!=0) {
            BitMesh |= CISFCache[i];
            BitMesh |= CGSFCache[j];
            ResourcesIssueStallSpanVector[i][j - RS_STALL] = BitMesh.count ();
          }
          else {
            if (InstructionsCountExtended[i] == 0 || ExecutionUnitsLatency[i]==0) {
              ResourcesIssueStallSpanVector[i][j - RS_STALL] =
              InstructionsCountExtended[j];;
            }
            else {
              if (InstructionsCountExtended[j] == 0 || ExecutionUnitsLatency[j]==0)
                ResourcesIssueStallSpanVector[i][j - RS_STALL] = IssueSpan[i];
            }
          }
         dbgs() << ResourcesIssueStallSpanVector[i][j - RS_STALL] << "\t";
        }
       dbgs() << "\n";
      }
    }
  }
  
  //==================== ResourceIssue-Stall Overlap =========================//
  printHeaderStat ("ResourceIssue-Stall Overlap (0-1)");
  dbgs() << "RESOURCE";
  for (unsigned j = RS_STALL; j <= LFB_STALL; j++)
   dbgs() << "\t" << getResourceName(j);

  dbgs() << "\n";
 
  float OverlapPercentage;
  for (unsigned i = 0; i < NExecutionUnits; i++) {
    if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
       ((i < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
        ((i%2 == 0 && !FloatPrecision) || (i %2 != 0 && FloatPrecision)))){
     dbgs() << getResourceName(i) << "\t\t";
      for (uint j = RS_STALL; j <= LFB_STALL; j++) {
        if (InstructionsCountExtended[i] != 0 &&
            InstructionsCountExtended[j] != 0 &&
            ExecutionUnitsLatency[i]!= 0 && ExecutionUnitsLatency[j]!=0) {
          Total = ResourcesIssueStallSpanVector[i][j - RS_STALL];
          T1 = IssueSpan[i];
          T2 = InstructionsCountExtended[j];
          assert (Total <= T1 + T2);
          OverlapCycles = T1 + T2 - Total;
          OverlapPercentage = (float) OverlapCycles / (float (min (T1, T2)));
          if (OverlapPercentage > 1.0) {
            dbgs() << "Issue Span " << T1 << "\n";
            dbgs() << "Stall Span " << T2 << "\n";
            dbgs() << "ResourcesIssueStallSpanVector[i][j - RS_STALL] (Total) "
            << ResourcesIssueStallSpanVector[i][j - RS_STALL] << "\n";
            dbgs() << "Overlap cycles " << OverlapCycles << "\n";
            report_fatal_error("Overlap > 1.0 RI-S Overlap (0-1)");
          }
        }
        else
          OverlapPercentage = 0;
        fprintf (stderr, " %1.3f ", OverlapPercentage);
      }
     dbgs() << "\n";
    }
  }
  
  //==================== Resource-Resource Span ==============================//
  printHeaderStat ("Resource-Resource Span (resources span without stalls)");
  
  dbgs() << "RESOURCE";
  for (unsigned j = 0; j < NExecutionUnits; j++) {
    if( j >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
       ((j < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
        ((j%2 == 0 && !FloatPrecision) || (j %2 != 0 && FloatPrecision)))){
     dbgs() << "\t" << getResourceName(j);
    }
  }
 dbgs() << "\n";
  {
    for (unsigned j = 0; j < NExecutionUnits; j++) {
      if( j >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
         ((j < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
          ((j%2 == 0 && !FloatPrecision) || (j %2 != 0 && FloatPrecision)))){
       dbgs() << getResourceName(j) << "\t\t";
        
        for (unsigned i = 0; i < j; i++) {
          if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
             ((i < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
              ((i%2 == 0 && !FloatPrecision) || (i %2 != 0 && FloatPrecision)))){
            
            vector < int >tv;
            if (InstructionsCountExtended[i] != 0 &&
                InstructionsCountExtended[j] != 0 &&
                ExecutionUnitsLatency[i]!= 0 && ExecutionUnitsLatency[j]!=0) {
              tv.push_back(j);
              tv.push_back(i);
              ResourcesResourcesNoStallSpanVector[j][i] = getGroupSpanFinal(tv);
            }
            else {
              if (InstructionsCountExtended[i] == 0 || ExecutionUnitsLatency[i]==0)
                ResourcesResourcesNoStallSpanVector[j][i] = ResourcesSpan[j];
              else if (InstructionsCountExtended[j] == 0 ||
                       ExecutionUnitsLatency[j]==0) {
                ResourcesResourcesNoStallSpanVector[j][i] = ResourcesSpan[i];
              }else
                report_fatal_error("This should not be executed\n");
            }
           dbgs() << ResourcesResourcesNoStallSpanVector[j][i] << "\t";
          }
        } // End of for loop for every other resource
       dbgs() << "\n";
      }
    }
  }
  
  printHeaderStat ("Resource-Resource Overlap Percentage (resources span \
                   without stall)");
  dbgs() << "RESOURCE";
  for (unsigned j = 0; j < NExecutionUnits; j++) {
    if( j >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
       ((j < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
        ((j%2 == 0 && !FloatPrecision) || (j %2 != 0 && FloatPrecision)))){
         dbgs() << "\t" << getResourceName(j);
    }
  }
 dbgs() << "\n";
  for (unsigned j = 0; j < NExecutionUnits; j++) {
    if( j >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
       ((j < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
        ((j%2 == 0 && !FloatPrecision) || (j %2 != 0 && FloatPrecision)))){
     dbgs() << getResourceName(j) << "\t\t";
      for (unsigned i = 0; i < j; i++) {
        if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
											((i < (NArithmeticExecutionUnits + NMovExecutionUnits))
                       && ((i%2 == 0 && !FloatPrecision) ||
                           (i %2 != 0 && FloatPrecision)))){
          if (InstructionsCountExtended[i] != 0 &&
              InstructionsCountExtended[j] != 0 && ResourcesSpan[j] != 0
              && ResourcesSpan[i] != 0) {
            Total = ResourcesResourcesNoStallSpanVector[j][i];
            T1 = ResourcesSpan[j];
            T2 = ResourcesSpan[i];
            OverlapCycles = T1 + T2 - Total;
            OverlapPercetage = (float) OverlapCycles / (float (min (T1, T2)));
            if (OverlapPercetage > 1.0) {
              report_fatal_error("Overlap > 1.0 R-R overlap % (resources \
                                 span without stall)");
            }
          }else
            OverlapPercetage = 0;
          fprintf (stderr, " %1.3f ", OverlapPercetage);
        }
      }
     dbgs() << "\n";
    }
  }
  
  //================= Resource-Resource Span (with stalls) ===================//
  printHeaderStat ("Resource-Resource Span (resources span with stalls)");
  dbgs() << "RESOURCE";
  for (unsigned j = 0; j < NExecutionUnits; j++) {
    if( j >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
       ((j < (NArithmeticExecutionUnits + NMovExecutionUnits))
        && ((j%2 == 0 && !FloatPrecision) || (j %2 != 0 && FloatPrecision)))){
     dbgs() << "\t" << getResourceName(j);
    }
  }
  dbgs() << "\n";
  {
    for (unsigned j = 0; j < NExecutionUnits; j++) {
      if( j >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
         ((j < (NArithmeticExecutionUnits + NMovExecutionUnits))
          && ((j%2 == 0 && !FloatPrecision) || (j %2 != 0 && FloatPrecision)))){
       dbgs() << getResourceName(j) << "\t\t";
        for (unsigned i = 0; i < j; i++) {
          if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
             ((i < (NArithmeticExecutionUnits + NMovExecutionUnits))
              && ((i%2 == 0 && !FloatPrecision) || (i %2 != 0 && FloatPrecision)))){
            if (InstructionsCountExtended[i] != 0 &&
                InstructionsCountExtended[j] != 0 && ResourcesSpan[j] != 0
                && ResourcesSpan[i] != 0) {
              vector < int >tv;
              tv.push_back(j);
              tv.push_back(i);
              for (unsigned k = RS_STALL; k <= LFB_STALL; k++) {
                tv.push_back(k);
              }
              ResourcesResourcesSpanVector[j][i] = getGroupSpanFinal(tv);
            }else {
              if (InstructionsCountExtended[i] == 0 || ResourcesSpan[i] == 0) {
                ResourcesResourcesSpanVector[j][i] = TotalStallSpan;
              }else if (InstructionsCountExtended[j] == 0 || ResourcesSpan[j] == 0)
                ResourcesResourcesSpanVector[j][i] = ResourcesTotalStallSpanVector[i];
            }
           dbgs() << ResourcesResourcesSpanVector[j][i] << "\t";
          }
        }
       dbgs() << "\n";
      }
    }
  }
  
  printHeaderStat ("Resource-Resource Overlap Percentage (resources span with \
                   stall)");
  dbgs() << "RESOURCE";
  for (unsigned j = 0; j < NExecutionUnits; j++) {
    if( j >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
       ((j < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
        ((j%2 == 0 && !FloatPrecision) || (j %2 != 0 && FloatPrecision)))){
         dbgs() << "\t" << getResourceName(j);
    }
  }
  dbgs() << "\n";
  for (unsigned j = 0; j < NExecutionUnits; j++) {
    if( j >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
       ((j < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
        ((j%2 == 0 && !FloatPrecision) || (j %2 != 0 && FloatPrecision)))){
     dbgs() << getResourceName(j) << "\t\t";
      for (unsigned i = 0; i < j; i++) {
        if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
           ((i < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
            ((i%2 == 0 && !FloatPrecision) || (i %2 != 0 && FloatPrecision)))){
          if (InstructionsCountExtended[i] != 0 && IssueSpan[i]!= 0 &&
              InstructionsCountExtended[j] != 0 && IssueSpan[j]!= 0 &&
              ResourcesTotalStallSpanVector[j] != 0 &&
              ResourcesTotalStallSpanVector[i] != 0) {
            Total = ResourcesResourcesSpanVector[j][i];
            T1 = ResourcesTotalStallSpanVector[j];
            T2 = ResourcesTotalStallSpanVector[i];
            
            assert (Total <= T1 + T2);
            OverlapCycles = T1 + T2 - Total;
            OverlapPercetage = (float) OverlapCycles / (float (min (T1, T2)));
            if (OverlapPercetage > 1.0) {
              report_fatal_error("Overlap > 1.0 R-R overlap % (resources span \
                                 with stall)");
            }
          }else
            OverlapPercetage = 0;
          fprintf (stderr, " %1.3f ", OverlapPercetage);
        }
      }
     dbgs() << "\n";
    }
  }

  printHeaderStat ("Stall-Stall Span");
  dbgs() << "RESOURCE";
  for (unsigned j = RS_STALL; j <= LFB_STALL; j++)
    dbgs() << "\t" << getResourceName(j);

  dbgs() << "\n";
  {
    for (unsigned j = RS_STALL; j <= LFB_STALL; j++) {
     dbgs() << getResourceName(j) << "\t\t";
      for (unsigned i = RS_STALL; i < j; i++) {
        if (InstructionsCountExtended[j] != 0 && InstructionsCountExtended[i] != 0) {
          vector < int >tv;
          tv.push_back(j);
          tv.push_back(i);
          StallStallSpanVector[j - RS_STALL][i - RS_STALL] = getGroupSpanFinal(tv);
        }else {
          if (InstructionsCountExtended[i] == 0) {
            StallStallSpanVector[j - RS_STALL][i - RS_STALL] = ResourcesSpan[j];
          }else if (InstructionsCountExtended[j] == 0)
            StallStallSpanVector[j - RS_STALL][i - RS_STALL] = ResourcesSpan[i];
        }
       dbgs() << StallStallSpanVector[j - RS_STALL][i - RS_STALL] << "\t";
      }
     dbgs() << "\n";
    }
  }
  
  printHeaderStat ("Stall-Stall Overlap Percentage ");
  dbgs() << "RESOURCE";
  for (unsigned j = RS_STALL; j <= LFB_STALL; j++)
    dbgs() << "\t" << getResourceName(j);
  dbgs() << "\n";
  
  for (unsigned j = RS_STALL; j <= LFB_STALL; j++) {
   dbgs() << getResourceName(j) << "\t\t";
    for (unsigned i = RS_STALL; i < j; i++) {
      if (InstructionsCountExtended[j] != 0 && InstructionsCountExtended[i] != 0) {
        Total = StallStallSpanVector[j - RS_STALL][i - RS_STALL];
        T1 = ResourcesSpan[j];
        T2 = ResourcesSpan[i];
        assert (Total <= T1 + T2);
        OverlapCycles = T1 + T2 - Total;
        OverlapPercetage = (float) OverlapCycles / (float (min (T1, T2)));
      }else
        OverlapPercetage = 0;
      fprintf (stderr, " %1.3f ", OverlapPercetage);
    }
   dbgs() << "\n";
  }
#endif
  
  // ===================== ALL OVERLAPS - SECOND APPROACH ====================//
#ifdef PRINT_ALL_OVERLAPS
  {
    printHeaderStat ("All overlaps");
    unsigned n = NExecutionUnits+NBuffers;
    int nCombinations = 0;
    bool ResourceWithNoInstructions = false;
    uint64_t T1, T2, OverlapCycles, MinResourceSpan, MinResource;
    
    // The variable k denotes the size of the groups
    for (unsigned k = 2; k <= NExecutionUnits+NBuffers; k++) {
      vector < vector < int > >combinations;
      vector < int >selected;
      vector < int >selector (n);
      fill (selector.begin(), selector.begin() + k, 1);
      do {
        for (unsigned i = 0; i < n; i++) {
          if (selector[i])
            selected.push_back(i);
        }
        nCombinations++;
        std::vector < int >result (selected.size());
        copy (selected.begin(), selected.end(), result.begin());
        OverlapCycles = 0;
        MinResourceSpan = 0;
        OverlapPercetage = 0;
        MinResource = 0;
        
        for (unsigned j = 0; j < result.size(); j++) {
          if (InstructionsCountExtended[result[j]] == 0) {
            ResourceWithNoInstructions = true;
            break;
          }
        }
        if (ResourceWithNoInstructions== false) {
          for (unsigned j = 0; j < result.size(); j++) {
           dbgs() << result[j] << " ";
            if (InstructionsCountExtended[result[j]] > 0) {
              if (MinResourceSpan == 0){
                MinResourceSpan = ResourcesSpan[result[j]];
                MinResource =result[j];
              }
              else{
                MinResourceSpan = min (MinResourceSpan, ResourcesSpan[result[j]]);
                if(MinResourceSpan == ResourcesSpan[result[j]] )
                MinResource = result[j];
              }
            }
          }
          
          if (MinResourceSpan != 0) {
            OverlapCycles = getGroupOverlapCyclesFinal (result);
            OverlapPercetage = (float)OverlapCycles/(float(MinResourceSpan));
            for (unsigned j = 0; j < result.size(); j++) {
              OverlapsMetrics[result[j]]+= OverlapCycles;
            }
          }
          dbgs() << "| " << MinResource << " | ";
          dbgs() << OverlapCycles << " ";
          fprintf (stderr, " %1.3f\n", OverlapPercetage);
          
          OverlapsDerivatives[MinResource]+= OverlapPercetage*pow(-1,k+1);
          
          for (unsigned j = 0; j < result.size(); j++) {
            AverageOverlapsCycles[result[j]]+= OverlapCycles;
            AverageOverlaps[result[j]]+= OverlapPercetage;
            OverlapsCount[result[j]]++;
          }
          
          if (OverlapPercetage > 1.0) {
           dbgs() << "Overlap Cycles " << OverlapCycles << "\n";
           dbgs() << "MinResourceSpan " << MinResourceSpan << "\n";
            report_fatal_error("Overlap > 1.0");
          }
        }else
          ResourceWithNoInstructions = false;
        
        result.clear();
        selected.clear();
      }
      while (prev_permutation (selector.begin(), selector.end()));
    }
    for (size_t j = 0; j < AverageOverlapsCycles.size(); j++) {
      if (OverlapsCount[j]!=0) {
        AverageOverlapsCycles[j] = AverageOverlapsCycles[j]/OverlapsCount[j];
        AverageOverlaps[j] = AverageOverlaps[j]/OverlapsCount[j];
      }
    }
    
    double MaxOverlapCycles = AverageOverlapsCycles[0];
    for (size_t j = 1; j < AverageOverlapsCycles.size(); j++) {
      if (AverageOverlapsCycles[j] > MaxOverlapCycles) {
        MaxOverlapCycles = AverageOverlapsCycles[j];
      }
    }
    
    double MaxOverlap = AverageOverlaps[0];
    for (size_t j = 1; j < AverageOverlaps.size(); j++) {
      if (AverageOverlaps[j] > MaxOverlap) {
        MaxOverlap = AverageOverlaps[j];
      }
    }
    
    vector < int >nonEmptyExecutionUnits;
    for (unsigned j = 0; j < NExecutionUnits; j++) {
      if (InstructionsCountExtended[j] > 0)
        nonEmptyExecutionUnits.push_back(j);
    }
    
    for (unsigned j = RS_STALL; j <= LFB_STALL; j++) {
      OverlapCycles = 0;
      OverlapPercetage = 0;
      if (InstructionsCountExtended[j] > 0) {
        // For every other resource
        vector < int >nonEmptyStalls;
        for (unsigned i = RS_STALL; i <= LFB_STALL; i++) {
          if (i != j && InstructionsCountExtended[i] > 0) {
            nonEmptyStalls.push_back(i);
          }
        }
        // Copy resources into stalls:
        nonEmptyStalls.insert (nonEmptyStalls.end(),
                               nonEmptyExecutionUnits.begin(),
                               nonEmptyExecutionUnits.end());
        nonEmptyStalls.insert (nonEmptyStalls.begin(), j);
        OverlapCycles = GetOneToAllOverlapCyclesFinal (nonEmptyStalls);
        OverlapPercetage = (float) OverlapCycles / (float (ResourcesSpan[j]));
      }
      dbgs() << j << " " << OverlapCycles;
      fprintf (stderr, " %1.3f\n", OverlapPercetage);
      if (OverlapPercetage > 1.0) {
       dbgs() << "Overlap Cycles " << OverlapCycles << "\n";
       dbgs() << "MinResourceSpan " << MinResourceSpan << "\n";
        report_fatal_error("Overlap > 1.0");
      }
    }
  }
#endif
  
  {
    // + 4 to include L1load together with L1store, as if they were one resource
    // for L1 cache, L2, L3, and the accesses to mem
    // Before +1 to include L1_load and L1_store. Now +2 because the one before
    // the last in the register file size
    vector < vector < uint64_t > >
    ResourcesOverlapCycles(NExecutionUnits+NBuffers+2,
                           vector < uint64_t > (NExecutionUnits+NBuffers));
    
    vector < vector < uint64_t > >
    ResourcesOnlyLatencyOverlapCycles (NExecutionUnits,
                                       vector < uint64_t > (NExecutionUnits+NBuffers));
    
    vector < vector < uint64_t > >
    ResourcesOnlyIssueOverlapCycles (NExecutionUnits,
                                     vector < uint64_t > (NExecutionUnits+NBuffers));
    
    bool L1ResourceFound = false;
    for (unsigned i = 0; i < TotalSpan; i++){
      vector < unsigned > resourcesInCycle;
      L1ResourceFound = false;
      for (unsigned j = 0; j< NExecutionUnits+NBuffers; j++){
        if (InstructionsCountExtended[j]!= 0 && ExecutionUnitsLatency[j]!= 0
            && CGSFCache[j][i]==1 )
        resourcesInCycle.push_back(j);
      }
      
      for (unsigned j = 0; j< resourcesInCycle.size(); j++){
        ResourcesOverlapCycles[resourcesInCycle[j]][resourcesInCycle.size()]++;
        if ((resourcesInCycle[j]==L1_LOAD_CHANNEL ||
             resourcesInCycle[j] == L1_STORE_CHANNEL)){
          if(L1ResourceFound==false){
            L1ResourceFound = true;
            ResourcesOverlapCycles.back()[resourcesInCycle.size()]++;
          }else{
            ResourcesOverlapCycles.back()[resourcesInCycle.size()]--;
            ResourcesOverlapCycles.back()[resourcesInCycle.size()-1]++;
          }
        }
      }
      
      for (unsigned j = 0; j< NExecutionUnits; j++){
        resourcesInCycle.clear();
        if (InstructionsCountExtended[j]!= 0 && ExecutionUnitsLatency[j]!= 0 &&
            CISFCache[j][i]==1 ){
          resourcesInCycle.push_back(j);
          for (unsigned k = 0; k< NExecutionUnits; k++){
            if ( k != j && InstructionsCountExtended[k]!= 0 &&
                ExecutionUnitsLatency[k]!= 0 && CGSFCache[k][i]==1){
              resourcesInCycle.push_back(k);
            }
          }
        }
        ResourcesOnlyIssueOverlapCycles[j][resourcesInCycle.size()]++;
      }
      
      for (unsigned j = 0; j< NExecutionUnits; j++){
        resourcesInCycle.clear();
        if (InstructionsCountExtended[j]!= 0  &&  ExecutionUnitsLatency[j]!= 0 &&
            CLSFCache[j][i]==1){
          resourcesInCycle.push_back(j);
          for (unsigned k = 0; k< NExecutionUnits; k++){
            if ( k != j && InstructionsCountExtended[k]!= 0  &&
                ExecutionUnitsLatency[k]!= 0 &&  CGSFCache[k][i]==1){
              resourcesInCycle.push_back(k);
            }
          }
        }
        ResourcesOnlyLatencyOverlapCycles[j][resourcesInCycle.size()]++;
      }
    }
    
    // Add data for caches
    ResourcesOverlapCycles[NExecutionUnits+NBuffers]=
    ResourcesOverlapCycles[REGISTER_LOAD_CHANNEL];
    ResourcesOverlapCycles.push_back(ResourcesOverlapCycles[L2_LOAD_CHANNEL]);
    ResourcesOverlapCycles.push_back(ResourcesOverlapCycles[L3_LOAD_CHANNEL]);
    ResourcesOverlapCycles.push_back(ResourcesOverlapCycles[MEM_LOAD_CHANNEL]);
    
    printHeaderStat ("Breakdown Overlap");
    
    for (unsigned j = 0; j< NExecutionUnits+NBuffers+5; j++){
      if( j >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
         ((j < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
          ((j%2 == 0 && !FloatPrecision) || (j %2 != 0 && FloatPrecision)))){
        if (j < NExecutionUnits+NBuffers)
          dbgs() << getResourceName(j);
        else{
          if (j == NExecutionUnits+NBuffers)
            dbgs() << "REGISTER";
          if (j ==  NExecutionUnits+NBuffers+1)
            dbgs() << "ALL_L1";
          if (j == NExecutionUnits+NBuffers+2)
            dbgs() << "L2";
          if (j == NExecutionUnits+NBuffers+3)
            dbgs() << "LLC";
          if (j == NExecutionUnits+NBuffers+4)
            dbgs() << "MEM";
        }
        for (unsigned i = 1; i< NExecutionUnits+NBuffers; i++)
          dbgs() << " "<< ResourcesOverlapCycles[j][i] ;
        dbgs() << "\n";
      }
    }
    
    printHeaderStat ("Breakdown Overlap - Issue and only latency separated");
    
    for (unsigned j = 0; j< NExecutionUnits; j++){
      if( j >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
         ((j < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
          ((j%2 == 0 && !FloatPrecision) || (j %2 != 0 && FloatPrecision)))){
    
        dbgs() << getResourceName(j) << " ISSUE";
        for (unsigned i = 1; i< NExecutionUnits+NBuffers; i++)
          dbgs() << " "<< ResourcesOnlyIssueOverlapCycles[j][i] ;
        dbgs() << "\n";
        dbgs() << getResourceName(j) << " ONLY_LAT";
        for (unsigned i = 1; i< NExecutionUnits+NBuffers; i++)
          dbgs() << " "<< ResourcesOnlyLatencyOverlapCycles[j][i] ;
        dbgs() << "\n";
      }
    }
    
    for (unsigned j = NExecutionUnits; j< NExecutionUnits+NBuffers+5; j++){
      if (j < NExecutionUnits+NBuffers)
        dbgs() << getResourceName(j);
      else{
        if (j == NExecutionUnits+NBuffers)
          dbgs() << "REGISTER";
        if (j == NExecutionUnits+NBuffers+1)
          dbgs() << "ALL_L1";
        if (j == NExecutionUnits+NBuffers+2)
          dbgs() << "L2";
        if (j == NExecutionUnits+NBuffers+3)
          dbgs() << "LLC";
        if (j == NExecutionUnits+NBuffers+4)
          dbgs() << "MEM";
      }
      for (unsigned i = 1; i< NExecutionUnits+NBuffers; i++){
        dbgs() << " "<< ResourcesOverlapCycles[j][i] ;
      }
      dbgs() << "\n";
    }
  }
#ifdef PRINT_ALL_OVERLAPS
  
  // ===================== ALL OVERLAPS - THIRD APPROACH ====================//
  {
    printHeaderStat ("Overlaps - Each resource with all the others");
    
    vector < int >nonEmptyExecutionUnits;
    for (unsigned i = 0; i < NExecutionUnits+NBuffers; i++) {
      OverlapCycles = 0;
      OverlapPercetage = 0;
      nonEmptyExecutionUnits.clear();
      
      if (InstructionsCountExtended[i]!= 0 ) {
        nonEmptyExecutionUnits.push_back(i);
        // For all others
        for (unsigned j = 0; j < NExecutionUnits+NBuffers ; j++) {
          if (j!= i) {
            if (InstructionsCountExtended[j]!= 0) {
              nonEmptyExecutionUnits.push_back(j);
            }
          }
        }
        if(ResourcesSpan[i]!= 0){
          OverlapCycles = GetOneToAllOverlapCyclesFinal (nonEmptyExecutionUnits);
          OverlapPercetage = (float) OverlapCycles / (float (ResourcesSpan[i]));
        }
       dbgs() << getResourceName(i) << " " << OverlapCycles;
        fprintf (stderr, " %1.3f\n", OverlapPercetage);
        
      }
    }
  }
  
  // ================ ALL OVERLAPS - ISSUE/LATENCY APPROACH ===================//

    printHeaderStat ("Overlaps - Issue/Latency with all the others");
    
    vector < int >nonEmptyExecutionUnits;
    // In this case only execution units
    for (unsigned i = 0; i < NExecutionUnits; i++) {
      OverlapCycles = 0;
      OverlapPercetage = 0;
      nonEmptyExecutionUnits.clear();
      
      if (InstructionsCountExtended[i]!= 0) {
        nonEmptyExecutionUnits.push_back(i);
        for (unsigned j = 0; j < NExecutionUnits+NBuffers ; j++) {
          if (j!= i) {
            if (InstructionsCountExtended[j]!= 0) {
              nonEmptyExecutionUnits.push_back(j);
            }
          }
        }
        if(IssueSpan[i]!=0){
          OverlapCycles = GetOneToAllOverlapCyclesFinal (nonEmptyExecutionUnits,
                                                         true);
          OverlapPercetage = (float) OverlapCycles / (float (IssueSpan[i]));
        }
        dbgs() << getResourceName(i) << " ISSUE " << OverlapCycles;
        fprintf (stderr, " %1.3f\n", OverlapPercetage);

        if (LatencyOnlySpan[i]==0){
          OverlapCycles = 0;
          OverlapPercetage = 0.0;
        }else{
          OverlapCycles = GetOneToAllOverlapCyclesFinal (nonEmptyExecutionUnits,
                                                         false); // Latency
          OverlapPercetage = (float) OverlapCycles / (float (LatencyOnlySpan[i]));
        }
        dbgs() << getResourceName(i) << " ONLY LAT " << OverlapCycles;
        fprintf (stderr, " %1.3f\n", OverlapPercetage);
        
      }
    }
#endif

  //======================= Bottlenecks ===============================//
  
  printHeaderStat ("Bottlenecks - Buffers stalls added to issue");
  dbgs() << "Bottleneck\tISSUE\tLAT\t";
  for (int j = RS_STALL; j <= LFB_STALL; j++)
   dbgs() << getResourceName(j) << "\t";
  
  dbgs() << "\n";
  uint64_t Work;
  for (unsigned i = 0; i < NExecutionUnits; i++) {
    if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
       ((i < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
        ((i%2 == 0 && !FloatPrecision) || (i %2 != 0 && FloatPrecision)))){
      
      if(InstructionsCountExtended[i] > 0){
        auto & BnkVec = BnkMat[i];
        // Work is always the total number of floating point operations...
        // Otherwise it makes no sense to compare with the performance for
        // memory nodes which is calcualted  with total work
        Work =nArithmeticInstructionCount;
        dbgs() << getResourceName(i) << "\t\t";
        if (IssueSpan[i] > 0) {
          Performance = (float) Work / ((float) IssueSpan[i]);
          fprintf (stderr, " %1.3f ", Performance);
          BnkVec[0] = Performance;
        }else {
          dbgs() << INF << "\t";
          BnkVec[0] = INF;
        }
        
        if (ResourcesSpan[i] > 0) {
          Performance = (float) Work / ((float) ResourcesSpan[i]);
          fprintf (stderr, " %1.3f ", Performance);
          BnkVec[1] = Performance;
        }else {
          dbgs() << INF << "\t";
          BnkVec[1] = INF;
        }
        
        for (unsigned j = 0; j < NBuffers; j++) {
          if (ResourcesIssueStallSpanVector[i][j] > 0 &&
              ResourcesSpan[j + RS_STALL] != 0) {
            Performance = (float) Work / ((float)ResourcesIssueStallSpanVector[i][j]);
            fprintf (stderr, " %1.3f ", Performance);
            BnkVec[j + 2] = Performance;
          }else {
            dbgs() << INF << "\t";
            BnkVec[j + 2] = INF;
          }
        }
       dbgs() << "\n";
      }else{
       dbgs() << getResourceName(i) << "\t\t";
        for (unsigned j = 0; j < NBuffers+2; j++)
         dbgs() << INF << "\t";
       dbgs() << "\n";
      }
    }
  }
  
  {
    printHeaderStat ("Bottlenecks - Buffers stalls added to latency");
    dbgs() << "Bottleneck\tISSUE\tLAT\t";
    for (int j = RS_STALL; j <= LFB_STALL; j++)
     dbgs() << getResourceName(j) << "\t";

    dbgs() << "\n";
    uint64_t Work;
    
    for (unsigned i = 0; i < NExecutionUnits; i++) {
      if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
         ((i < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
          ((i%2 == 0 && !FloatPrecision) || (i %2 != 0 && FloatPrecision)))){
        if(InstructionsCountExtended[i] > 0){
          auto & BnkVec = BnkMat[i];
          Work = nArithmeticInstructionCount;
          dbgs() << getResourceName(i) << "\t\t";
          
          if (IssueSpan[i] > 0) {
            Performance = (float) Work / ((float) IssueSpan[i]);
            fprintf (stderr, " %1.3f ", Performance);
            BnkVec[0] = Performance;
          }else {
            dbgs() << INF << "\t";
            BnkVec[0] = INF;
          }
          
          if (ResourcesSpan[i] > 0) {
            Performance = (float) Work / ((float) ResourcesSpan[i]);
            fprintf (stderr, " %1.3f ", Performance);
            BnkVec[1] = Performance;
          }else {
            dbgs() << INF << "\t";
            BnkVec[1] = INF;
          }
          
          for (unsigned j = 0; j < NBuffers; j++) {
            if (ResourcesIssueStallSpanVector[i][j] > 0 &&
                ResourcesSpan[j + RS_STALL] != 0) {
              Performance = (float) Work / ((float) ResourcesStallSpanVector[i][j]);
              fprintf (stderr, " %1.3f ", Performance);
              BnkVec[j + 2] = Performance;
            }else {
              dbgs() << INF << "\t";
              BnkVec[j + 2] = INF;
            }
          }
         dbgs() << "\n";
        }else{
          dbgs() << getResourceName(i) << "\t\t";
          for (unsigned j = 0; j < NBuffers+2; j++)
           dbgs() << INF << "\t";
         dbgs() << "\n";
        }
      }
    }
  }
  
  //======================= Buffers Bottlenecks ==============================//
  printHeaderStat ("Buffers Bottlenecks");
  {
    uint64_t Work = nArithmeticInstructionCount;
    dbgs() << "Bottleneck\tISSUE\n";
    for (int j = RS_STALL; j <= LFB_STALL; j++) {
     dbgs() << getResourceName(j) << "\t";
      if(InstructionsCountExtended[j]!=0)
        fprintf (stderr, " %1.3f\n", ((float)Work)/ResourcesSpan[j]);
      else
        dbgs() << INF << "\n";
    }
  }
  
  //=============== Botttlenecks without buffers and latency separate =========//
  {
   printHeaderStat ("Bottlenecks II");
   dbgs() << "Bottleneck\tISSUE\tLAT_ONLY\n";
 
    uint64_t Work;
    for (unsigned i = 0; i < NExecutionUnits; i++) {
      if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
         ((i < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
          ((i%2 == 0 && !FloatPrecision) || (i %2 != 0 && FloatPrecision)))){
        if(InstructionsCountExtended[i] > 0){
          Work = nArithmeticInstructionCount;
          dbgs() << getResourceName(i) << "\t\t";
      
          if (IssueSpan[i] > 0) {
            Performance = (float) Work / ((float) IssueSpan[i]);
            fprintf (stderr, " %1.3f ", Performance);
          }else
           dbgs() << INF << "\t";
          
          if (LatencyOnlySpan[i] > 0) {
            Performance = (float) Work / ((float) LatencyOnlySpan[i]);
            fprintf (stderr, " %1.3f ", Performance);
          }else
           dbgs() << INF << "\t";
         dbgs() << "\n";
        }else{
         dbgs() << getResourceName(i) << "\t\t";
          for (unsigned j = 0; j < 2; j++) {
           dbgs() << INF << "\t";
          }
         dbgs() << "\n";
        }
      }
    }
  }
  
  //===================== Issue latency overlap  =============================//
  printHeaderStat ("Issue/Only Latency Cycles");
  
  for(uint i = 0; i< NExecutionUnits; i++){
    if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
       ((i < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
        ((i%2 == 0 && !FloatPrecision) || (i %2 != 0 && FloatPrecision)))){
      dbgs() << getResourceName(i) << " " <<  IssueSpan[i] << " " <<
         LatencyOnlySpan[i] << "\n";
      
    }
  }
  
  //======================= Execution Times Breakdown ========================//
  printHeaderStat ("Execution Times Breakdowns");
  dbgs() << "RESOURCE\tMIN-EXEC-TIME\tISSUE-EFFECTS\tLATENCY-EFFECTS\t \
    STALL-EFFECTS\tTOTAL\n";

  unsigned MinExecutionTime = 0;
  unsigned IssueEffects = 0;
  unsigned LatencyEffects = 0;
  unsigned StallEffects = 0;
  double Throughput = 0;
  
  for (unsigned i = 0; i < NExecutionUnits; i++) {
    if( i >= (NArithmeticExecutionUnits + NMovExecutionUnits) ||
       ((i < (NArithmeticExecutionUnits + NMovExecutionUnits)) &&
        ((i%2 == 0 && !FloatPrecision) || (i %2 != 0 && FloatPrecision)))){
      if (InstructionsCountExtended[i] == 0) {
        MinExecutionTime = 0;
        LatencyEffects = 0;
        IssueEffects = 0;
        StallEffects = ResourcesTotalStallSpanVector[i];
       dbgs() << getResourceName(i) << "\t\t";
       dbgs() << " " << MinExecutionTime;
       dbgs() << "\t";
       dbgs() << " " << IssueEffects;
       dbgs() << "\t";
       dbgs() << " " << LatencyEffects;
       dbgs() << "\t";
       dbgs() << " " << StallEffects;
        // fprintf(stderr, " %1.3f ", StallEffects);
        if (MinExecutionTime + IssueEffects + LatencyEffects + StallEffects !=
            ResourcesTotalStallSpanVector[i]
            && MinExecutionTime != 0) {
          report_fatal_error("Breakdown of execution time does not match total\
                             execution time\n");
          
        }else
         dbgs() << "\t" << ResourcesTotalStallSpanVector[i] << "\n";
        
      }else {
        Throughput= getEffectiveThroughput(i, AccessWidths[i],VectorWidth);
        if (ExecutionUnitsLatency[i] == 0) {
          MinExecutionTime = 0;
        }else {
          if (i < NArithmeticExecutionUnits + NMovExecutionUnits) {
            if (Throughput == INF) {
              MinExecutionTime = 1;
            } else {
              double tmpMinExecutionTime = InstructionsCountExtended[i]/Throughput;
              double intpart;
              double fractpart = modf (tmpMinExecutionTime , &intpart);
              if (fractpart <= 0.005){
                MinExecutionTime =
                (unsigned)(InstructionsCountExtended[i]/Throughput);
            }else{
                MinExecutionTime =
                (unsigned)ceil(InstructionsCountExtended[i]/Throughput);
              }
            }
          }else {
            if (Throughput == INF) {
              MinExecutionTime = 1;
            }else {
              double tmpMinExecutionTime = InstructionsCountExtended[i]/ Throughput;
              double intpart;
              double fractpart = modf (tmpMinExecutionTime , &intpart);
							if (fractpart <= 0.005)
                MinExecutionTime =
                (unsigned)(InstructionsCountExtended[i]/ Throughput);
              else
                MinExecutionTime =
                (unsigned)ceil(InstructionsCountExtended[i]/ Throughput);
            }
          }
        }
        if (IssueSpan[i] != 0 && IssueSpan[i] < MinExecutionTime) {
          dbgs() << "Throughput " << Throughput << "\n";
          dbgs() << "IssueSpan[i] " << IssueSpan[i] << "\n";
          dbgs() << "MinExecutionTime " << MinExecutionTime << "\n";
          report_fatal_error("IssueSpan < Min execution time");
        }
        if (ScalarInstructionsCountExtended[i] != 0 &&
            VectorInstructionsCountExtended[i] != 0) {
          unsigned AdjustedIssueSpan = IssueSpan[i];
          if (ExecutionUnitsParallelIssue[i] != INF) {
          }else {// If ExecutionUnitsParallelIssue in INF but Throughput is not
            if (ExecutionUnitsThroughput[i] != INF) {
              if (AccessWidths[i] * VectorWidth > (ExecutionUnitsThroughput[i])){
                // Issue Span calculated as before needs correction.
                unsigned IssueGranularity =ceil(AccessWidths[i] * VectorWidth /
                                                (ExecutionUnitsThroughput[i]));
                AdjustedIssueSpan = AdjustedIssueSpan +
                VectorInstructionsCountExtended[i] *
                (IssueGranularity - 1);
                // Because span already includes the first issue cycle.
              }
            }
          }
          
          if(AdjustedIssueSpan < MinExecutionTime){
            dbgs() << "AdjustedIssueSpan " << AdjustedIssueSpan << "\n";
            dbgs() << "MinExecutionTime " << MinExecutionTime << "\n";
            report_fatal_error("Span cannot be smaller than MinExecutionTime");
          }
          
          IssueEffects = AdjustedIssueSpan - MinExecutionTime;
          if (ResourcesSpan[i] != 0) {
            LatencyEffects = ResourcesSpan[i] - AdjustedIssueSpan;
          }
        }else {
          if(IssueSpan[i] < MinExecutionTime){
            dbgs() << "IssueSpan[i] " << IssueSpan[i] << "\n";
            dbgs() << "MinExecutionTime " << MinExecutionTime << "\n";
            report_fatal_error("Span cannot be smaller than MinExecutionTime");
          }
          
          IssueEffects = IssueSpan[i] - MinExecutionTime;
          if (ResourcesSpan[i] != 0) {
            if(ResourcesSpan[i] < IssueSpan[i] ){
              dbgs() << "IssueSpan[i] " << IssueSpan[i] << "\n";
              dbgs() << "ResourcesSpan[i] " << ResourcesSpan[i] << "\n";
              report_fatal_error("Span cannot be smaller than issue spans");
            }
            LatencyEffects = ResourcesSpan[i] - IssueSpan[i];
          }
        }
        StallEffects = ResourcesTotalStallSpanVector[i] - ResourcesSpan[i];
        
       dbgs() << getResourceName(i) << "\t\t";
       dbgs() << " " << MinExecutionTime;
       dbgs() << "\t";
       dbgs() << " " << IssueEffects;
       dbgs() << "\t";
       dbgs() << " " << LatencyEffects;
       dbgs() << "\t";
       dbgs() << " " << StallEffects;
       dbgs() << "\t" << ResourcesTotalStallSpanVector[i] << "\n";
      }
    }
  }
  
    printHeaderStat ("TOTAL");
    dbgs() << "TOTAL COMP" << "\t" <<nArithmeticInstructionCount +
    InstructionsCountExtended[FP32_SHUFFLE_NODE] +
    InstructionsCountExtended[FP32_BLEND_NODE] +
    InstructionsCountExtended[FP32_BOOL_NODE]+
    InstructionsCountExtended[FP64_SHUFFLE_NODE] +
    InstructionsCountExtended[FP64_BLEND_NODE] +
    InstructionsCountExtended[FP64_BOOL_NODE]
    << "\t\t" << calculateGroupSpanFinal(allCompResources) << " \n";
    
    dbgs() << "TOTAL FLOPS" << "\t" << nArithmeticInstructionCount << "\t\t" <<
    calculateGroupSpanFinal(compResources) << " \n";
    dbgs() << "TOTAL SHUFFLE/BLEND/BOOL" << "\t" <<
    InstructionsCountExtended[FP32_SHUFFLE_NODE] +
    InstructionsCountExtended[FP32_BLEND_NODE] +
    InstructionsCountExtended[FP32_BOOL_NODE]+
    InstructionsCountExtended[FP64_SHUFFLE_NODE] +
    InstructionsCountExtended[FP64_BLEND_NODE] +
    InstructionsCountExtended[FP64_BOOL_NODE]
    << "\t\t" << calculateGroupSpanFinal(movResources) << " \n";
    dbgs() << "TOTAL MOPS" << "\t" << InstructionsCount[1] << "\t\t" <<
    calculateGroupSpanFinal(memResources) << " \n";
    dbgs() << "TOTAL" << "\t\t" << InstructionsCount[0] + InstructionsCount[1] <<
    "\t\t" << TotalSpan << " \n";
    Performance = (float) nArithmeticInstructionCount / ((float) TotalSpan);
    fprintf (stderr, "PERFORMANCE %1.3f\n", Performance);
    dbgs() << "KIPS\t"  <<TotalInstructions << "\n";
    dbgs() << "RegisterSpills - Loads " << "\t" << NRegisterSpillsLoads <<" \n";
    dbgs() << "RegisterSpills - Stores " << "\t" << NRegisterSpillsStores <<" \n";
    if (NRegisterSpillsStores > NRegisterSpillsLoads)
    report_fatal_error("The number of spill stores should not be larger than \
                       the number of spill loads. Nothing should be spilled if \
                       is not going to be loaded again");
    cycleList.push_back(TotalSpan);
    flopList.push_back(nArithmeticInstructionCount);
    if (OutputDir == ""){
      report_fatal_error("Outputdir must be specified");
    }else{
      dumpList(cycleList, OutputDir + "/cycles.txt");
      dumpList(flopList, OutputDir+ "/flops.txt");
    }
  
#ifdef SOURCE_CODE_ANALYSIS
  printHeaderStat("SOURCE CODE LINE INFO");
  //First, iterate over the map that contains an entry for each code line, and
  // the value mapped is a set of all the distinct cycles to which this source
  // code line contributes to
  typedef map < uint64_t, set < uint64_t > >::iterator it_type;
  // Get all source code lines:
  vector < unsigned >AllSourceCodeLines;
  for (it_type iterator = SourceCodeLineInfo.begin();
       iterator != SourceCodeLineInfo.end(); iterator++) {
    AllSourceCodeLines.push_back(iterator->first);
  }
  // Sort the vector
  std::sort (AllSourceCodeLines.begin(), AllSourceCodeLines.end());
    
  std::map < unsigned, unsigned >AdjustedSourceCodeLines;
  for (std::vector < unsigned >::iterator it = AllSourceCodeLines.begin();
       it != AllSourceCodeLines.end(); ++it)
    AdjustedSourceCodeLines[*it] = *(it) + 1;

  for (it_type iterator = SourceCodeLineInfo.begin();
       iterator != SourceCodeLineInfo.end(); iterator++) {
    for (it_type iteratorSourceCodeLineOperations =
         SourceCodeLineOperations.begin();
         iteratorSourceCodeLineOperations != SourceCodeLineOperations.end();
         iteratorSourceCodeLineOperations++) {
      if (iteratorSourceCodeLineOperations->first == iterator->first) {
       dbgs() << "Line "<< iterator->first << ":";
        for (std::set < uint64_t >::iterator it =
             iteratorSourceCodeLineOperations->second.begin();
             it != iteratorSourceCodeLineOperations->second.end(); ++it) {
         dbgs() << " " << getResourceName(*it);
        }
       dbgs() << "\n";
      }
    }
    
    map < uint64_t, vector < uint64_t > >::iterator it =
    SourceCodeLineInfoBreakdown.find (iterator->first);
    if (it == SourceCodeLineInfoBreakdown.end())
      report_fatal_error("Source code line not found\n");
    else {
      // Iterate over the vector
      for (unsigned i = 0; i < it->second.size(); i++) {
       dbgs() << " " << it->second[i];
      }
     dbgs() << "\n";
    }
   dbgs() << "__________________________________________________________\n";
  }
#endif
}




//===----------------------------------------------------------------------===//
//                              Class ACT
//===----------------------------------------------------------------------===//

void ACT::push_back(ACTNode * n, unsigned BitPosition)
{
  uint64_t i = n->key;
  uint64_t TreeChunk = i / SplitTreeRange;
  if (TreeChunk >= act_vec.size())
    act_vec.resize(TreeChunk + 1);
  bool cond = (n->issueOccupancy != 0);	// Add optional prefetch conditional
  if (cond)
    act_vec[TreeChunk].insert_node(n->key, BitPosition);
  delete n;
}


bool ACT::get_node_ACT (uint64_t key, unsigned BitPosition)
{
  uint64_t TreeChunk = key / SplitTreeRange;
  if (TreeChunk >= act_vec.size())
    return false;
  return act_vec[TreeChunk].get_node(key, BitPosition);
}


size_t ACT::size()
{
  return 0;
}


void ACT::clear()
{
  act_vec.clear();
}
