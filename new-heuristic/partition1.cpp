/* partition1.cpp */

#include "Dependencies.h"
#include "getSCC.h"
#include "partition.h"
#include "llvm/ADT/GraphTraits.h"

using namespace llvm;


bool partition::doInitialization(Loop *L, LPPassManager &LPM) {
    function = L->getHeader()->getParent();
    module = function->getParent();
    context = &module->getContext();
    return false;
}

bool partition::runOnLoop(Loop* L, LPPassManager &LPM) {

   // getSCC* scc = &getAnalysis<getSCC>();
   // scc->printNodes();
  
    errs() << "***************************  Loop encountered: ************************" << '\n' << L->getHeader()->getName() << '\n';

    if (function->getName() != "main")
        return false;


    IntegerType* int32Ty = Type::getInt32Ty(*context);
    IntegerType* int64Ty = Type::getInt64Ty(*context);
    FunctionType* funcTy = FunctionType::get(int32Ty, false);
   
    Constant* func1_c;
    Function* func1;

    func1_c = module->getOrInsertFunction("func1", funcTy);
    func1 = cast<Function>(func1_c);
    
    BasicBlock* func1EntryBlock = BasicBlock::Create(*context, "entry.func1", func1);
    AllocaInst* i_var = new AllocaInst(int32Ty, NULL, 4, "i", func1EntryBlock);
    

    Value* liveIn;
    ValueToValueMapTy VMap;
    std::map<BasicBlock *, BasicBlock *> BlockMap;

    for (Loop::block_iterator BB = L->block_begin(), BBe = L->block_end(); BB != BBe; ++BB) {
        BasicBlock* func1Block = CloneBasicBlock(*BB, VMap, ".func1", func1);
        BlockMap[*BB] = func1Block;
        for (BasicBlock::iterator it = func1Block->begin(), ite = func1Block->end(); it != ite; ++it) {
            for (User::op_iterator oit = it->op_begin(), oite = it->op_end(); oit != oite; ++oit) {
                if (VMap[*oit] != NULL) {
                    *oit = VMap[*oit];
                } else {
                    Constant* cons = dyn_cast<Constant>(*oit);
                    BranchInst* br = dyn_cast<BranchInst>(it);
                    if (cons == NULL && br == NULL) {
                        liveIn = *oit;
                        *oit = i_var;
                    }
                }
               
            }
        }
        
        // remove instructions in the main thread
        if ((*BB)->getName() == "for.body") {
            Instruction* term = (*BB)->getTerminator();
            term->removeFromParent();
            for (int i = 0; i < 4; i++) {
                (*BB)->back().eraseFromParent();
            }
            term->insertAfter(&(*BB)->back());
                            
        }
    }


    // set branch instructions to restructure the CFG in created function
    BasicBlock* func1EndBlock = BasicBlock::Create(*context, "if.end.func1", func1); 
    BasicBlock* garbageBB = BasicBlock::Create(*context, "garbage", func1);
    ConstantInt* retVal_g = ConstantInt::get(int32Ty, (uint32_t) 0);
    ReturnInst* ret_g = ReturnInst::Create(*context, retVal_g, garbageBB);

    for (Function::iterator fit = func1->begin(), fite = func1->end(); fit != fite; ++fit) {
        if (fit->getTerminator() == NULL || fit->getName() == "garbage")
            continue;
      
        BranchInst* br = dyn_cast<BranchInst>(fit->getTerminator());
        int numSuccessors = br->getNumSuccessors();
        for (int i = 0; i < numSuccessors; i++) {
            BasicBlock* successor = br->getSuccessor(i);
            if (BlockMap[successor] != NULL) {
                br->setSuccessor(i, BlockMap[successor]);
            } 
            else {
                br->setSuccessor(i, func1EndBlock);
            }
        }

        if (fit->getName() == "for.body.func1") {
            for (int i = 0; i < 4; i++) {
                BasicBlock::iterator it = fit->begin();
                it->moveBefore(ret_g);
            }
        }

        
    }
    garbageBB->eraseFromParent();

    ConstantInt* retVal = ConstantInt::get(int32Ty, (uint32_t) 0);
    ReturnInst::Create(*context, retVal, func1EndBlock);

    BasicBlock* loopHeader = BlockMap.at(L->getHeader());
    BranchInst::Create(loopHeader, func1EntryBlock);
    
    std::vector<Value *> produce_args;
    ConstantInt* val = ConstantInt::get(int64Ty, (uint64_t) 0);
    produce_args.push_back(val);
    val = ConstantInt::get(int32Ty, (uint32_t) 3);
    produce_args.push_back(val);
    Function* pro = module->getFunction("produce");
    CallInst::Create(pro, ArrayRef<Value*>(produce_args), "", func1EndBlock->getTerminator());
   
    std::vector<Value *> consume_args;
    val = ConstantInt::get(int32Ty, (uint32_t) 2);
    consume_args.push_back(val);
    Function* con = module->getFunction("consume");
    CallInst* call = CallInst::Create(con, ArrayRef<Value*>(consume_args), "", func1EntryBlock->getTerminator());
    CastInst* cast = CastInst::CreateIntegerCast(call, int32Ty, true);
    cast->insertAfter(call);
    StoreInst* str = new StoreInst(cast, i_var);
    str->insertAfter(cast);


    // Add synchronization instructions to main thread
    BasicBlock* loopPreheader = L->getLoopPreheader();
    produce_args.clear();
    cast = CastInst::CreatePointerCast(func1, int64Ty);
    cast->insertBefore(loopPreheader->getTerminator());
    produce_args.push_back(cast);
    val = ConstantInt::get(int32Ty, (uint32_t) 0);
    produce_args.push_back(val);
    CallInst::Create(pro, ArrayRef<Value*>(produce_args), "", loopPreheader->getTerminator());
  
    produce_args.clear();
    LoadInst* load = new LoadInst(liveIn, "i", loopPreheader->getTerminator());

    cast = CastInst::CreateIntegerCast(load, int64Ty, true);
    cast->insertBefore(loopPreheader->getTerminator());
    produce_args.push_back(cast);
    val = ConstantInt::get(int32Ty, (uint32_t) 2);
    produce_args.push_back(val);
    CallInst::Create(pro, ArrayRef<Value*>(produce_args), "", loopPreheader->getTerminator());

    return true;
}

void partition::getAnalysisUsage(AnalysisUsage &AU) const {
   // AU.addRequired<getSCC>();
}

bool partition::doFinalization() {

    return false;
}


char partition::ID = 0;
static RegisterPass<partition> Y("partition", "code partition", false, false);
