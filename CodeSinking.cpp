#include "CodeSinking.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include <unordered_set>

char CodeSinking::ID = 0;

BasicBlock *CodeSinking::getDestination(Instruction &I, BasicBlock *IfBlock, BasicBlock *ElseBlock) {
    std::unordered_set<BasicBlock*> useBlocks;

    if(auto *StInst = dyn_cast<StoreInst>(&I)) {
        if(auto *AllocInst = dyn_cast<AllocaInst>(StInst->getPointerOperand())) {
            if(!isa<Instruction>(StInst->getValueOperand()))
                return nullptr;

            for(User *UseInst : AllocInst->users()) {
                if(UseInst == StInst) continue;

                auto *LdInst = dyn_cast<LoadInst>(UseInst);

                if(!LdInst)
                    return nullptr;

                useBlocks.insert(LdInst->getParent());
            }
        }
    } else {
        for(User *U : I.users()) {
            if(auto *UseInst = dyn_cast<Instruction>(U)) {
                BasicBlock *UseBlock = UseInst->getParent();

                useBlocks.insert(UseBlock);
            }
        }
    }

    if(useBlocks.size() != 1)
        return nullptr;

    BasicBlock *Dest = *useBlocks.begin();

    if(Dest != IfBlock && Dest != ElseBlock)
        return nullptr;

    return Dest;
}

bool CodeSinking::runOnFunction(Function &F) {
    BasicBlock *Entry = &F.getEntryBlock();
    auto *Branch = dyn_cast<BranchInst>(Entry->getTerminator());

    if(!Branch || !Branch->isConditional())
        return false;

    BasicBlock *IfBlock = Branch->getSuccessor(0);
    BasicBlock *ElseBlock = Branch->getSuccessor(1);

    if(!IfBlock || !ElseBlock)
        return false;

    {
        auto *IfBranch = dyn_cast<BranchInst>(IfBlock->getTerminator());
        auto *ElseBranch = dyn_cast<BranchInst>(ElseBlock->getTerminator());

        if(!IfBranch || !ElseBranch)
            return false;

        if(!IfBranch->isUnconditional() || !ElseBranch->isUnconditional())
            return false;

        BasicBlock *IfSuccessor = IfBranch->getSuccessor(0);
        BasicBlock *ElseSuccessor = ElseBranch->getSuccessor(0);

        if(IfSuccessor != ElseSuccessor)
            return false;
    }

    auto It = Entry->rbegin();
    bool changed = false;

    while(It != Entry->rend()) {
        Instruction *CurrInst = &*It;
        ++It;

        if(isa<AllocaInst>(CurrInst) || isa<BranchInst>(CurrInst) || isa<ReturnInst>(CurrInst))
            continue;

        BasicBlock *Dest = getDestination(*CurrInst, IfBlock, ElseBlock);

        if(!Dest)
            continue;

        IRBuilder<> Builder(Dest, Dest->getFirstInsertionPt());
        CurrInst->removeFromParent();
        Builder.Insert(CurrInst);
        changed = true;
    }

    return changed;
}
