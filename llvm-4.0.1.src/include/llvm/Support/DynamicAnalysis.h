//=------------------- llvm/Support/DynamicAnalysis.h ------======= -*- C++ -*//
//
//                     The LLVM Compiler Infrastructure
//
//  Victoria Caparros Cabezas <caparrov@inf.ethz.ch>
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_DYNAMIC_ANALYSIS_H
#define LLVM_SUPPORT_DYNAMIC_ANALYSIS_H

#define INTERPRETER

//#define EFF_TBV

#include "../../../lib/ExecutionEngine/Interpreter/Interpreter.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include <boost/bimap.hpp>

#ifdef INTERPRETER
#include "llvm/Support/LinkedList.h"
#include "llvm/Support/top-down-size-splay.hpp"
#else
#include "LinkedList.h"
#include "top-down-size-splay.hpp"
#endif

#include <iostream>
#include <map>
#include <stdarg.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <cstdlib>
#include <limits>
#include <map>

#define LAST_INST 64
#define FP32_BLEND_INST 		LAST_INST+1
#define FP64_BLEND_INST 		LAST_INST+2
#define FP32_FMA_INST		LAST_INST+3
#define FP64_FMA_INST		LAST_INST+4
#define FP32_BOOL_INST		LAST_INST+5
#define FP64_BOOL_INST		LAST_INST+6
#define FP32_SHUFFLE_INST	LAST_INST+7
#define FP64_SHUFFLE_INST	LAST_INST+8

//#define INT_FP_OPS
///#define  SOURCE_CODE_ANALYSIS

#ifdef SOURCE_CODE_ANALYSIS
#include <unordered_map>
#endif
#include <deque>
#define ROUND_REUSE_DISTANCE
#define NORMAL_REUSE_DISTRIBUTION

//#define STACK_DEQUE

#define PRINT_OVERLAPS
//#define PRINT_ALL_OVERLAPS

#define DEBUG_MEMORY_TRACES
#define DEBUG_REUSE_DISTANCE
#define DEBUG_GENERIC



#define DEBUG_REGISTER_FILE
#define DEBUG_ISSUE_CYCLE
#define DEBUG_POINTERS_TO_MEMORY

#define DEBUG_WARM
//#define DEBUG_OOO_BUFFERS

#define DEBUG_SOURCE_CODE_LINE_ANALYSIS
//#define PRINT_DEPENDENCIES
//#define DEBUG_DEPS_FUNCTION_CALL
//#define DEBUG_OOO_BUFFERS
#define DEBUG_PHI_NODE

//#define ILP_DISTRIBUTION

//#define ASSERT


//TODO: Check when this is the case
//#define REUSE_STACK
#define VALUE_ANALYSIS
//#define STORES_IN_REGISTER

#define INTERMEDIATE_RESULTS_STACK

#define INF -1

// ========= Instructions considered in the analysis ===========================

#define INT_ADD          -1
#define INT_SUB          -1
#define INT_MUL          -1
#define INT_DIV          -1
#define INT_REM         -1

#define INT_LD_4_BITS    -1
#define INT_LD_8_BITS    -1
#define INT_LD_16_BITS   -1
#define INT_LD_32_BITS   -1
#define INT_LD_64_BITS   -1
#define INT_LD_80_BITS   -1
#define INT_LD_128_BITS  -1
#define INT_ST_4_BITS    -1
#define INT_ST_8_BITS    -1
#define INT_ST_16_BITS   -1
#define INT_ST_32_BITS   -1
#define INT_ST_64_BITS   -1
#define INT_ST_80_BITS   -1
#define INT_ST_128_BITS  -1


#define FP32_ADD           0
#define FP64_ADD           0
#define FP32_SUB           0
#define FP64_SUB           0
#define FP32_MUL           0
#define FP64_MUL           0
#define FP32_FMA           0
#define FP64_FMA           0
#define FP32_DIV           0
#define FP64_DIV           0
#define FP_REM          -1
#define FP_LD_16_BITS    1
#define FP_LD_32_BITS    1
#define FP_LD_64_BITS    1
#define FP_LD_80_BITS    1
#define FP_LD_128_BITS   1
#define FP_ST_16_BITS    1
#define FP_ST_32_BITS    1
#define FP_ST_64_BITS    1
#define FP_ST_80_BITS    1
#define FP_ST_128_BITS   1
#define MISC_MEM        -1
#define CTRL            -1
#define FP32_SHUFFLE       0
#define FP64_SHUFFLE       0
#define FP64_BOOL       	   0
#define FP32_BOOL           0
#define FP32_BLEND         0
#define FP64_BLEND         0

