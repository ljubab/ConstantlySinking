#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "CodeSinking.h"

using namespace llvm;

namespace {
  struct CombinedPass : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    CombinedPass() : FunctionPass(ID) {}

    bool Changed = false;

    bool runOnFunction(Function &F) override {
        CodeSinking *CD = new CodeSinking();
        bool changed = false;

        while(true) {
            bool IRChanged = false;

            if(CD->runOnFunction(F)) {
                IRChanged = true;
                changed = true;
            }

            if(!IRChanged) break;
        }

        return changed;
    }
  };
}

char CombinedPass::ID = 0;
static RegisterPass<CombinedPass> X("combined-pass", "Our combined pass");
