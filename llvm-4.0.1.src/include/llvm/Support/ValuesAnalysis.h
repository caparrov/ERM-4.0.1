//=-------------------- llvm/Support/ValuesAnalysis.h ------======= -*- C++ -*//
//
//                     The LLVM Compiler Infrastructure
//
//  Victoria Caparros Cabezas <caparrov@inf.ethz.ch>
//===----------------------------------------------------------------------===//
#ifndef LLVM_SUPPORT_VALUES_ANALYSIS_H
#define LLVM_SUPPORT_VALUES_ANALYSIS_H

#ifdef INTERPRETER
#include "llvm/Support/DynamicAnalysis.h"
#else
#include "DynamicAnalysis.h"
#endif


bool operator <(const InstructionValue& x, const InstructionValue& y)
{
  return std::tie(x.v, x.valueRep) < std::tie(y.v, y.valueRep);
}


bool operator== ( PointerToMemory a, PointerToMemory b ){
  return std::tie(a.BasePointer, a.Offset1, a.Offset2, a.Offset3, a.Offset4,
                  a.Offset5) == std::tie(b.BasePointer, b.Offset1,
                                         b.Offset2,b.Offset3, b.Offset4,
                                         b.Offset5);
}


bool operator< (const PointerToMemory& a, const PointerToMemory& b )
{
  return std::tie(a.BasePointer, a.Offset1, a.Offset2, a.Offset3, a.Offset4,
                  a.Offset5) < std::tie(b.BasePointer, b.Offset1, b.Offset2,
                                        b.Offset3, b.Offset4, b.Offset5);
}


bool operator== ( PointerToMemoryInstance a, PointerToMemoryInstance b )
{
  return std::tie(a.PTM,  a.Rep, a.IterationCount) == std::tie(b.PTM,  b.Rep,
                                                               b.IterationCount);
}


bool operator <(const PointerToMemoryInstance& x, const PointerToMemoryInstance& y)
{
  return std::tie(x.PTM, x.Rep,x.IterationCount) < std::tie(y.PTM, y.Rep,
                                                            y.IterationCount);
}


void
DynamicAnalysis::printInstructionValue(InstructionValue IV)
{
  DEBUG(dbgs() << IV.v << ", rep "<< IV.valueRep <<"\n");
}


void
DynamicAnalysis::printPointerToMemoryInstance(PointerToMemoryInstance PTMI)
{
  printPointerToMemory(PTMI.PTM);
  DEBUG(dbgs() << ", "<< PTMI.Rep);
  DEBUG(dbgs() << ", "<< PTMI.IterationCount);
}


void
DynamicAnalysis::printPointerToMemory(PointerToMemory PTM)
{
  DEBUG(dbgs() << PTM.BasePointer <<" " <<  PTM.Offset1 << " " << PTM.Offset2 <<
        " " << PTM.Offset3<< " " << PTM.Offset4<< " " << PTM.Offset5);
}


void
DynamicAnalysis::printPointerToMemoryInstanceMap()
{
  DEBUG(dbgs() << "--------PointerToMemoryInstanceMap --------:\n");
  PointerToMemoryInstanceMapIterator it;
  for(it = PointerToMemoryInstanceMap.begin(); it !=
      PointerToMemoryInstanceMap.end(); it++){
    printPointerToMemoryInstance((*it).first);
    DEBUG(dbgs() << ", Associated PTMI: ");
    printPointerToMemoryInstance((*it).second);
    DEBUG(dbgs() << "\n");
  }
  DEBUG(dbgs() << "--------PointerToMemoryInstanceMap --------:\n");
  DEBUG(dbgs() << "Total elements: " << PointerToMemoryInstanceMap.size() << "\n");
}


void
DynamicAnalysis::increaseNUses(PointerToMemoryInstance PTMI)
{
  PointerToMemoryInstanceNUsesMapIterator it =
  PointerToMemoryInstanceNUsesMap.find(PTMI);
  if(it== PointerToMemoryInstanceNUsesMap.end()){
    PointerToMemoryInstanceNUsesMap[PTMI] = 1;
  }else{
    PointerToMemoryInstanceNUsesMap[PTMI]++;
  }
}


void
DynamicAnalysis::decreaseNUses(PointerToMemoryInstance PTMI)
{
  PointerToMemoryInstanceNUsesMap[PTMI]--;
}


PointerToMemoryInstance
DynamicAnalysis::insertIntermediateResultInRegisterStack(PointerToMemoryInstance PTMI,
                                                         Instruction & currentInstruction,
                                                         unsigned valueRep,
                                                         bool WarmRun,
                                                         bool isSpill)
{
  insertAssociatedPointerToMemoryInstance(PTMI, PTMI);
  // If operand not in the stack, trigger a load.
  //The load will insert in in the stack
  int RegisterStackDistance = registerStackReuseDistance(PTMI,currentInstruction,
                                                         WarmRun, isSpill);
  if(RegisterStackDistance < 0){
    insertRegisterStack(PTMI, currentInstruction, valueRep,  WarmRun);
  }else{
    if(!WarmRun)
    updateRegisterReuseDistanceDistribution(RegisterStackDistance);
  }
  return PTMI;
}


void
DynamicAnalysis::getOriginalIncomingEdgesPhiNode(PHINode * phiNode,
                                                 vector<Value *>& originalIncomingEdges){
  
  vector<Value *> tmpOriginalIncomingEdges;
  unsigned nIncomingEdges = phiNode->getNumIncomingValues();
  bool samePHiNodeFound = false;
  
  for(unsigned i = 0; i< nIncomingEdges; i++){
    Value * incomingEdge =  phiNode->getIncomingValue(i);
    samePHiNodeFound = false;
    // Insert if it is not there already
    for(unsigned j = 0; j< originalIncomingEdges.size();j++){
      if(incomingEdge == originalIncomingEdges.at(j)){
        samePHiNodeFound = true;
        break;
      }
    }
    if(samePHiNodeFound == false){
      originalIncomingEdges.push_back(incomingEdge);
      if(PHINode * PN = dyn_cast<PHINode>(incomingEdge))
      getOriginalIncomingEdgesPhiNode(PN,originalIncomingEdges);
    }
  }
  if(originalIncomingEdges.size()==0)
  report_fatal_error("The number of incoming edges of a PHI node cannot be \
                     zero");
}