#define MISC_CONVERT_SELECT 2

#define MISC            -1


//===================== INSTRUCTION TYPES ========================//


enum {
  
  // Arithmetic computation nodes
  FP32_ADD_NODE = 0,
  FP64_ADD_NODE,
  
  FP32_MUL_NODE,
  FP64_MUL_NODE,
  
  FP32_FMA_NODE,
  FP64_FMA_NODE,
  
  FP32_DIV_NODE,
  FP64_DIV_NODE,
  
  // Vector nodes
  FP32_SHUFFLE_NODE,
  FP64_SHUFFLE_NODE,
  
  FP32_BLEND_NODE,
  FP64_BLEND_NODE,
  
  FP32_BOOL_NODE,
  FP64_BOOL_NODE,
  
  //Memory nodes
  REGISTER_LOAD_NODE,
  REGISTER_STORE_NODE,
  L1_LOAD_NODE,
  L1_STORE_NODE,
  L2_LOAD_NODE,
  FIRST_PREFETCH_LEVEL=L2_LOAD_NODE,
  L2_STORE_NODE,
  L3_LOAD_NODE,
  L3_STORE_NODE,
  MEM_LOAD_NODE,
  MEM_STORE_NODE,
  
  RS_STALL_NODE,
  ROB_STALL_NODE,
  LB_STALL_NODE,
  SB_STALL_NODE,
  LFB_STALL_NODE,
  
  AGU_NODE,
  STORE_AGU_NODE  = AGU_NODE,
  LOAD_AGU_NODE = AGU_NODE,
  
  PORT_0_NODE,
  PORT_1_NODE,
  PORT_2_NODE,
  PORT_3_NODE,
  PORT_4_NODE,
  PORT_5_NODE,
  
  L2_LOAD_PREFETCH_NODE,
  L2_STORE_PREFETCH_NODE,
  L3_LOAD_PREFETCH_NODE,
  L3_STORE_PREFETCH_NODE,
  MEM_LOAD_PREFETCH_NODE,
  MEM_STORE_PREFETCH_NODE,
  
  MISC_NODE,
  
  TOTAL_NODES
};


//==============================================================================
// EXECUTION UNITS: Execution units are all those resources for which we have a
// cycle inFullOccupacyTree
//
// IMPORTANT: If a new execution unit is added to the enumeration below, the
// corresponding latencies, throughput, parallel issue, etc. must be pushed back
// in the corresponding positions in DynamicAnalysis.cpp
//==============================================================================

enum {
  FP32_ADDER = 0,
  FP64_ADDER,
  
  FP32_MULTIPLIER,
  FP64_MULTIPLIER,
  
  FP32_FMADDER,
  FP64_FMADDER,
  
  FP32_DIVIDER,
  FP64_DIVIDER,
  
  FP32_SHUFFLE_UNIT,
  FP64_SHUFFLE_UNIT,
  
  FP32_BLEND_UNIT,
  FP64_BLEND_UNIT,
  
  FP32_BOOL_UNIT,
  FP64_BOOL_UNIT,
  
  REGISTER_LOAD_CHANNEL,
  REGISTER_STORE_CHANNEL = REGISTER_LOAD_CHANNEL,
  L1_LOAD_CHANNEL,
  L1_STORE_CHANNEL,
  L2_LOAD_CHANNEL,
  L2_STORE_CHANNEL = L2_LOAD_CHANNEL,
  L3_LOAD_CHANNEL,
  L3_STORE_CHANNEL = L3_LOAD_CHANNEL,
  MEM_LOAD_CHANNEL,
  MEM_STORE_CHANNEL = MEM_LOAD_CHANNEL,
  
  RS_STALL,
  ROB_STALL,
  LB_STALL,
  SB_STALL,
  LFB_STALL,
  
  ADDRESS_GENERATION_UNIT,
  STORE_ADDRESS_GENERATION_UNIT = ADDRESS_GENERATION_UNIT,
  LOAD_ADDRESS_GENERATION_UNIT = ADDRESS_GENERATION_UNIT,
  
  // This is a functional unit with 0 latency and infinity throughput,
  // that executed instructions such as select, fptosip.. Which we
  // need to be executed to track dependencies, but do not affect
  // the usual functional units.
  MISC_UNIT,
  
  // Ports must always be the last functional units because the number of ports
  // may increase.
  PORT_0,
  PORT_1,
  PORT_2,
  PORT_3,
  PORT_4,
  PORT_5,
  
  MAX_RESOURCE_VALUE
};



// ====================================================================//

#define EXECUTION_UNITS MEM_STORE_CHANNEL+1
#define NODES TOTAL_NODES+1

