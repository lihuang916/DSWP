/**
*    Decoupled Software Pipelining -
*    EECS 583 Project 
*    getSCC.h
*    Purpose: Header file for getSCC.cpp. This calculates all the SCCs in the code using LLVM

*    @author Li, Prashipa, Shravya
*    @version 1.0 
*/

#ifndef getSCC_H
#define getSCC_H



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


namespace llvm{

  class LoopPass;
  
  class getSCC: public LoopPass {

  public:
    static char ID;
    getSCC() : LoopPass(ID), cnt(0) {}   
    Dependencies *dep;

    std::set<std::pair<Instruction*,Instruction*> > tempdep; //map to store dependencies
    std::list<llvm::StoreInst *> stores;//Maintains a list of stores
    std::list<llvm::LoadInst *> loads;//Maintains a list of loads
    std::list<llvm::Instruction*> nodes;//Maintains a list of instructions
    std::set<std::pair<Instruction*,Instruction*> > edges;//Maintains a list of all edges 
    std::map<Instruction*,int> nodes_to_int;// Maps instructions to their respective IDs
    std::map<int,Instruction*> int_to_nodes;//Maps IDs to the instructions
    std::list<int> nodes1;//Maintains the list of instruction IDs
    std::set<std::pair<int,int> > edges_int;//Maintains list of edges with instruction ids
    std::map<int,int> visited; // to store the list of visited nodes
    std::map<int,int> dfsnum; // to store the number of vertices before the node n
    std::map<int,int> low; // to store the smallest dfsnum reachable by a backedge or crossedge
    std::list<int> L;//list for maintaning the current SCC
    std::map<int,std::vector<Instruction*>* > SCCmap;//The list of SCC id to the list of instructions in a SCC
    std::map<Instruction*,std::vector<int>* > inst_to_SCC;//This list states which SCC the instruction belongs to
    int N;
    int cnt;// a counter to keep track of number of sccs

/**
   *  Returns the minimum of two integers

   * @param a
   * @param b
   * @return The minimum of two integers
*/
    virtual int min(int a,int b);

/**
   *  Visits the nodes recursively to see if any component i sstrongly connected
    
   * @param p This is the node where the traversal to start from
*/  
    virtual  void visit(int p);

/**
   *  Implements Tarjan DFS
*/  
    virtual void DFS();

/**
   *  Runs on loop
    
   * @param L 
   * @param LPM
   * @return returns bool
*/  
    virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

/**
   *  Implements getAnalysisUsafe
    
   * @param AU
*/  
    virtual void getAnalysisUsage(AnalysisUsage& AU) const;

/**
   * Prints all the dependencies
    
   * @param tempdeps
*/  
    virtual void printSet(std::set<std::pair<Instruction*,Instruction*> > tempdep);

/**
   * Prints all the instructions. Here the instructions are referred to as nodes
*/  
    virtual void printNodes();

};


}

#endif