// Every operand has been an intermediate result
PointerToMemoryInstance
DynamicAnalysis::insertOperandsInRegisterStack(int i, Instruction & I,
                                               unsigned rep, bool WarmRun,
                                               unsigned Line)
{
  unsigned operandRepetition = rep;
  bool constantOperand = false;
  bool argumentOperand = false;
  InstructionValue instructionValueOperand;
  PointerToMemory operandPTM;
  PointerToMemoryInstance operandPTMI;
  PointerToMemoryInstance associatedPTMI =
  {{NULL,NULL,NULL,NULL, NULL, NULL},0,-1};
  int64_t operandValueInstance;
  vector<PointerToMemoryInstance> associatedPTMIvector ;
  vector<unsigned> associatedPTMIindexesvector ;
  
  // Prepare the proper operand repetition (0 by default unless the operand was an
  // intrinsic) and operand value instance
  if(i < 0){
    if(operandRepetition > 0){
      operandRepetition = operandRepetition -1;
    }else{
      if(operandRepetition == 0){
        if(CallInst *CI = dyn_cast<CallInst> (&I)){
          Function * f = CI->getCalledFunction();
          operandRepetition = getLastNonMemRepetitionIntrinsic (f->getName());
        }else{
          report_fatal_error("If operand position is -1, must be an special \
                             case - intrinsic");
        }
      }
    }
    instructionValueOperand = {&I,operandRepetition};
    operandValueInstance = getInstructionValueInstance(instructionValueOperand);
    // We have to decrease instance because instance was increased after
    // the execution of the corresponding instruction => need to search for the
    // previous one.
    if(operandValueInstance == 1)
    operandValueInstance = -1;
    else
    operandValueInstance = operandValueInstance-1;
    
    operandPTM = {&I,NULL, NULL, NULL, NULL, NULL};
    operandPTMI = {operandPTM,operandRepetition,operandValueInstance};
    associatedPTMI = operandPTMI;
  }else{
    if(dyn_cast<Constant> (I.getOperand(i)))
    constantOperand = true;
    if(dyn_cast<Argument> (I.getOperand(i)))
    argumentOperand = true;
    
    if(!constantOperand && !argumentOperand){
      if(PHINode * PHI = dyn_cast<PHINode>(I.getOperand(i))){
        // ====================================================================
        //                 OPERAND IS A PHI NODE
        //=====================================================================
        // If what I want to insert is a PHI node, do not insert the value of
        // the phi node, but the value of the definition of the phi node.
        
        bool prevInstanceFound = false;
        bool constantIncomingEdge = false;
        PointerToMemory candidatePTM;
        PointerToMemoryInstance candidatePTMI;
        vector<Value *> originalIncomingEdges;
        
        getOriginalIncomingEdgesPhiNode(PHI,originalIncomingEdges);
        // Remove all the PHI nodes from originalIncomingEdges
        for (unsigned i = 0; i< originalIncomingEdges.size(); i++){
          if(dyn_cast<PHINode> (originalIncomingEdges.at(i)))
          originalIncomingEdges.erase(originalIncomingEdges.begin() + i);
        }
        
        for (unsigned i = 0; i< originalIncomingEdges.size(); i++){
          if(dyn_cast<Constant> (originalIncomingEdges.at(i))){
            constantIncomingEdge = true;
          }else{
            int64_t originalIncomingEdgeInstance =
            getInstructionValueInstance({originalIncomingEdges.at(i),0});
            // If originalIncomingEdgeInstance == -1, the incoming edge has not
            // appeared yet, so do not consider
            if(originalIncomingEdgeInstance != -1){
              prevInstanceFound = true;
              // We have to decrease instance because instance was increased
              // after the execution of the corresponding instruction => need to
              // search for the previous one.
              if(originalIncomingEdgeInstance == 1)
              originalIncomingEdgeInstance = -1;
              else
              originalIncomingEdgeInstance = originalIncomingEdgeInstance-1;
              
              candidatePTM =
              {originalIncomingEdges.at(i), NULL, NULL, NULL, NULL, NULL};
              candidatePTMI = {candidatePTM, 0, originalIncomingEdgeInstance};
              
              PointerToMemoryInstanceMapIterator it =
              PointerToMemoryInstanceMap.find(candidatePTMI);
              if(it == PointerToMemoryInstanceMap.end()){
                report_fatal_error("Every operand should have an entry in \
                                   PointerToMemoryInstanceMap??");
              }else
              associatedPTMIvector.push_back(it->second);
            }
          }
        }
        // The instance of the candidate operand may be zero if there are more than
        // one incoming edge, because then it corresponds to an incoming edge that
        // has not been executed yet. But at least one incoming edge must
        // have an instance different from -1.
        if(!prevInstanceFound && !constantIncomingEdge)
        report_fatal_error("At least one incoming edge should have an entry \
                           in PointerToMemoryInstanceMap");
        if(!prevInstanceFound && constantIncomingEdge)
        constantOperand = true;
      }else{
        SIToFPInst *SIToFPI = dyn_cast<SIToFPInst> (I.getOperand(i));
        UIToFPInst *UIToFPI = dyn_cast<UIToFPInst> (I.getOperand(i));
        // =====================================================================
        //                 OPERAND IS A SIToFPI or UIToFPI
        //======================================================================
        if(SIToFPI || UIToFPI){
          InstructionValue SIToFPInstValue;
          if(SIToFPI)
          SIToFPInstValue = {SIToFPI,0};
          else if(UIToFPI){
            SIToFPInstValue = {UIToFPI,0};
          }
          else
          report_fatal_error("Instruction not SIToFPInst or  UIToFPInst\n");
          
          int SIToFPInstValueInstance =
          getInstructionValueInstance(SIToFPInstValue);
          if(SIToFPI)
          operandPTMI = {{SIToFPI, NULL, NULL, NULL, NULL, NULL},
            0, SIToFPInstValueInstance};
          else if(UIToFPI){
            operandPTMI = {{UIToFPI, NULL, NULL, NULL, NULL, NULL},
              0,SIToFPInstValueInstance};
          }else
          report_fatal_error("Instruction not SIToFPInst or  UIToFPInst\n");
          
          associatedPTMI = operandPTMI;
          if(WarmRun){
            insertAssociatedPointerToMemoryInstance(operandPTMI,associatedPTMI);
          }else{
            PointerToMemoryInstanceMapIterator it =
            PointerToMemoryInstanceMap.find(associatedPTMI);
            if(it == PointerToMemoryInstanceMap.end()){
              report_fatal_error("In analysis warm, there should be an \
                                 associated PTMI for the PTMI of the SIToFPInst\
                                 operand");
            }else
            associatedPTMI = it->second;
          }
          int RegisterStackDistance =
          registerStackReuseDistance(associatedPTMI, I, WarmRun, false);
          
          if(RegisterStackDistance < 0){
            insertRegisterStack(associatedPTMI, I, 0,WarmRun);
          }else{
            if(!WarmRun)
            updateRegisterReuseDistanceDistribution(RegisterStackDistance);
          }
          increaseInstructionValueInstance(SIToFPInstValue);
          // In this case, we have artificially inserted in the stack, so avoid
          // checking again. We do this way instead of just doing here the
          // definition of associatedPTMI,  because this operand was not
          // analyzed as an instruction, so it was not in the stack. And if we
          // execute the code below it will trigger a spill load because
          // every operand should have been executed before, which is not the case.
          constantOperand = true;
        }else{ // if not of SIToFPInst or UIToFPInst
          // ===================================================================
          //                 OPERAND IS a bitcast
          //====================================================================
          if(BitCastInst *BI = dyn_cast<BitCastInst> (I.getOperand(i))){
            Type * Ty = BI->getDestTy();
            if(Ty->getTypeID () == Type::FloatTyID){
              InstructionValue BIValue = {BI,0};;
              int BIValueInstance =  getInstructionValueInstance(BIValue);
              if(!WarmRun){
                if(BIValueInstance == 1)
                BIValueInstance = -1;
                else
                BIValueInstance = BIValueInstance-1;
              }
              operandPTMI =
              {{BI, NULL, NULL, NULL, NULL, NULL},0,BIValueInstance};
              associatedPTMI = operandPTMI;
              if(WarmRun){
                insertAssociatedPointerToMemoryInstance(operandPTMI,
                                                        associatedPTMI);
              }else{
                PointerToMemoryInstanceMapIterator it =
                PointerToMemoryInstanceMap.find(associatedPTMI);
                if(it == PointerToMemoryInstanceMap.end()){
                  report_fatal_error("In analysis warm, there should be an \
                                     associated PTMI for the PTMI of the Bicast\
                                     operand");
                }else{
                  associatedPTMI = it->second;
                }
              }
              
              int RegisterStackDistance =
              registerStackReuseDistance(associatedPTMI, I, WarmRun, false);
              if(RegisterStackDistance < 0){
                insertRegisterStack(associatedPTMI, I, 0,WarmRun);
              }else{
                if(!WarmRun)
                updateRegisterReuseDistanceDistribution(RegisterStackDistance);
              }
              
              increaseInstructionValueInstance(BIValue);
              constantOperand = true;
            }else
            report_fatal_error("Operand is a cast to a type that is not float");
          }else{
            // =================================================================
            //              OPERAND is the result of a select instruction
            //==================================================================
            if(SelectInst *SI = dyn_cast<SelectInst> (I.getOperand(i))){
              if(SI->getTrueValue()->getType()->getTypeID() == Type::DoubleTyID){
                Value* SIV = 	SI->getTrueValue();
                if(dyn_cast<Constant> (SI->getTrueValue()))
                  constantOperand = true;
                
                InstructionValue SIValue = {SIV,0};
                int SIValueInstance =  getInstructionValueInstance(SIValue);
                operandPTMI = {{SIV, NULL, NULL, NULL, NULL, NULL},0,SIValueInstance};
                associatedPTMI = operandPTMI;
                PointerToMemoryInstanceMapIterator it =
                PointerToMemoryInstanceMap.find(operandPTMI);
                if(it == PointerToMemoryInstanceMap.end())
                  insertAssociatedPointerToMemoryInstance(operandPTMI,
                                                          associatedPTMI);
                else
                  associatedPTMI = it->second;
              }
            }else{
              // Operand is not a bit cast
              // ===============================================================
              //                 OPERAND IS a call instruction
              //================================================================
              if(CallInst *CI = dyn_cast<CallInst> (I.getOperand(i))){
                Function * f = CI->getCalledFunction();
                operandRepetition = getLastRepetitionIntrinsic (f->getName());
              }else
                operandRepetition = 0;

              instructionValueOperand = {I.getOperand(i),operandRepetition};
              operandValueInstance =
              getInstructionValueInstance(instructionValueOperand);
              // We have to decrease instance because instance was increased
              // after the execution of the corresponding instruction => need to
              // search for the  previous one.
              if(operandValueInstance == 1 || operandValueInstance == -1)
                operandValueInstance = -1;
              else
                operandValueInstance = operandValueInstance-1;
              
              operandPTM = {I.getOperand(i),NULL, NULL, NULL, NULL, NULL};
              operandPTMI = {operandPTM,operandRepetition,operandValueInstance};
            }
          }
          
        }// END OF ELSE
        
        // The operand can be:
        // (1) Value loaded from memory, in which case it will be the
        // use of some pointer to memory in PointerToMemoryInstructionsMap, or
        // (2) the result of an arithmetic operation. In this case, it does not
        // appear in PointerToMemoryInstructionsMap.
        // (3) A constant that has been passed as an argument
        PointerToMemoryInstanceMapIterator it =
        PointerToMemoryInstanceMap.find(operandPTMI);
        if(it == PointerToMemoryInstanceMap.end()){
          report_fatal_error("Every operand should have an entry in \
                             PointerToMemoryInstanceMap??");
        }else
          associatedPTMI = PointerToMemoryInstanceMap[operandPTMI];
      }
    }
  }
  if(!constantOperand && !argumentOperand){
    if(associatedPTMIvector.size()>0){
      bool incomingEdgeInStack = false;
      int positionMaxDistance = -1;
      int maxDistance = -1;
      
      for(unsigned i = 0; i< associatedPTMIvector.size() ; i++){
        associatedPTMI = associatedPTMIvector.at(i);
        // Just take the first one that is on the stack or, if any, the first
        // one found. This approach has a problem: the one chosen in the warm
        // run may not be the same one as the one chosen in the analysis run,
        // and that may create problem with nUses, and spills of values that are
        // never used again (because the elements in the stack in the warm and
        // in the analysis run are different).
        int associatedPTMIRegisterDistance =
        checkRegisterStackReuseDistance(associatedPTMI);
        
        if(associatedPTMIRegisterDistance >= 0){
          incomingEdgeInStack = true;
          if(maxDistance==-1)
            maxDistance = associatedPTMIRegisterDistance;
          else
            maxDistance = max(maxDistance, associatedPTMIRegisterDistance);
          
          if(maxDistance == associatedPTMIRegisterDistance)
            positionMaxDistance = i;
        }
      }
      
      if(incomingEdgeInStack == true){
        associatedPTMI = associatedPTMIvector.at(positionMaxDistance);
        if(WarmRun){
          increaseNUses(associatedPTMI);
        }else{
          decreaseNUses(associatedPTMI);
        }
        if(!WarmRun){
          // TODO: FIx when this should be + NElementsVector
          InstructionsCountExtended[REGISTER_LOAD_CHANNEL]++;
          ScalarInstructionsCountExtended[REGISTER_LOAD_CHANNEL]++;
          //VectorInstructionsCountExtended[REGISTER_LOAD_CHANNEL]++;
          updateRegisterReuseDistanceDistribution(maxDistance);
        }
      }else{
        associatedPTMI = associatedPTMIvector.front();
        // Use the same instruction to constraint data dependencies.
        if(!WarmRun)
          NRegisterSpillsLoads++;
        bm_type::left_const_iterator left_iter =
        PointerToMemoryInstanceAddressBiMap.left.find(associatedPTMI);
        uint64_t SpillLoadAddress =left_iter->second;

        if(SpillLoadAddress == 0){
          report_fatal_error("Any value loaded that was a spill, should have \
                             an associated address");
        }
        analyzeInstruction(I, Instruction::Load,  SpillLoadAddress, Line,
                           true,VectorWidth,0, true, true, true);
        TotalInstructions++;
      }
    }else{
      // =======================================================================
      // 1.Check whether the operand is associated already to a pointer to memory.
      //========================================================================
      
      // =======================================================================
      // 2.Check if the operand is in the stack. If not in the stack, load.
      //========================================================================
      int RegisterStackDistance = registerStackReuseDistance(associatedPTMI,I,
                                                             WarmRun, false);
      if(RegisterStackDistance < 0){
        uint64_t SpillLoadAddress;
        // Use the same instruction to constraint data dependencies.
        if(!WarmRun)
        NRegisterSpillsLoads++;
        bm_type::left_const_iterator left_iter =
        PointerToMemoryInstanceAddressBiMap.left.find(associatedPTMI);
        if(left_iter == PointerToMemoryInstanceAddressBiMap.left.end()){
          report_fatal_error("There is not entry for associatedPTMI in \
                             PointerToMemoryInstanceAddressBiMap\n");
        }else
          SpillLoadAddress=left_iter->second;

        if(SpillLoadAddress == 0){
          report_fatal_error("Any value loaded that was a spill, should have an \
                             associated address");
        }
        analyzeInstruction(I, Instruction::Load,  SpillLoadAddress, Line,
                           true,VectorWidth,0, true, true, true);
        TotalInstructions++;
      }else{
        if(!WarmRun){
          InstructionsCountExtended[REGISTER_LOAD_CHANNEL]++;
          ScalarInstructionsCountExtended[REGISTER_LOAD_CHANNEL]++;
          //	VectorInstructionsCountExtended[REGISTER_LOAD_CHANNEL]++;
          updateRegisterReuseDistanceDistribution(RegisterStackDistance);
        }
      }
    }
  }
  return associatedPTMI;
}