#define ARITHMETIC_NODES 8 // ( ADD (SUB), MUL, FMA, DIV )*2 -> SP and DP
#define ARITHMETIC_EXECUTION_UNITS 8

#define MOV_NODES 6
#define MOV_EXECUTION_UNITS 6


#define MEM_NODES 10 // Before 8, we add 1 for register file size
#define MEM_EXECUTION_UNITS 6  // Before 5

#define MISC_EXECUTION_UNITS 1

#define DISPATCH_PORTS 6
#define BUFFERS 5
#define AGUS 2
#define LOAD_AGUS 0
#define STORE_AGUS 0
#define PREFETCH_NODES 3


#define N_TOTAL_NODES N_COMP_NODES + N_MEM_NODES + N_BUFFER_NODES + \\
                      N_PREFETCH_RESOURCES + N_MISC_RESOURCES
#define N_MEM_RESOURCES_START L1_LOAD_CHANNEL
#define N_MEM_RESOURCES_END MEM_STORE_CHANNEL


using namespace llvm;
using namespace std;
using namespace SplayTree;
using namespace SplayTreeBoolean;
using namespace SimpleSplayTree;
using namespace ComplexSplayTree;

// For FullOccupancyCyles, the vector has a different meaning that for
// AvailableCycles. Each element of the vector contains the elements of the tree
// in a corresponding rage.
static const unsigned SplitTreeRange = 131072;




// =============================================================================
//  Data structures definitions for tracking pointer to memory
//==============================================================================

struct PointerToMemory{
  Value * BasePointer;
  Value * Offset1;
  Value * Offset2;
  Value * Offset3;
  Value * Offset4;
  Value * Offset5;
};

struct PointerToMemoryInstance{
  PointerToMemory PTM;
  unsigned Rep;
  int64_t IterationCount;
};


struct InstructionValue{
  Value * v;
  unsigned valueRep;
};


bool operator <(const PointerToMemory& x, const PointerToMemory& y);
bool operator <(const PointerToMemoryInstance& x,
                const PointerToMemoryInstance& y);



// =============================================================================
//  End of data structures definitions for tracking pointer to memory
//==============================================================================

struct CacheLineInfo{
  uint64_t IssueCycle;
  uint64_t LastAccess;
};


struct InstructionDispatchInfo{
  uint64_t IssueCycle;
  uint64_t CompletionCycle;
};


struct LessThanOrEqualValuePred
{
  uint64_t CompareValue;
  
  bool operator()(const uint64_t Value) const
  {
    return Value <= CompareValue;
  }
};

struct StructMemberLessThanOrEqualThanValuePred
{
  const uint64_t CompareValue;
  
  bool operator()(const InstructionDispatchInfo& v) const
  {
    return v.CompletionCycle <= CompareValue;
  }
};


class TBV_node {
  public:
#ifdef EFF_TBV
  TBV_node():BitVector(SplitTreeRange) {
  }
  bool e;
#else
  TBV_node():BitVector(MAX_RESOURCE_VALUE) {
  }
#endif
  dynamic_bitset<> BitVector; // from boost
#ifdef SOURCE_CODE_ANALYSIS
  vector<pair<unsigned,unsigned>> SourceCodeLinesOperationPair;
#endif
  
  bool get_node(uint64_t bitPosition);
  void insert_node(uint64_t bitPosition);
  bool get_node_nb(uint64_t bitPosition);
  bool empty();
  
};


class TBV {
  
  public:
  
  vector<TBV_node> tbv_map;
  bool e;
  
  
  TBV();
  void insert_source_code_line(uint64_t key, unsigned SourceCodeLine,
                               unsigned Resource);
  vector<pair<unsigned,unsigned> >  get_source_code_lines(uint64_t key);
  bool get_size();
  void resize();
  
  bool get_node(uint64_t key, unsigned bitPosition);
  void insert_node(uint64_t key, unsigned bitPosition);
  
  bool get_node_nb(uint64_t key, unsigned bitPosition);
  
  void delete_node(uint64_t key, unsigned bitPosition);
  bool empty();
};




struct ACTNode {
  public:
  uint64_t key;
  int32_t issueOccupancy;
  int32_t widthOccupancy;
  int32_t occupancyPrefetch;
  uint64_t address;
};

class ACT {
  private:
  vector< TBV> act_vec;
  
  public:
  bool get_node_ACT(uint64_t, unsigned);
  void push_back(ACTNode*, unsigned);
  void DebugACT();
  size_t size();
  void clear();
};



