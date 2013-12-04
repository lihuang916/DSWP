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



	  Type* int32Ty = Type::getInt32Ty(M.getContext());
	  Type* int64Ty = Type::getInt64Ty(M.getContext());
	  PointerType* voidPtrTy = Type::getInt8PtrTy(M.getContext());
	  Type* voidTy = Type::getVoidTy(M.getContext());
	  Type* pthreadTy;
    	  if (M.getPointerSize() == Module::Pointer64) {
	    pthreadTy = int64Ty;
	  } else {
	    pthreadTy = int32Ty;
	  }
		
	  // add produce function
	  Type* produce_args[] = { int64Ty, int32Ty};
	  FunctionType* produce_ft = FunctionType::get(voidTy, ArrayRef<Type*>(produce_args, 2), false);
	  Function* produce = Function::Create(produce_ft, Function::ExternalLinkage, "produce", &M);
	  
	  // add consume function
	  Type* consume_args[] = { int32Ty };
	  FunctionType* consume_ft = FunctionType::get(int64Ty, ArrayRef<Type*>(consume_args, 1), false);
	  Function* consume = Function::Create(consume_ft, Function::ExternalLinkage, "consume", &M);

	  // add thread_init function
	  FunctionType* init_ft = FunctionType::get(voidTy, false);
	  Function* threads_init = Function::Create(init_ft, Function::ExternalLinkage, "threads_init", &M);

	  CallInst* callinit = CallInst::Create(threads_init, "", F.getEntryBlock().getTerminator());
	  
      
	  LoopInfo &LI = getAnalysis<LoopInfo>(F);
	  Constant* func1_c;
	  Constant* func2_c;
	  Function* func1;
	  Function* func2;
	
	  for(LoopInfo::iterator lit = LI.begin(), lite = LI.end(); lit != lite; ++lit) {
	    errs() << "Loop found: " << *lit << '\n';


	    for (Loop::block_iterator BB = (*lit)->block_begin(), BBe = (*lit)->block_end(); BB != BBe; ++BB) {
	      errs() << "BB in loop: " << (*BB)->getName() << '\n';
	      //	(*BB)->dump();
	
	      if ((*BB)->getName() == "for.body") {
		
		


		Type* funcArgs[] = { voidPtrTy };
		FunctionType* funcTy = FunctionType::get(voidPtrTy, ArrayRef<Type*>(funcArgs, 1), false);

		Constant* pthreadCreateFun = M.getOrInsertFunction("pthread_create", int32Ty, pthreadTy->getPointerTo(), voidPtrTy,
								   static_cast<Type*>(funcTy)->getPointerTo(),
								   voidPtrTy, (Type*) 0);
		
		Constant* pthreadExit = M.getOrInsertFunction("pthread_exit", voidTy, voidPtrTy, (Type*)0);
	  

		func1_c = M.getOrInsertFunction("func1", funcTy);
		func2_c = M.getOrInsertFunction("func2", funcTy);
		func1 = cast<Function>(func1_c);
		func2 = cast<Function>(func2_c);
		
		BasicBlock* func1Block = BasicBlock::Create(M.getContext(), "entry", func1);
		BasicBlock* func2Block = BasicBlock::Create(M.getContext(), "entry", func2);
		
		// Return instruction of the functions
		Value* retVal = ConstantPointerNull::get(voidPtrTy);
		ReturnInst* ret1 = ReturnInst::Create(M.getContext(), retVal, func1Block);
		ReturnInst* ret2 = ReturnInst::Create(M.getContext(), retVal, func2Block);
		
		
		// Copy instructions into the created functions
		for (int i = 0; i < 4; i++) {
		  BasicBlock::iterator it = (*BB)->begin();
		  it->moveBefore(ret1);
		}
		for (int i = 0; i < 4; i++) {
		  BasicBlock::iterator it = (*BB)->begin();
		  it->moveBefore(ret2);
		}


		/*
		// Create threads to run func1
		std::vector<Value*> pthreadCreateArgs1(4);
		AllocaInst* longint1 = new AllocaInst(int64Ty, NULL, 8, "t1", (*BB)->begin());	
		AllocaInst* threadT1 = new AllocaInst(int64Ty, NULL, 8, "thread1", (*BB)->begin());
		AllocaInst* threadRet1 = new AllocaInst(int32Ty, NULL, 8, "rc1", (*BB)->begin());
		LoadInst* ld1 = new LoadInst(longint1, "ld1", false, 8);
		ld1->insertAfter(longint1);
		IntToPtrInst* itoptr1 = new IntToPtrInst(ld1, voidPtrTy);
		itoptr1->insertAfter(ld1);
       
		pthreadCreateArgs1[0] = threadT1;
		pthreadCreateArgs1[1] = ConstantPointerNull::get(voidPtrTy);
		pthreadCreateArgs1[2] = func1;
		pthreadCreateArgs1[3] = itoptr1;
	  
		CallInst* call1 = CallInst::Create(pthreadCreateFun, ArrayRef<Value*>(pthreadCreateArgs1), "", (*BB)->getTerminator());
		StoreInst* str1 = new StoreInst(call1, threadRet1);
		str1->insertAfter(call1);
		call1->removeFromParent();
		call1->insertAfter(itoptr1);

		std::vector<Value*> pthreadExitArgs1(1);
		pthreadExitArgs1[0] = ConstantPointerNull::get(voidPtrTy);
		CallInst* exit1 = CallInst::Create(pthreadExit, ArrayRef<Value*>(pthreadExitArgs1), "", ret1);
		


		// Create threads to run func2
		std::vector<Value*> pthreadCreateArgs2(4);
		AllocaInst* longint2 = new AllocaInst(int64Ty, NULL, 8, "t2", (*BB)->begin());	
		AllocaInst* threadT2 = new AllocaInst(int64Ty, NULL, 8, "thread2", (*BB)->begin());
		AllocaInst* threadRet2 = new AllocaInst(int32Ty, NULL, 8, "rc2", (*BB)->begin());
		LoadInst* ld2 = new LoadInst(longint2, "ld2", false, 8);
		ld2->insertAfter(longint2);
		IntToPtrInst* itoptr2 = new IntToPtrInst(ld2, voidPtrTy);
		itoptr2->insertAfter(ld2);
       
		pthreadCreateArgs2[0] = threadT2;
		pthreadCreateArgs2[1] = ConstantPointerNull::get(voidPtrTy);
		pthreadCreateArgs2[2] = func2;
		pthreadCreateArgs2[3] = itoptr2;
	  
		CallInst* call2 = CallInst::Create(pthreadCreateFun, ArrayRef<Value*>(pthreadCreateArgs2), "", (*BB)->getTerminator());
		StoreInst* str2 = new StoreInst(call2, threadRet2);
		str2->insertAfter(call2);
		call2->removeFromParent();
		call2->insertAfter(itoptr2);

		std::vector<Value*> pthreadExitArgs2(1);
		pthreadExitArgs2[0] = ConstantPointerNull::get(voidPtrTy);
		CallInst* exit2 = CallInst::Create(pthreadExit, ArrayRef<Value*>(pthreadExitArgs2), "", ret2);

		*/
			
	      }
	    }
	  }




       
	}
	
	/*
	  if (it->getName() == "main") {
	  Type* int32Ty;
	  Type* int64Ty;
	  Type* pthreadTy;
	  PointerType* voidPtrTy;
	  int32Ty = Type::getInt32Ty(M.getContext());
	  int64Ty = Type::getInt64Ty(M.getContext());
	  voidPtrTy = Type::getInt8PtrTy(M.getContext());

	  if (M.getPointerSize() == Module::Pointer64) {
	  pthreadTy = int64Ty;
	  } else {
	  pthreadTy = int32Ty;
	  }
      
	  Type* funArgs[] = { voidPtrTy };
    
	  FunctionType* startRoutineTy = FunctionType::get(voidPtrTy, ArrayRef<Type*>(funArgs, 1), false);
      
	  Constant* pthreadCreateFun = M.getOrInsertFunction("pthread_create", int32Ty, 
	  pthreadTy->getPointerTo(), voidPtrTy, 
	  static_cast<Type*>(startRoutineTy)->getPointerTo(),
	  voidPtrTy, (Type*)0);

	  Constant* pthreadExit = M.getOrInsertFunction("pthread_exit", Type::getVoidTy(M.getContext()), voidPtrTy, (Type*)0);
	  

	  //    errs() << "pthreadCreateFun " << pthreadCreateFun << '\n';

	  // Create an instruction to call pthread_create
	  std::vector<Value*> pthreadCreateArgs(4);
	  AllocaInst* longint = new AllocaInst(int64Ty, NULL, 8, "t", it->getEntryBlock().begin());	
	  AllocaInst* threadT = new AllocaInst(int64Ty, NULL, 8, "threads", it->getEntryBlock().begin());
	  AllocaInst* ret = new AllocaInst(int32Ty, NULL, 8, "rc", it->getEntryBlock().begin());
	  LoadInst* ld = new LoadInst(longint, "ld", false, 8);
	  ld->insertAfter(longint);
	  IntToPtrInst* itoptr = new IntToPtrInst(ld, voidPtrTy);
	  itoptr->insertAfter(ld);
       
	  pthreadCreateArgs[0] = threadT;
	  pthreadCreateArgs[1] = ConstantPointerNull::get(voidPtrTy);
	  pthreadCreateArgs[2] = funToRun;
	  pthreadCreateArgs[3] = itoptr;
	  
	  
	  CallInst* call = CallInst::Create(pthreadCreateFun, ArrayRef<Value*>(pthreadCreateArgs), "", it->getEntryBlock().getTerminator());
	  StoreInst* str = new StoreInst(call, ret);
	  str->insertAfter(call);
	  call->removeFromParent();
	  call->insertAfter(itoptr);
	  
	  std::vector<Value*> pthreadExitArgs(1);
	  pthreadExitArgs[0] = ConstantPointerNull::get(voidPtrTy);
	  CallInst* exit = CallInst::Create(pthreadExit, ArrayRef<Value*>(pthreadExitArgs), "", it->getEntryBlock().getTerminator());
	  
	  }*/
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