int
DynamicAnalysis::checkRegisterStackReuseDistance(PointerToMemoryInstance address)
{
  int Distance = -1;
  
  if(RegisterFileSize == 0)
    return Distance;
  
#ifdef STACK_DEQUE
  std::deque<PointerToMemoryInstance>::iterator itPosition;
  unsigned PositionCounter = 0;
  if(!ReuseStack.empty()){
    for (std::deque<PointerToMemoryInstance>::iterator it = ReuseStack.end()-1;
         it>= ReuseStack.begin(); --it){
      if((*it).PTMindex == address.PTMindex && (*it).Rep == address.Rep &&
          (*it).IterationCount == address.IterationCount){
        Distance = PositionCounter;
        itPosition = it;
        break;
      }
      PositionCounter++;
    }
  }
#else
  if(!ReuseStack.empty())
    Distance = ReuseStack.findElement(address);
#endif
  if(Distance > (int)RegisterFileSize)
    report_fatal_error("Distance > RegisterFileSize");
  return Distance;
}


void
DynamicAnalysis::removeUnusedSpilledCacheLinesFromReuseTree(){
  
  PointerToMemoryInstanceNUsesMapIterator nUsesit;
  
  map <uint64_t, uint64_t> CacheLinesUses;
  for (bm_type::right_iterator right_iter =
      PointerToMemoryInstanceAddressBiMap.right.begin();
      right_iter != PointerToMemoryInstanceAddressBiMap.right.end();
       right_iter++){
    nUsesit = PointerToMemoryInstanceNUsesMap.find(right_iter->second);
    uint64_t CL = right_iter->first >> BitsPerCacheLine;
    if(CacheLinesUses.find(CL)==CacheLinesUses.end()){
      if(nUsesit->second >3)
        CacheLinesUses[CL] = nUsesit->second;
      else
        CacheLinesUses[CL] =0;
    }
    else
      if(nUsesit->second >3)
        CacheLinesUses[CL] += nUsesit->second;
  }
  
  for (map<uint64_t, uint64_t> ::iterator it = CacheLinesUses.begin();
      it != CacheLinesUses.end(); it++){
    if(it->second==0){
      CacheLineInfo Info  = getCacheLineInfo (it->first);
      ReuseTree=  delete_node(Info.LastAccess, ReuseTree);
    }
  }
}