uint64_t BitScan(vector< TBV> &FullOccupancyCyclesTree,
                 uint64_t key,
                 unsigned bitPosition);


// =============================================================================
//                      Class DynamicAnalysis
//==============================================================================
class DynamicAnalysis {
  
  public:
  
  // Variables that define the number of nodes of each type in the performance
  // model, and execution resources in the high-level microarchitecture model.
  string Microarchitecture;
  
  unsigned NTotalResources;
  
  unsigned NExecutionUnits;
  unsigned NArithmeticExecutionUnits;
  unsigned NMovExecutionUnits;
  unsigned NMemExecutionUnits;
  unsigned NMiscExecutionUnits;
  
  unsigned NArithmeticNodes;
  unsigned NMovNodes;
  unsigned NMemNodes;
  unsigned NNodes;
  
  unsigned NPorts;
  unsigned NBuffers;
  unsigned NAGUs;
  unsigned NLoadAGUs;
  unsigned NStoreAGUs;
  
  unsigned MemoryWordSize;
  unsigned CacheLineSize;
  unsigned RegisterFileSize;
  
  //Cache sizes are specified in number of cache lines of size CacheLineSize
  unsigned L1CacheSize;
  unsigned L2CacheSize;
  unsigned LLCCacheSize;
  
  unsigned BitsPerCacheLine;
  
  unsigned ReservationStationSize;
  unsigned ReorderBufferSize;
  unsigned LoadBufferSize;
  unsigned StoreBufferSize;
  unsigned LineFillBufferSize;
  
  int AddressGenerationUnits;
  int InstructionFetchBandwidth;
  
  
  //For every node, the execution unit in which it executes.
  vector<unsigned> ExecutionUnit;
  vector<vector<unsigned> > DispatchPort;
  vector<string> NodesNames;
  // For every execution unit, latency, throughput...
  vector<unsigned> ExecutionUnitsLatency;
  vector<double> ExecutionUnitsThroughput;
  vector<int> ExecutionUnitsParallelIssue;
  vector<unsigned> IssueCycleGranularities;
  vector<unsigned> AccessWidths;
  vector<bool> ShareThroughputAmongPorts;
  unsigned MaxLatencyResources;
  vector<unsigned > AccessGranularities;
  vector<string> ResourcesNames;
  


  
  deque<uint64_t> CacheLinesHistory;
  
  bool SmallBuffers;
  
  bool DebugWarm;
  
  
  
  bool RARDependences;
  bool WarmCache;
  bool x86MemoryModel;
  bool ARMMemoryModel;
  bool SpatialPrefetcher;
  bool ConstraintPorts;
  bool ConstraintPortsx86;
  bool ConstraintPortsARM;
  bool ConstraintAGUs;
  bool FloatPrecision;
  bool VectorCode;
  unsigned VectorWidth;
  unsigned PrefetchLevel;
  unsigned PrefetchDispatch;
  unsigned PrefetchTarget;
  unsigned PrefetchDestination;
  
  bool InOrderExecution;
  bool ReportOnlyPerformance;
  
  
  
  string TargetFunction;
  uint8_t FunctionCallStack;
  
  
  
  int rep;
  
  // ===========================================================================
  // Variables to track instructions count (for each type of nodes/instructions)
  // ===========================================================================
  uint64_t TotalInstructions;
  vector<uint64_t> InstructionsCount;
  vector<uint64_t> InstructionsCountExtended;
  vector<uint64_t> ScalarInstructionsCountExtended;
  vector<uint64_t> VectorInstructionsCountExtended;
  vector<uint64_t> InstructionsLastIssueCycle;
  vector<uint64_t> IssueSpan;
  vector<uint64_t> LatencyOnlySpan;
  vector<uint64_t> SpanGaps;
  vector<uint64_t> FirstNonEmptyLevel;
  vector<uint64_t> BuffersOccupancy;
  vector<uint64_t> LastIssueCycleVector;
  vector<double> AverageOverlapsCycles;
  vector<uint64_t> OverlapsCount;
  vector<double> AverageOverlaps;
  vector<double> OverlapsDerivatives;
  vector<double> OverlapsMetrics;
  
  vector<unsigned> MaxOccupancy;
  vector<bool> FirstIssue;
  
  uint64_t LastLoadIssueCycle;
  uint64_t LastStoreIssueCycle;
  uint64_t LastInstructionIssueCycle;
  uint64_t LastIssueCycleFinal;
  
  unsigned NRegisterSpillsLoads;
  unsigned NRegisterSpillsStores;
  
  uint64_t GlobalAddrForArtificialMemOps;

  // ===========================================================================
  // Variables to track the scheduling of instructions
  // ===========================================================================
  
