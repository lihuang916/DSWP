/**
*    Decoupled Software Pipelining -
*    EECS 583 Project 
*    getSCC.h
*    Purpose: Header file for getSCC.cpp. This calculates all the SCCs in the code using LLVM

*    @author Li, Prashipa, Shravya
*    @version 1.0 
*/


#ifndef PARTITION_H
#define PARTITION_H

#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Transforms/Utils/Cloning.h"

namespace llvm {

    class LoopPass;

    class partition : public LoopPass{

    public:
        static char ID;
        
        partition() : LoopPass(ID) {}

        bool doInitialization(Loop *L, LPPassManager &LPM);
      
        bool runOnLoop(Loop* L, LPPassManager &LPM);

        void getAnalysisUsage(AnalysisUsage &AU) const;
        
        bool doFinalization();

    private:
        Module* module;
        Function* function;
        LLVMContext* context;
    };
    
}



#endif