int
DynamicAnalysis::registerStackReuseDistance(PointerToMemoryInstance address,
                                            Instruction & CurrentInst,
                                            bool WarmRun, bool isSpill)
{
  int Distance = -1;
  
  if(RegisterFileSize == 0)
  return Distance;
  
  if(!isSpill){
    if(WarmRun)
      increaseNUses(address);
    else
      decreaseNUses(address);
  }
  
#ifdef STACK_DEQUE
  std::deque<PointerToMemoryInstance>::iterator itPosition;
  unsigned PositionCounter = 0;
  if(!ReuseStack.empty()){
    for (std::deque<PointerToMemoryInstance>::iterator it = ReuseStack.end()-1;
         it>= ReuseStack.begin(); --it){
      if((*it).PTMindex == address.PTMindex && (*it).Rep == address.Rep &&
          (*it).IterationCount == address.IterationCount){
        Distance = PositionCounter;
        itPosition = it;
        break;
      }
      PositionCounter++;
    }
  }
  // If element was not in ReuseStack,
  // - If size of ReuseStack is equal to RegisterFile, pop front and push back
  //   the new element
  // - Else, simply push_back
  if(Distance >= 0){
    ReuseStack.erase(itPosition);
    ReuseStack.push_back(address);
  }
#else
  if(!ReuseStack.empty()){
    Distance = ReuseStack.findElement(address);
    if(Distance >= 0){
      ReuseStack.removeElement(address);
      ReuseStack.insertAtBack(address);
    }
  }
#endif
  return Distance;
}


void DynamicAnalysis::printRegisterStack()
{
#ifdef STACK_DEQUE
  unsigned counter = 1;
  for (std::deque<PointerToMemoryInstance>::iterator it = ReuseStack.begin();
       it< ReuseStack.end(); ++it){
    DEBUG( dbgs() <<counter << "\t");
    printPointerToMemoryInstance((*it));
    DEBUG(dbgs()<< "\n");
    counter++;
  }
  DEBUG(dbgs()<<"\n");
#else
  unsigned counter = 1;
  for(unsigned i = 0; i< ReuseStack.size(); i++){
    DEBUG( dbgs() <<counter << "\t");
    printPointerToMemoryInstance(ReuseStack.elementAt(i));
    DEBUG(dbgs()<< "\n");
    counter++;
  }
  DEBUG(dbgs()<<"\n");
#endif
}

void
DynamicAnalysis::removeRegisterStack(PointerToMemoryInstance address)
{
#ifdef STACK_DEQUE
  deque<PointerToMemoryInstance>::iterator it ;
  DEBUG(dbgs() <<  "Removing ");
  printPointerToMemoryInstance(address);
  DEBUG(dbgs() << " from the stack.\n");
  it = find(ReuseStack.begin(), ReuseStack.end(), address);
  if(it == ReuseStack.end())
    report_fatal_error("Trying to remove an element from the stack that is not\
                       in the stack");
  else
    ReuseStack.erase(it);
#else
  ReuseStack.removeElement(address);
#endif
  
}