  // BasicBlockBarrier was defined to prevent instructions from being scheduled
  // before the first instruction in the BB. This variable is set to 0 and this
  // functionality is not implemented anymore
  uint64_t BasicBlockBarrier;
  int64_t RemainingInstructionsFetch;
  uint64_t InstructionFetchCycle;
  
  vector<uint64_t> ReservationStationIssueCycles;
  deque<uint64_t> ReorderBufferCompletionCycles;
  vector<uint64_t> LoadBufferCompletionCycles;
  SimpleTree<uint64_t> *LoadBufferCompletionCyclesTree;
  vector<uint64_t> StoreBufferCompletionCycles;
  vector<uint64_t> LineFillBufferCompletionCycles;
  vector<InstructionDispatchInfo> DispatchToLoadBufferQueue;
  ComplexTree<uint64_t> *DispatchToLoadBufferQueueTree;
  vector<pair<uint64_t,uint64_t> > DispatchToLoadBufferQueueTreeCyclesToRemove;
  Value * PrevBB;
  vector<InstructionDispatchInfo> DispatchToStoreBufferQueue;
  vector<InstructionDispatchInfo> DispatchToLineFillBufferQueue;
  
  vector< Tree<uint64_t> * > AvailableCyclesTree;
  
#ifdef EFF_TBV
  vector< TBV_node> FullOccupancyCyclesTree;
#else
  vector< TBV> FullOccupancyCyclesTree;
#endif
  vector <Tree<uint64_t> * > StallCycles;
  vector <uint64_t> NInstructionsStalled;
  
  uint64_t MinLoadBuffer;
  uint64_t MaxDispatchToLoadBufferQueueTree;
  
  vector<unsigned> IssuePorts;
  
  // ===========================================================================
  // Data structures for tracking value analysis
  // ===========================================================================
  
  map <PointerToMemoryInstance, PointerToMemoryInstance> PointerToMemoryInstanceMap;
  typedef map<PointerToMemoryInstance, PointerToMemoryInstance> ::iterator PointerToMemoryInstanceMapIterator;
  
  map <PointerToMemoryInstance, uint64_t> PointerToMemoryInstanceNUsesMap;
  typedef map<PointerToMemoryInstance, uint64_t> ::iterator
                                        PointerToMemoryInstanceNUsesMapIterator;
  
  typedef boost::bimap< PointerToMemoryInstance, uint64_t > bm_type;
  bm_type PointerToMemoryInstanceAddressBiMap;
  
  map <InstructionValue, int64_t> InstructionValueMap;
  
  set<uint64_t> SpilledCacheLine;
  vector<uint64_t> SpilledAddress;
  
  map <Value*, Value*> InstructionValueInstructionNameMap;
  map <Value*, uint64_t> InstructionValueIssueCycleMap;
  map <uint64_t , CacheLineInfo> CacheLineIssueCycleMap;
  map <uint64_t , uint64_t> MemoryAddressIssueCycleMap;
  
#ifdef INTERMEDIATE_RESULTS_STACK
#ifdef STACK_DEQUE
  deque<PointerToMemoryInstance> ReuseStack;
#else
  LinkedList<PointerToMemoryInstance> ReuseStack;
#endif
#else
  deque<uint64_t> ReuseStack;
#endif
  
  
  // ===========================================================================
  // Variables for reuse distance analysis
  // ===========================================================================
  Tree<uint64_t> * ReuseTree;
  Tree<uint64_t> * PrefetchReuseTree;
  uint64_t PrefetchReuseTreeSize;
  map<int,int> ReuseDistanceDistribution;
  map<int,int> RegisterReuseDistanceDistribution;
  map<int,map<uint64_t,uint> > ReuseDistanceDistributionExtended;
  
  
  // ILP
  vector< vector<unsigned> > ParallelismDistribution;
  
  // ===========================================================================
  // Variables for source code analuysis
  // ===========================================================================
  unsigned SourceCodeLine;

#ifdef SOURCE_CODE_ANALYSIS
  map<uint64_t,set<uint64_t> > SourceCodeLineOperations;
  map<uint64_t,set<uint64_t> > SourceCodeLineInfo;
  map<uint64_t,vector<uint64_t> > SourceCodeLineInfoBreakdown;
#endif

  // Output dir where to dump data
  string OutputDir;


  // ===========================================================================
  // From Contech (): efficiently calculating span using bitvectors.
  // ===========================================================================
  vector< dynamic_bitset<> > CGSFCache;
  vector< dynamic_bitset<> > CISFCache;
  vector< dynamic_bitset<> > CLSFCache;
  
