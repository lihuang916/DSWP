/**
 *    Decoupled Software Pipelining -
 *    EECS 583 Project 
 *    Dependencies.h
 *    Purpose: Header file for Dependencies.cpp. This finds all dependencies in the code using LLVM

 *    @author Li, Prashipa, Shravya
 *    @version 1.0 
 */

#ifndef Dependencies_H
#define Dependencies_H



#include <string>
#include <utility>
#include <map>
#include <iostream>
#include <set>
#include <list>
#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CFG.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Support/InstIterator.h"


namespace llvm{
    class LoopPass;
    class Dependencies : public LoopPass{
    public:
        //Type indicates the type of dependency between the instructions 
        enum Type {
            REG, MEMFLOW, MEMANTI, MEMOUT, CONTROL
        };
        //This structure stores the dependency attributes, like the instructions involved in the dependency, the type of the dependency.
        struct DepEdge{
            Instruction* i1;
            Instruction* i2;
            Type t;
        DepEdge(Instruction* i1, Instruction* i2, Type t) : i1(i1), i2(i2), t(t){};
        };

        std::map<Instruction*, std::vector<DepEdge>*> allDeps;//A map from the instructions to all the edges from that instruction. If v depends on u, there is a edge u->v. So the vector for u has edge u->v in it.
        std::set<std::pair<Instruction*,Instruction*> > DI; //map to store dependencies
        std::list<Instruction*> L;//List of all instructions
        std::list<BranchInst*> backEdgeParents;//The parents of backedges
/**
 * Prints all the dependencies
    
 * @param DI Dependent instructions
 */

        virtual void printSet(std::set<std::pair<Instruction*,Instruction*> > DI);
/**
 * Prints all the dependencies in the form given by allDeps
 */

        virtual void printDeps();

        static char ID;
    Dependencies() : LoopPass(ID) {}
/**
 *  Implements getAnalysisUsafe
    
 * @param AU
 */
        virtual void getAnalysisUsage(AnalysisUsage &AU) const;
/**
 *  Runs on loop
    
 * @param L 
 * @param LPM
 * @return returns bool
 */

        virtual bool runOnLoop(Loop *L, LPPassManager &LPM);
  
    }; 
}

	
#endif