void DynamicAnalysis::insertRegisterStack(PointerToMemoryInstance address,
                                          Instruction & CurrentInst,
                                          unsigned valueRep, bool WarmRun)
{
  bool isSpill = false;
  uint64_t MemAddress = 0;
  
  // Make sure that the value inserted is not a phi node.
  // Phi nodes are always inserted with the value of its incoming edge
  if(RegisterFileSize == 0)
    return;
  
  if(ReuseStack.size() == RegisterFileSize){
#ifdef STACK_DEQUE
    PointerToMemoryInstance SpilledPointerToMemory = ReuseStack.front();
    ReuseStack.pop_front();
    ReuseStack.push_back(address);
#else
    PointerToMemoryInstance SpilledPointerToMemory = ReuseStack.getFront();
    ReuseStack.removeFromFront();
    ReuseStack.insertAtBack(address);
#endif
    
    bool forceSpill = true;
    if(!WarmRun){
      PointerToMemoryInstanceNUsesMapIterator nUsesit =
      PointerToMemoryInstanceNUsesMap.find(address);
      if(nUsesit== PointerToMemoryInstanceNUsesMap.end())
        report_fatal_error("There should exist an entry in \
                           PointerToMemoryInstanceNUsesMap for a value being \
                           inserted into the stack");
      else{
        if(nUsesit->second == 0)
          forceSpill = false;
      }
    }
    
    if(forceSpill){
      // If this is an intermediate value generated in an arithmetic instruction,
      // then there is not associated address, and we have to assign one.
      if(!WarmRun){
        if(PointerToMemoryInstanceNUsesMap[SpilledPointerToMemory] == 0)
          report_fatal_error("A value not used should not be spilled");
      }
      bool SpilledValueWasMemOp = true;
      PointerToMemoryInstanceMapIterator it =
      PointerToMemoryInstanceMap.find(SpilledPointerToMemory);
      bm_type::left_iterator left_iter =
      PointerToMemoryInstanceAddressBiMap.left.find(SpilledPointerToMemory);
      
      if(it != PointerToMemoryInstanceMap.end() ){
        if(it->second == SpilledPointerToMemory)
          SpilledValueWasMemOp = false;
        else{
          PointerToMemoryInstanceNUsesMapIterator nUsesit =
          PointerToMemoryInstanceNUsesMap.find(SpilledPointerToMemory);
          if(nUsesit!= PointerToMemoryInstanceNUsesMap.end())
            SpilledValueWasMemOp = false;
        }
      }
      
      if(WarmRun){
        if(SpilledValueWasMemOp == false){
          // Check whether there is an associated address
          if(left_iter == PointerToMemoryInstanceAddressBiMap.left.end()){
            // Make sure that the memAddress does not exist
            // If the spill is triggered by an artificial mem op, then the
            // artificial address assigned to the spill and the address of the
            // artificial mem op would overlap, because
            // globalAddrForArtificialMemOps is only increased after the
            // execution of the artificial mem op.
            if(dyn_cast < CallInst >(&CurrentInst))
              GlobalAddrForArtificialMemOps = getNextArtificialAddress();
            
            MemAddress = GlobalAddrForArtificialMemOps;
            bm_type::right_iterator address_iter =
            PointerToMemoryInstanceAddressBiMap.right.find(MemAddress);
            if(address_iter != PointerToMemoryInstanceAddressBiMap.right.end())
              report_fatal_error("Trying to insert an entry in \
                                 PointerToMemoryInstanceAddressBiMap for an \
                                 address that exists already");
            
            bm_type::left_iterator PTMI_iter =
            PointerToMemoryInstanceAddressBiMap.left.find(SpilledPointerToMemory);
            
            if(PTMI_iter != PointerToMemoryInstanceAddressBiMap.left.end())
              report_fatal_error("Trying to insert an entry in \
                                 PointerToMemoryInstanceAddressBiMap for a PTMI \
                                 that exists already");
            PointerToMemoryInstanceAddressBiMap.
            insert(bm_type::value_type(SpilledPointerToMemory,MemAddress));
            GlobalAddrForArtificialMemOps = getNextArtificialAddress();
          }else
            MemAddress = left_iter->second;
        }else{
          // A spilled mem opt must have an associated address.
          if(left_iter == PointerToMemoryInstanceAddressBiMap.left.end())
            report_fatal_error("A spill of a mem op should have already an \
                               associated address");
          else
            MemAddress = left_iter->second;
        }
      }else{
        // In analysis run, everything should have an associated address.
        // But still need to make sure that there is an associated address
        if(left_iter == PointerToMemoryInstanceAddressBiMap.left.end())
          report_fatal_error("In analysis run, all spills should have already \
                             an associated address, regardless they are mem op \
                             or not");
        else
          MemAddress = left_iter->second;
      }
      
      if(SpilledValueWasMemOp){
      }else{
        if(!WarmRun)
          NRegisterSpillsStores++;
        SpilledCacheLine.insert(MemAddress);
        isSpill = true;
        // TODO: vector width is not always 1
        analyzeInstruction(CurrentInst, Instruction::Store,  MemAddress,
                           SourceCodeLine, true, VectorWidth, valueRep, true,
                           true,  isSpill);
        TotalInstructions++;
      }
    }
  }else{
#ifdef STACK_DEQUE
    ReuseStack.push_back(address);
#else
    ReuseStack.insertAtBack(address);
#endif
  }
}


int64_t
DynamicAnalysis::getInstructionValueInstance(InstructionValue instValue)
{
  int64_t InstructionInstance = -1;
  map < InstructionValue, int64_t >::iterator InstanceMapIt;
  //&I
  InstanceMapIt = InstructionValueMap.find(instValue);
  if(InstanceMapIt != InstructionValueMap.end ())
    InstructionInstance = InstanceMapIt->second;
  return InstructionInstance;
}


void
DynamicAnalysis::insertAssociatedPointerToMemoryInstance(PointerToMemoryInstance instructionPTMI,
                                                         PointerToMemoryInstance associatedPTMI)
{
  PointerToMemoryInstanceMapIterator it =
  PointerToMemoryInstanceMap.find(instructionPTMI);
  if(it == PointerToMemoryInstanceMap.end())
    PointerToMemoryInstanceMap[instructionPTMI] = associatedPTMI;
  else{
    if(!(it->second== associatedPTMI)){
      if(!(instructionPTMI == associatedPTMI)){
        // This special case captures the situation when there is an if/else
        // statement such as for example one of the two loads that are the
        // users of a getelementptr instruction is not executed. In this case,
        // we would get the error that an associatedPTMI already exists for
        // the iteration count -1 of the load, although it has never been
        // executed. In this case we check that iterationCount of the
        // two PTMI are consecutive, and update the entry of the instructionPTMI
        // with the new associatedPTMI.
        // We could leave the initial value of the iteration Count of the
        // associatedPTMI, but by updating we can keep track better.
        PointerToMemoryInstanceMap[instructionPTMI] = associatedPTMI;
      }
    }
  }
}


void
DynamicAnalysis::updateAssociatedPointerToMemoryInstance(PointerToMemoryInstance instructionPTMI,
                                                         PointerToMemoryInstance associatedPTMI)
{
  PointerToMemoryInstanceMapIterator it =
  PointerToMemoryInstanceMap.find(instructionPTMI);
  if(it == PointerToMemoryInstanceMap.end())
    report_fatal_error("Cannot update associatedPTMI of an instruction PTMI \
                     that does not have an entry");
  else
    PointerToMemoryInstanceMap[instructionPTMI] = associatedPTMI;

}