  vector<vector<float> > BnkMat;
  ACT ACTFinal;

  
  //============================================================================
  // Constructor
  //============================================================================
  
  DynamicAnalysis(string TargetFunction,
                  string Microarchitecture,
                  unsigned MemoryWordSize,
                  unsigned CacheLineSize,
                  unsigned RegisterFileSize,
                  unsigned L1CacheSize,
                  unsigned L2CacheSize,
                  unsigned LLCCacheSize,
                  vector<float> ExecutionUnitsLatency,
                  vector<double> ExecutionUnitsThroughput,
                  vector<int> ExecutionUnitsParallelIssue,
                  vector<unsigned>  MemAccessGranularity,
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
                  unsigned VectorWidth);
  

    void printILPDistribuion (unsigned span);
  
  // ===========================================================================
  // 	Funtions for tracking pointers to memory - Implemented in ValuesAnalysis
  //============================================================================
  bool insertUsesOfPointerToMemory(Value * v, PointerToMemoryInstance PTMI,
                                   bool recursiveCall = false);
  uint64_t getNextArtificialAddress();
  
  void
  updateAssociatedPointerToMemoryInstance(PointerToMemoryInstance instructionPTMI,
                                          PointerToMemoryInstance associatedPTMI);
  
  void
  insertAssociatedPointerToMemoryInstance(PointerToMemoryInstance instructionPTMI,
                                          PointerToMemoryInstance associatedPTMI);
  
  PointerToMemoryInstance
  managePointerToMemory(PointerToMemoryInstance instructionPTMI, Instruction & I,
                        unsigned valueRep, int64_t valueInstance, uint64_t & addr,
                        unsigned OpCode, bool WarmRun, bool forceAnalyze);
  
  void check();
  
  void removeUnusedSpilledCacheLinesFromReuseTree();
  
  uint64_t adjustMemoryAddress(uint64_t addr, uint64_t addrFound,
                               PointerToMemoryInstance duplicatedPTMI,
                               bool forceAnalyze);
  void resetInstructionValueMap();
  PointerToMemoryInstance getPointerToMemoryInstance(uint64_t address);
  
  int64_t getInstructionValueInstance(InstructionValue instValue);
  
  void increaseInstructionValueInstance(InstructionValue v);
  
  PointerToMemoryInstance
  insertIntermediateResultInRegisterStack(PointerToMemoryInstance PTMI,
                                          Instruction & currentInstruction,
                                          unsigned valueRep,  bool WarmRun,
                                          bool isSpill = false);
  
  int checkRegisterStackReuseDistance(PointerToMemoryInstance address);
  int registerStackReuseDistance(PointerToMemoryInstance address,
                                 Instruction & CurrentInst, bool WarmRun,
                                 bool isSpill);
  void insertRegisterStack(PointerToMemoryInstance address,
                           Instruction & CurrentInst, unsigned valueRep,
                           bool warmRun);
  void removeRegisterStack(PointerToMemoryInstance address);
  
  
  void increaseNUses(PointerToMemoryInstance PTMI);
  void decreaseNUses(PointerToMemoryInstance PTMI);
  
  uint64_t adjustMemoryAddress(PointerToMemory v, uint64_t addr,
                               bool forceAnalyze);
  void getOriginalIncomingEdgesPhiNode(PHINode * phiNode,
                                       vector<Value *>&  originalIncomingEdges);
  
  void printInstructionValue(InstructionValue IV);
  void printPointerToMemory (PointerToMemory ptrmem);
  void printPointerToMemoryGlobalVector();
  void printPointerToMemoryInstanceMap();
  void printPointerToMemoryInstance(PointerToMemoryInstance PTMI);
  void printBiMap();
  void printRegisterStack();
  
  // =========================================================================//
  // 	OTHER FUNCTIONS
  //===========================================================================//
  
  string getResourceName(unsigned Resource);
  
  string getNodeName(unsigned Node);
  
  int getInstructionType(Instruction &I);
  
  unsigned getLastRepetitionIntrinsic(string functionName);
  unsigned getLastNonMemRepetitionIntrinsic(string functionName);
  int64_t getStoreOperandPositionIntrinsic(string functionName);

  void getOperandsPositionsIntrinsic(string functionName,
                                     vector<int64_t> & positions,
                                     unsigned valueRep);

