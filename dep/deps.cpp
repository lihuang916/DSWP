/* Dependencies.cpp */

#include "Dependencies.h"

using namespace llvm;

bool Dependencies::runOnLoop(Loop *L, LPPassManager &LPM) {
    //Memory dependence analysis
    MemoryDependenceAnalysis &mda = getAnalysis<MemoryDependenceAnalysis>();
    for (Loop::block_iterator BB = L->block_begin(), BBe = L->block_end(); BB != BBe; ++BB) {
       // errs() << "BB in loop: " << (*BB)->getName() << '\n';
        for (BasicBlock::iterator it = (*BB)->begin(), ite = (*BB)->end(); it != ite; ++it) {
            //DATA DEPENDENCIES using use-def
            Instruction * inst = cast<Instruction>(it);
            for (Value::use_iterator ui = it->use_begin(), E = it->use_end(); ui != E; ++ui) {
                Instruction *user = cast<Instruction>(*ui);
                DepEdge(inst,user,REG);
                if(allDeps.find(inst) == allDeps.end()){
                    allDeps[inst] = new std::vector<DepEdge>();
                }
                allDeps[inst]->push_back(DepEdge(inst,user,REG));
                DI.insert(std::make_pair (inst,user));//Dependent instructions are inserted here
               // errs() << *user <<"\n"; 
            }

            //MEMORY DEPENDENCIES using memory dependence analysis
            MemDepResult mdr = mda.getDependency(inst);//Get the dependency
            if(mdr.isDef()){
                Instruction * dep = mdr.getInst();
                if(isa<StoreInst>(inst)){
                    if(isa<LoadInst>(dep)){
                        if(allDeps.find(inst) == allDeps.end()){
                            allDeps[inst] = new std::vector<DepEdge>();
                        }
                        allDeps[inst]->push_back(DepEdge(inst,dep,MEMFLOW));//Memory flow
                        DI.insert(std::make_pair(inst,dep));
                    }

                    if(isa<StoreInst>(dep)){
                        if(allDeps.find(inst) == allDeps.end()){
                            allDeps[inst] = new std::vector<DepEdge>();
                        }

                        allDeps[inst]->push_back(DepEdge(inst,dep,MEMOUT));//Memory output
                        DI.insert(std::make_pair(inst,dep));
                    }
                }

                if(isa<LoadInst>(inst)){
                    if(isa<StoreInst>(dep)){
                        if(allDeps.find(inst) == allDeps.end()){
                            allDeps[inst] = new std::vector<DepEdge>();
                        }

                        allDeps[inst]->push_back(DepEdge(inst,dep,MEMANTI));//Memory anti
                        DI.insert(std::make_pair(inst,dep));
                    }
                }
            }
	  		
        }  // end of basic block
    }  // end of loop iterator
 
    // CONTROL DEPENDENCIES
    // Populate the list backedge parents
    LoopInfo *LI = &getAnalysis<LoopInfo>();
    for (Loop::block_iterator BB = L->block_begin(), BBe = L->block_end(); BB != BBe; ++BB) {
        for (BasicBlock::iterator it = (*BB)->begin(), ite = (*BB)->end(); it != ite; ++it) {
         //   Instruction * inst = cast<Instruction>(it);
            BranchInst *br = dyn_cast<BranchInst> (it);
            if (br) {
                Loop *instLoc = LI->getLoopFor(*BB);//The basic block to which the loop is located
                bool branches = false;
                for (int j = br->getNumSuccessors() - 1; j >= 0	&& !branches; j--) {
                    if (LI->getLoopFor(br->getSuccessor(j)) != instLoc) { //if the loop in which the successor is present is different from that of the branch instructions, then we call we say that instruction is the parent of the backedge 
                        branches = true;
                    }
                }
                if (branches) {
                    backEdgeParents.push_back(br);
                }
            }//end if br
        } //end inner for
    }//end outer for
     
    //Insert control dependencies between the branch instructions and it's successors

    for (Loop::block_iterator BBIter = L->block_begin(), BBe = L->block_end(); BBIter != BBe; ++BBIter) {
      	BasicBlock *BB = *BBIter;
        Instruction* ter = (*BBIter)->getTerminator();
	      BranchInst *br = dyn_cast<BranchInst> (ter);
	      if(br){
            if(br->getNumSuccessors() > 1){
                BasicBlock* succBB = br->getSuccessor(0);
                if(LI->getLoopDepth(BB) < LI->getLoopDepth(BB)){//This makes sure that control dependecies are not added across nested loops
                    continue;
                }
		while(succBB != NULL && (LI->getLoopDepth(BB) >= LI->getLoopDepth(succBB)) && (succBB != (BB))){
                    //CASE1
                    for(BasicBlock:: iterator succIt = succBB->begin(); succIt != succBB->end(); succIt++){
		      	if(allDeps.find(ter) == allDeps.end()){
                            allDeps[ter] = new std::vector<DepEdge>();
			}
			allDeps[ter]->push_back(DepEdge(ter,succIt,CONTROL));
			DI.insert(std::make_pair(ter,succIt));
                    }
                    //Going to the next successor
                    //Check if the next successor has a conditional branch or an unconditional branch

                    BranchInst *succBr = dyn_cast<BranchInst>(succBB->getTerminator());
                    if(succBr){
                        if(succBr->getNumSuccessors() > 1){
                            if(allDeps.find(ter) == allDeps.end()){
                                allDeps[ter] = new std::vector<DepEdge>();
                            }       
                            allDeps[ter]->push_back(DepEdge(ter,succBB->getTerminator(),CONTROL));
                            DI.insert(std::make_pair(ter,succBB->getTerminator()));
                        }
                        else if(succBr->getNumSuccessors() == 1){
                            succBB = succBr->getSuccessor(0);
                        }
                        else{
                            succBB = NULL;
                        }
                    }
                    else{
                        succBB = NULL;
                    }
                }
            }
	}

    }


	 
    for (std::list<BranchInst*>::iterator exitIter = backEdgeParents.begin(); exitIter != backEdgeParents.end(); ++exitIter) {
        BranchInst *exitBranch = *exitIter;
        if (exitBranch->isConditional()) {
            BasicBlock *header = LI->getLoopFor(exitBranch->getParent())->getHeader();
            for (BasicBlock::iterator ctrlIter = header->begin(); ctrlIter != header->end(); ++ctrlIter) {
                if(allDeps.find(exitBranch) == allDeps.end()){
                    allDeps[exitBranch] = new std::vector<DepEdge>();
                }
                allDeps[exitBranch]->push_back(DepEdge(exitBranch,&(*ctrlIter),CONTROL));

            }
        }
    }

    //printDeps();

    return false;
}
   
void Dependencies::printSet(std::set<std::pair<Instruction*,Instruction*> > DI){
    for(std::set<std::pair<Instruction*, Instruction*> > :: iterator j = DI.begin(); j != DI.end(); j++){
	errs() << "1:" << *(j->first) << "\t" << "2:" << *(j->second) << "\n";
    }
}

void Dependencies::printDeps(){
    for(std::map<Instruction *, std::vector<DepEdge>*> :: iterator j = allDeps.begin(); j != allDeps.end(); j++){
	std::vector<DepEdge>* edges = j->second;
	for(std::vector<DepEdge> :: iterator k = edges->begin(); k != edges->end(); k++){
            errs() << "1:" << *(k->i1) << "\t" << "2:" << *(k->i2) << "\t" << "Type:" << k->t << "\n"; 
	}
    }
}


void Dependencies::getAnalysisUsage(AnalysisUsage& AU) const {
    AU.addRequired<DominatorTree>();
    AU.addPreserved<DominatorTree>();
    AU.addRequired<MemoryDependenceAnalysis>();
    AU.addRequired<LoopInfo>();
}

char Dependencies::ID = 0;
static RegisterPass<Dependencies> Y("deps", "finding dependencies", false, false);