// Return true is one of the uses is an instruction to be analyezd
bool
DynamicAnalysis::insertUsesOfPointerToMemory(Value * v,
                                             PointerToMemoryInstance associatedPTMI,
                                             bool recursiveCall)
{
  // ===========================================================================
  // 1.Iterate over the uses of the getelementptr or alloca instruction,
  //   and when a use is found, create an entry in PointerToMemoryMap.
  //   The entry is of the form: PointerToMemoryInstance - PointerToMemoryInstance
  //============================================================================
  InstructionValue useInstructionValue;
  PointerToMemoryInstance instructionPTMI;
  PointerToMemory instructionPTM;
  bool analyzeUses = true;
  bool insertUse = false;
  bool insertUseBitCast = false;
  string instructionType;
  
  if(dyn_cast<GetElementPtrInst> (v))
    instructionType = "getelementprt";
  else if(dyn_cast<AllocaInst> (v))
    instructionType = "alloca";
  else if(BitCastInst *BI = dyn_cast<BitCastInst> (v)){
    instructionType = "bitcast";
    Value * op = BI->getOperand(0);
    // If the operator is a getelementptr/alloca, we have already dealt with it
    if(!recursiveCall){
      if(dyn_cast<GetElementPtrInst>(op) || dyn_cast<AllocaInst>(op))
        analyzeUses = false;
    }
  }else
    report_fatal_error("Pointer to memory instruction not recognized");
  
  if(analyzeUses){
    for (Value::use_iterator ib = v->use_begin(), ie = v->use_end();
         ib != ie; ++ib) {
      Use &U = *ib;
      auto *i = cast<Instruction>(U.getUser());
      if  (dyn_cast<BitCastInst>(i)) {
        // If insertUse was true, we don't want it overwritten with false.
        insertUseBitCast = insertUsesOfPointerToMemory(i, associatedPTMI, true);
        if(insertUse == false)
        insertUse = insertUseBitCast;
      }else if(LoadInst *LI = dyn_cast<LoadInst>(i)){
        if(getInstructionType (*LI) >=0){
          insertUse = true;
          useInstructionValue = {LI, 0};
          instructionPTM = {LI, NULL, NULL, NULL, NULL, NULL};
          instructionPTMI = {instructionPTM, 0,
            getInstructionValueInstance(useInstructionValue)};
          insertAssociatedPointerToMemoryInstance(instructionPTMI, associatedPTMI);
        }
      }else if(StoreInst *SI = dyn_cast<StoreInst>(i)) {
        if(getInstructionType (*SI) >=0){
          insertUse = true;
          useInstructionValue = {SI, 0};
          instructionPTM = {SI, NULL, NULL, NULL, NULL, NULL};
          instructionPTMI = {instructionPTM, 0,
            getInstructionValueInstance(useInstructionValue)};
          insertAssociatedPointerToMemoryInstance(instructionPTMI, associatedPTMI);
        }
      }else if(CallInst * CI = dyn_cast < CallInst > (i)){
        if(CI->getCalledFunction()->getName().find("llvm.x86") != string::npos){
          insertUse = true;
          useInstructionValue = {CI, 0};
          instructionPTM = {CI, NULL, NULL, NULL, NULL, NULL};
          instructionPTMI = {instructionPTM, 0,
            getInstructionValueInstance(useInstructionValue)};
          insertAssociatedPointerToMemoryInstance(instructionPTMI, associatedPTMI);
        }else if(CI->getCalledFunction()->getName().find("__ct") == string::npos){
          Function *  F = CI->getCalledFunction();
          unsigned int k = 0;
          for (Function::arg_iterator AI = F->arg_begin (), E = F->arg_end ();
               AI != E; ++AI, k++) {
            if(CI->getArgOperand(k) == v){
              // Iterate through the uses of the argument
              for (Value::use_iterator vi = (*AI).use_begin (),
                   vie = (*AI).use_end (); vi != vie; ++vi) {
                insertUse = true;
                useInstructionValue = {*vi, 0};
                instructionPTM = {*vi, NULL, NULL, NULL, NULL, NULL};
                instructionPTMI = {instructionPTM, 0,
                  getInstructionValueInstance(useInstructionValue)};
                insertAssociatedPointerToMemoryInstance(instructionPTMI,
                                                        associatedPTMI);
              }
            }
          }
        }
      }
    }
  }
  return insertUse;
}


void DynamicAnalysis::resetInstructionValueMap()
{
  map < InstructionValue, int64_t >::iterator InstanceMapIt;
  for(InstanceMapIt = InstructionValueMap.begin();
      InstanceMapIt != InstructionValueMap.end(); InstanceMapIt++ )
    (*InstanceMapIt).second = -1;
}


void
DynamicAnalysis::increaseInstructionValueInstance(InstructionValue v)
{
  map < InstructionValue, int64_t >::iterator InstanceMapIt;
  InstanceMapIt = InstructionValueMap.find(v);
  if(InstanceMapIt != InstructionValueMap.end()) {
    if(InstanceMapIt->second == -1)
      InstanceMapIt->second = 1;
    else
      InstanceMapIt->second++;
  }else{
    // First use
    InstructionValueMap.insert(std::pair<InstructionValue, int64_t>(v, 1));
  }
}


uint64_t
DynamicAnalysis::adjustMemoryAddress(uint64_t addr, uint64_t addrFound,
                                     PointerToMemoryInstance duplicatedPMTI,
                                     bool forceAnalyze)
{
  // addr and addrFound are different
  uint64_t finalMemAddr;
  if(forceAnalyze == true){
    finalMemAddr = addrFound;
  }else{
    if(addr < GlobalAddrForArtificialMemOps){
      if(addrFound < GlobalAddrForArtificialMemOps){
        // There is a case in which this still may happen. E.g.:
        // %arrayidx143 = getelementptr inbounds %struct.Arrays* @as1, i64 0,
        // i32 39, i64 %j.0, i64 %k.0225, !dbg !431 (0x2b62038)
        // %arrayidx84 = getelementptr inbounds %struct.Arrays* @as1, i64 0,
        // i32 39, i64 %j.0, i64 %k.0225, !dbg !416 (0x2b5e4a8)
        // They correspond to the same PTMI, but since there are if/else
        // statements, the iteration count of the associated PTMI is not
        // increase, and 2 real addresses appear for the same PTMI.
        // In this case, check whether the instructions PRTMI corresponding to
        // the problematic associatedPTMI are different. In this case:
        // Increase the instance of the associatedPTMI and associate the
        // correct address.
        vector<PointerToMemoryInstance> instructionsPTMI;
        for(PointerToMemoryInstanceMapIterator it  =
            PointerToMemoryInstanceMap.begin();
            it != PointerToMemoryInstanceMap.end(); it++){
          if(it->second == duplicatedPMTI)
            instructionsPTMI.push_back(it->second);
        }
        bool instructionsPTMIequal = true;
        if(instructionsPTMI.size() <=1){
          report_fatal_error("There should be at least two corresponding \
                             instructionPTMI for an associatedPTMI for which \
                             there are two different addresses");
        }else{
          for(unsigned i = 0; i< instructionsPTMI.size()-1; i++){
            for(unsigned j = i+1; j < instructionsPTMI.size(); j++){
              if(!(instructionsPTMI.at(i) == instructionsPTMI.at(j))){
                instructionsPTMIequal = false;
                // Increase the instance of the associated PTMI
                PointerToMemoryInstance newDuplicatedPMTI = duplicatedPMTI;
                if(newDuplicatedPMTI.IterationCount == -1)
                  newDuplicatedPMTI.IterationCount = 1;
                else
                  newDuplicatedPMTI.IterationCount++;
                PointerToMemoryInstanceMap[instructionsPTMI.at(j)]=
                newDuplicatedPMTI;
                bm_type::right_iterator address_iter =
                PointerToMemoryInstanceAddressBiMap.right.find(addrFound);
                if(address_iter->second == instructionsPTMI.at(i)){
                  // TODO: check if element inserted exists or not with iterators
                  PointerToMemoryInstanceAddressBiMap.
                  insert(bm_type::value_type(instructionsPTMI.at(j),addr));
                }else{
                  if(address_iter->second == instructionsPTMI.at(j)){
                    PointerToMemoryInstanceAddressBiMap.
                    insert(bm_type::value_type(instructionsPTMI.at(i),addr));
                  }else{
                    report_fatal_error("CHECK, there might be more than 2 \
                                       instructionsPTMI with the same associated PTMI");
                  }
                }
                break;
              }
            }
            if(instructionsPTMIequal == false)
              break;
          }
        }
        if(instructionsPTMIequal == false){
          report_fatal_error("Two different addresses for the same pointer to \
                             memory");
        }
      }else{
        finalMemAddr = addr;
      }
      
    }else{
      report_fatal_error("If do not force analyze, address should not be smaller\
                         than globalAddrForArtificialMemOps");
    }
  }
  return finalMemAddr;
}