  uint64_t getInstructionValueIssueCycle(Value* v);
  void insertInstructionValueIssueCycle(Value* v,uint64_t InstructionIssueCycle,
                                        bool isPHINode = 0 );
  void insertInstructionValueName(Value * v);
  void printInstructionValueNames();
  unsigned getMemoryInstructionType(int ReuseDistance, uint64_t MemoryAddress,
                                    bool isLoad = true);
  unsigned getExtendedInstructionType(Instruction &I, int OpCode,
                                      int ReuseDistance = 0,
                                      int RegisterStackReuseDistance = -1);
  
  //===----------------------------------------------------------------------===//
  //        Routines for handling memory addresses and cache lines
  //===----------------------------------------------------------------------===//
  CacheLineInfo getCacheLineInfo(uint64_t v);
  uint64_t getCacheLineLastAccess(uint64_t v);
  uint64_t getMemoryAddressIssueCycle(uint64_t v);

  void insertCacheLineInfo(uint64_t v,CacheLineInfo Info );
  void insertCacheLineLastAccess(uint64_t v,uint64_t LastAccess );
  void insertMemoryAddressIssueCycle(uint64_t v,uint64_t Cycle );

  //===----------------------------------------------------------------------===//
  //        Routines for managing bandwidth and throughput, and for
  //        scheduling a node in the DAG
  //===----------------------------------------------------------------------===//
#ifdef EFF_TBV
  void getTreeChunk(uint64_t i, unsigned int ExecutionResource);
#else
  uint64_t getTreeChunk(uint64_t i);
#endif

  unsigned
  findNextAvailableIssueCyclePortAndThroughtput(unsigned InstructionIssueCycle,
                                                unsigned ExtendedInstructionType,
                                                unsigned NElementsVector=1);
  
  unsigned getNElementsAccess(unsigned ExecutionResource,
                              unsigned AccessWidth, unsigned nElementsVector);
  
  unsigned getIssueCycleGranularity(unsigned ExecutionResource,
                                    unsigned AccessWidth,
                                    unsigned NElementsVector);
  
  float getEffectiveThroughput(unsigned ExecutionResource, unsigned AccessWidth,
                               unsigned NElementsVector);
  
  unsigned getNodeWidthOccupancy(unsigned ExecutionResource, unsigned AccessWidth,
                                 unsigned NElementsVector);
  
  bool getLevelFull(unsigned ExecutionResource, unsigned AccessWidth,
                    unsigned NElementsVector, unsigned NodeIssueOccupancy,
                    unsigned NodeWidthOccupancy, bool potentiallyFull = false);
  
  bool thereIsAvailableBandwidth(unsigned NextAvailableCycle,
                                 unsigned ExecutionResource,
                                 unsigned NElementsVector,
                                 bool& FoundInFullOccupancyCyclesTree,
                                 bool TargetLevel);
  
  uint64_t
  findNextAvailableIssueCycleUntilNotInFullOrEnoughBandwidth(unsigned NextCycle,
                                                             unsigned ExecutionResource,
                                                             bool&FoundInFullOccupancyCyclesTree,
                                                             bool& EnoughBandwidth);
  
  // Returns the DAG level occupancy after the insertion
  unsigned findNextAvailableIssueCycle(unsigned OriginalCycle,
                                       unsigned ExecutionResource,
                                       uint8_t NElementsVector = 1,
                                       bool TargetLevel = true);
  
  bool insertNextAvailableIssueCycle(uint64_t NextAvailableCycle,
                                     unsigned ExecutionResource,
                                     unsigned NElementsVector = 1,
                                     int IssuePort = -1, bool isPrefetch = 0);
  
  void increaseInstructionFetchCycle(bool EmptyBuffers = false);
  
  
  //===----------------------------------------------------------------------===//
  //                Routines for Analysis of Reuse Distance
  //===----------------------------------------------------------------------===//
  
#ifdef INTERMEDIATE_RESULTS_STACK
  PointerToMemoryInstance insertOperandsInRegisterStack(int operandPosition,
                                                        Instruction & I,
                                                        unsigned rep,
                                                        bool WarmCache,
                                                        unsigned Line);
  void removeOperandsInRegisterStack(unsigned i, Instruction & I, bool WarmRun,
                                     unsigned Line);
#else
  int registerStackReuseDistance(uint64_t address, bool WarmCache,
                                 bool insert = true);
  void insertRegisterStack(uint64_t address);
#endif
  
  int ReuseDistance(uint64_t Last, uint64_t Current, uint64_t address,
                    bool FromPrefetchReuseTree = false);
  int reuseTreeSearchDelete(uint64_t Current, uint64_t address,
                            bool FromPrefetchReuseTree = false);
  void updateReuseDistanceDistribution(int Distance,
                                       uint64_t InstructionIssueCycle);
  void updateRegisterReuseDistanceDistribution(int Distance);
  
