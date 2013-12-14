/* threadpass.cpp */

#include <string>
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

using namespace llvm;

namespace {
    /* Pass to create threads */
    struct createThread: public ModulePass {
        static char ID;
        createThread() : ModulePass(ID) {}
    
        virtual bool runOnModule(Module &M) {
            Function* funToRun;
            for (Module::iterator it = M.begin(), ite = M.end(); it != ite; ++it) {
                if (it->getName() == "PrintHello")
                    funToRun = &(*it);
            }
            errs() <<"FunToRun: " << funToRun->getName() << '\n';

            for (Module::iterator it = M.begin(), ite = M.end(); it != ite; ++it) {
                errs() << "Find function: " << it->getName() << '\n';
                Function &F = *it;
                if (F.getName() == "main") {

                    IntegerType* int32Ty = Type::getInt32Ty(M.getContext());
                    IntegerType* int64Ty = Type::getInt64Ty(M.getContext());
                    Type* voidTy = Type::getVoidTy(M.getContext());
                    Type* pthreadTy;
                    if (M.getPointerSize() == Module::Pointer64) {
                        pthreadTy = int64Ty;
                    } else {
                        pthreadTy = int32Ty;
                    }
		
                    // add produce function
                    Type* produce_args[] = { int64Ty, int32Ty };
                    FunctionType* produce_ft = FunctionType::get(voidTy, ArrayRef<Type*>(produce_args, 2), false);
                    Function* produce = Function::Create(produce_ft, Function::ExternalLinkage, "produce", &M);
	  
                    // add consume function
                    Type* consume_args[] = { int32Ty };
                    FunctionType* consume_ft = FunctionType::get(int64Ty, ArrayRef<Type*>(consume_args, 1), false);
                    Function* consume = Function::Create(consume_ft, Function::ExternalLinkage, "consume", &M);

                    // add thread_init function
                    FunctionType* init_ft = FunctionType::get(voidTy, false);
                    Function* threads_init = Function::Create(init_ft, Function::ExternalLinkage, "threads_init", &M);

                    CallInst::Create(threads_init, "", F.getEntryBlock().getTerminator());
	  
                    
                    BasicBlock* endBB;
                    for (Function::iterator BB = F.begin(), BBe = F.end(); BB != BBe; ++BB) {
                        if (BB->getName() == "for.end") {
                            endBB = BB;
                        }
                    }

                    std::vector<Value *> con_args;
                    ConstantInt* val = ConstantInt::get(int32Ty, (uint32_t) 5);
                    con_args.push_back(val);
                    CallInst::Create(consume, ArrayRef<Value*>(con_args), "", endBB->getFirstNonPHI());
                    
                    // stop worker threads
                    std::vector<Value *> pro_args;
                    val = ConstantInt::get(int64Ty, (uint64_t) 0);
                    pro_args.push_back(val);
                    val = ConstantInt::get(int32Ty, (uint32_t) 0);
                    pro_args.push_back(val);
                    CallInst::Create(produce, ArrayRef<Value*>(pro_args), "", endBB->getFirstNonPHI());

                }

            }


            return false;
        }

        void getAnalysisUsage(AnalysisUsage& AU) const {
            AU.addRequired<DominatorTree>();
            AU.addPreserved<DominatorTree>();
            AU.addRequired<LoopInfo>();
        }

    };
        
}

char createThread::ID = 0;
static RegisterPass<createThread> Y("createThread", "Thread creating", false, false);