void DynamicAnalysis::check()
{
  bool PTMIfound = false;
  for(PointerToMemoryInstanceNUsesMapIterator it =
      PointerToMemoryInstanceNUsesMap.begin();
      it != PointerToMemoryInstanceNUsesMap.end(); it++){
    PTMIfound = false;
    for(PointerToMemoryInstanceMapIterator it2 =
        PointerToMemoryInstanceMap.begin();
        it2 != PointerToMemoryInstanceMap.end(); it2++){
      if(it->first == it2->second){
        PTMIfound = true;
        break;
      }
    }
    if(PTMIfound == false){
      report_fatal_error("ERROR");
    }
  }
}

uint64_t DynamicAnalysis::getNextArtificialAddress()
{
  return GlobalAddrForArtificialMemOps - 32;
}

PointerToMemoryInstance
DynamicAnalysis::managePointerToMemory(PointerToMemoryInstance instructionPTMI,
                                       Instruction & I, unsigned valueRep,
                                       int64_t valueInstance, uint64_t & addr,
                                       unsigned OpCode, bool WarmRun,
                                       bool forceAnalyze)
{
  // ===========================================================================
  // Check whether the instruction has an associated PointerToMemoryInstanceInfo,
  // that is, it was the user of a getlementptr/alloca/bitcast
  //============================================================================
  PointerToMemoryInstanceMapIterator it  =
  PointerToMemoryInstanceMap.find(instructionPTMI);
  PointerToMemoryInstance  associatedPTMI;
  // ===========================================================================
  //  WARM RUN - Need to track pointers to memory and addresses
  //============================================================================
  if(WarmRun){
    if(it == PointerToMemoryInstanceMap.end()){
      // =======================================================================
      // 1.1. The load/store accesses directly the memory location
      //      Although there is no associated pointer to memory, every
      //      load/store inst has an address check whether there is an entry in
      //      AddressMap for the given address
      //========================================================================
      bm_type::right_iterator address_iter =
      PointerToMemoryInstanceAddressBiMap.right.find(addr);
      if(address_iter != PointerToMemoryInstanceAddressBiMap.right.end()){
        // =====================================================================
        //  This address already appeared and have an associated pointer to
        //  memory. Associate that pointer to memory to the instructionPTMI
        //======================================================================
        // associatedPTMI already has an entry in addressMap and NUsesMap
        associatedPTMI = address_iter->second;
        insertAssociatedPointerToMemoryInstance(instructionPTMI, associatedPTMI);
      }else{
        bm_type::left_iterator P_iter =
        PointerToMemoryInstanceAddressBiMap.left.find(instructionPTMI);
        if(P_iter != PointerToMemoryInstanceAddressBiMap.left.end()){
          report_fatal_error("Case not considered\n");
        }
        // =====================================================================
        // Insert Pointer to memory into global vector of pointers to memory
        //======================================================================
        PointerToMemory associatedPTM;
        int64_t associadtedPTMinstance;
        if(dyn_cast < CallInst > (&I)) {
          associatedPTM = {I.getOperand(0), NULL, NULL, NULL, NULL, NULL};
          associadtedPTMinstance =
          getInstructionValueInstance({I.getOperand(0),valueRep});
          // The instance of the associatedPTMI should be 1: that means that
          // the pointer was used once
          if(associadtedPTMinstance > 1)
            report_fatal_error("The associated PTM does not have a previous \
                               instance, or has an instace > 1");

          if(valueInstance == 0)
            report_fatal_error("Load/Store instruction not found as the use of \
                               any other instruction should not happen for the \
                               first instance of an instruction");
          
          PointerToMemoryInstanceMapIterator prevIt;
          PointerToMemoryInstance prevInstructionPTMI;
          if(associadtedPTMinstance == -1){
            // In this case, I.getOperand(0) is not found as an instruction
            // value - This happens when, for example:
            //  %add.ptr1741 = getelementptr inbounds double* %S0, i64 208,
            //  %244 = bitcast double* %add.ptr1741 to i8* (0x7faab60608f8)
            // This is the second instance of the maskload
            // %530 = tail call <4 x double> @llvm.x86.avx.maskload.pd.256(...
            
            // In this case, %224 does not appear as an instructionValue, but
            // the instructionValue associated to  %add.ptr1741
            // Therefore, in this case, search for the associated PTMI of a
            // previous instance
            for (int instance = valueInstance -1; instance >= 0; instance--){
              if(instance == 0)
                instance = -1;
              prevInstructionPTMI = {instructionPTMI.PTM,
                instructionPTMI.Rep, instance};
              prevIt  =  PointerToMemoryInstanceMap.find(prevInstructionPTMI);
              if(prevIt != PointerToMemoryInstanceMap.end()){
                associatedPTMI =prevIt->second;
                break;
              }else
                report_fatal_error("The previous instance should have an \
                                   associated PTMI");
            }
          }else if(associadtedPTMinstance == 1){
            associatedPTMI = {associatedPTM, valueRep, -1};
          }
        }else{
          unsigned operandIndex = 0;
          if(OpCode == Instruction::Load){
            operandIndex = 0;
          }else if(dyn_cast < StoreInst > (&I)) {
            operandIndex = 1;
          }else{
            report_fatal_error("Real load/Store not found as the use of any \
                               other instruction, and address not found");
            
          }
          if(ConstantExpr * CE = dyn_cast<ConstantExpr>(I.getOperand(operandIndex))){
            unsigned NOperands = CE->getNumOperands();
            if(NOperands == 1)
              associatedPTM = {CE->getOperand(0), NULL, NULL, NULL, NULL, NULL};
            else if(NOperands == 2)
              associatedPTM = {CE->getOperand(0), CE->getOperand(1),
                NULL, NULL, NULL, NULL};
            else if(NOperands == 3)
              associatedPTM = {CE->getOperand(0), CE->getOperand(1),
                CE->getOperand(2), NULL, NULL, NULL};
            else if(NOperands == 4)
              associatedPTM = {CE->getOperand(0), CE->getOperand(1),
                CE->getOperand(2), CE->getOperand(3), NULL, NULL};
            else if(NOperands == 5)
              associatedPTM = {CE->getOperand(0), CE->getOperand(1),
                CE->getOperand(2), CE->getOperand(3), CE->getOperand(4), NULL};
            else if(NOperands == 6)
              associatedPTM = {CE->getOperand(0), CE->getOperand(1),
                CE->getOperand(2), CE->getOperand(3), CE->getOperand(4), CE->getOperand(5)};
            associatedPTMI = {associatedPTM, 0, -1};
          }else{
            if(dyn_cast<Argument> (I.getOperand(operandIndex))){
              Function * F = (&I)->getParent ()->getParent();
              bool loadArgument = false;
              for (Function::arg_iterator AI = F->arg_begin (),
                   E = F->arg_end (); AI != E; ++AI) {
                Value *A = &*AI;
                if(A == I.getOperand(operandIndex)){
                  loadArgument = true;
                  associatedPTM = {I.getOperand(operandIndex), NULL, NULL,
                    NULL, NULL, NULL};
                  associatedPTMI = {associatedPTM, 0, -1};
                  break;
                }
              }
              if(loadArgument == false){
                report_fatal_error("Real load/Store not found as the use of any \
                                   other instruction, and address not found");
              }
            }else{
              report_fatal_error("Real load/Store not found as the use of any \
                                 other instruction, and address not found");
            }
          }
        }
        // In this case, the instance of the pointer to memory is the instance
        // of the instruction
        insertAssociatedPointerToMemoryInstance(instructionPTMI, associatedPTMI);
        // Insert address
        bm_type::left_iterator PTMI_iter =
        PointerToMemoryInstanceAddressBiMap.left.find(associatedPTMI);
        if(PTMI_iter != PointerToMemoryInstanceAddressBiMap.left.end()){
        }else{
          bm_type::right_iterator address_iter =
          PointerToMemoryInstanceAddressBiMap.right.find(addr);
          if(address_iter != PointerToMemoryInstanceAddressBiMap.right.end()){
            report_fatal_error("Trying to insert an entry in \
                               PointerToMemoryInstanceAddressBiMap for an \
                               address that exists already");
          }
          PointerToMemoryInstanceAddressBiMap.
          insert(bm_type::value_type(associatedPTMI,addr));
        }
      }
    }else{
      associatedPTMI = (*it).second;
      // =======================================================================
      // 1.2. The load/store address has an associated pointer to memory.
      //========================================================================
      // Check whether the associatedPTMI already has an address
      bm_type::left_iterator associatedPTMIAddress_iter =
      PointerToMemoryInstanceAddressBiMap.left.find(associatedPTMI);
      
      if(associatedPTMIAddress_iter == PointerToMemoryInstanceAddressBiMap.left.end()){
        // ======================================================================
        // The associated PTMI does not have an associated address.
        // In principle, simply set the address to the address of the current
        // instruction.
        // However, we must first check that the address of the current
        // instruction does not have already  different associated pointer.
        // We need to check this because of a special case: 2 DIFFFERENT
        // POINTERS TO MEMORY MAY HAVE THE SAME ADDRESS. For example:
        // It is possible that we find two different pointers to memory for the
        // same address. For example, this set of instructions:
        // %add.ptr408 = getelementptr inbounds double* %L, i64 %128
        // %__v.i28542 = bitcast double* %add.ptr408 to <4 x double>*
        // %129 = load <4 x double>* %__v.i28542
        // are associated to PTM 0x3a8d1b0 0x3b73800 0x0 0x0
        // And these:
        // %add.ptr1977 = getelementptr inbounds double* %L, i64 204
        // %__v.i28238 = bitcast double* %add.ptr1977 to <4 x double>*
        // %615 = load <4 x double>* %__v.i28238, align 1
        // are associated to PTM 0x3a8d1b0 0x3b73800 0x0 0x0
        // In reality they correspond to:
        // t1_3 = _mm256_loadu_pd(L + 51*k2);
        // and
        //  t8_3 = _mm256_loadu_pd(L + 204);
        // which in fact access the same memory location.
        // In this case, stick to the first memory location
        // =====================================================================
        bm_type::right_iterator addressAssociatedPTMI_iter =
        PointerToMemoryInstanceAddressBiMap.right.find(addr);
        if(addressAssociatedPTMI_iter ==
           PointerToMemoryInstanceAddressBiMap.right.end()){
          // The address does not have another associated PTMI => create an
          // entry for this address and the associatedPTMI
          bm_type::right_iterator address_iter =
          PointerToMemoryInstanceAddressBiMap.right.find(addr);
          if(address_iter != PointerToMemoryInstanceAddressBiMap.right.end()){
            report_fatal_error("Trying to insert an entry in \
                               PointerToMemoryInstanceAddressBiMap for an \
                               address that exists already");
          }
          bm_type::left_iterator PTMI_iter =
          PointerToMemoryInstanceAddressBiMap.left.find(associatedPTMI);
          if(PTMI_iter != PointerToMemoryInstanceAddressBiMap.left.end()){
            report_fatal_error("Trying to insert an entry in \
                               PointerToMemoryInstanceAddressBiMap for a PTMI \
                               that exists already");
          }
          PointerToMemoryInstanceAddressBiMap.
          insert(bm_type::value_type(associatedPTMI,addr) );
        }else{
          // Keep the first PTMI
          PointerToMemoryInstance existingPTMI = addressAssociatedPTMI_iter->second;
          updateAssociatedPointerToMemoryInstance(instructionPTMI, existingPTMI);
          associatedPTMI = existingPTMI;
        }
      }else{
        // If the associatedPTMI has an associated address, compare with the
        // address of the current instruction.
        uint64_t addressFound = associatedPTMIAddress_iter->second;
        uint64_t finalAddress = 0;
        if(addr!=addressFound){
          finalAddress = adjustMemoryAddress(addr, addressFound,
                                             associatedPTMIAddress_iter->first,
                                             forceAnalyze);
          //finalAddress is either addr or addressFound
          if(finalAddress != addr){
            addr = finalAddress;
          }else{
            PointerToMemoryInstanceAddressBiMap.left.
            replace_data(associatedPTMIAddress_iter,addr);
          }
        }// Else, if addresses are the same, so nothing.
      }
    }
  }else{
    // =========================================================================
    //  ANALYSIS RUN
    //==========================================================================
    if(it == PointerToMemoryInstanceMap.end()){
      report_fatal_error("After the warm run, all pointers to memory \
                         should have been tracked.");
    }
  }
  return associatedPTMI;
}
#endif