  //===----------------------------------------------------------------------===//
  //        Routine to schedule a node in the DAG
  //===----------------------------------------------------------------------===//
  
  void analyzeInstruction(Instruction &I, unsigned OpCode, uint64_t addr,
                           unsigned SourceCodeLine = 0 ,
                           bool forceAnalyze = false, unsigned VectorWidth = 1,
                           unsigned valueRep = 0, bool lastValue = true,
                           bool firstValue = true, bool isSpill = false);
  
  
  


  
  
  
  //===---------------------------------------------------------------------===//
  //               Routines for span analysis
  //===---------------------------------------------------------------------===//
  uint64_t getLastIssueCycle(unsigned ExecutionResource,
                             bool WithPrefetch = false);

  unsigned calculateIssueSpanFinal(vector<int> & ResourcesVector,
                                   bool onlyScalar = false);
  
  uint64_t calculateSpanFinal(int ResourceType);
  
  bool isEmptyLevelFinal(unsigned ExecutionResource, uint64_t Level);

  unsigned calculateLatencyOnlySpanFinal(unsigned i);
  unsigned getGroupSpanFinal(vector<int> & ResourcesVector);
  unsigned getGroupOverlapCyclesFinal(vector<int> & ResourcesVector);
  unsigned getOneToAllOverlapCyclesFinal(vector < int >&ResourcesVector);
  unsigned getOneToAllOverlapCyclesFinal(vector < int >&ResourcesVector,
                                         bool Issue);
  unsigned calculateGroupSpanFinal(vector<int> & ResourcesVector);

  void computeAvailableTreeFinalHelper(uint p, Tree<uint64_t>* t, uint d);
  void computeAvailableTreeFinal();

  


  
  //===----------------------------------------------------------------------===//
  //                    Source code analysis routines
  //===----------------------------------------------------------------------===//
  
  unsigned getPositionSourceCodeLineInfoVector(uint64_t Resource);
  
  void collectSourceCodeLineStatistics(uint64_t ResourceType, uint64_t Cycle,
                                       uint64_t MaxLatencyLevel,
                                       uint64_t SpanIncrease,
                                       uint64_t IssueCycleGranularity,
                                       bool IsInAvailableCyclesTree);
  
  //===----------------------------------------------------------------------===//
  //                      OoO Buffers routines
  //===----------------------------------------------------------------------===//
  
  uint64_t getMinIssueCycleReservationStation();
  uint64_t getMinCompletionCycleLoadBuffer();
  uint64_t getMinCompletionCycleLoadBufferTree();
  uint64_t getMinCompletionCycleStoreBuffer();
  uint64_t getMinCompletionCycleLineFillBuffer();
  
  void removeFromReservationStation(uint64_t Cycle);
  void removeFromReorderBuffer(uint64_t Cycle);
  void removeFromLoadBuffer(uint64_t Cycle);
  void removeFromLoadBufferTree(uint64_t Cycle);
  void removeFromStoreBuffer(uint64_t Cycle);
  void removeFromLineFillBuffer(uint64_t Cycle);
  void removeFromDispatchToLoadBufferQueue(uint64_t Cycle);
  void removeFromDispatchToStoreBufferQueue(uint64_t Cycle);
  void removeFromDispatchToLineFillBufferQueue(uint64_t Cycle);
  
  void inOrder(uint64_t i, ComplexTree<uint64_t> * n);
  
  void dispatchToLoadBuffer(uint64_t Cycle);
  void dispatchToLoadBufferTree(uint64_t Cycle);
  void dispatchToStoreBuffer(uint64_t Cycle);
  void dispatchToLineFillBuffer(uint64_t Cycle);
  
  uint64_t findIssueCycleWhenLoadBufferIsFull();
  uint64_t findIssueCycleWhenLoadBufferTreeIsFull();
  uint64_t findIssueCycleWhenStoreBufferIsFull();
  uint64_t findIssueCycleWhenLineFillBufferIsFull();
  
  

  
  //===----------------------------------------------------------------------===//
  //                  Some auxiliary routines
  //===----------------------------------------------------------------------===//
  
  unsigned int roundNextPowerOfTwo(unsigned int v);
  uint64_t roundNextMultiple(uint64_t num, int multiple);
  

  
  
  void finishAnalysisContechSimplified();
  
  
  void printHeaderStat(string Header);
  
  void dumpList(std::list< double > const & l, string const & filename);
  
  // PREFECTH ARM
  void insertCacheLineHistory(uint64_t CacheLine);
  bool cacheLineRecentlyAccessed(uint64_t CacheLine);
  
}; // End of class DynamicAnalysis
#endif
