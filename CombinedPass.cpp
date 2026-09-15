#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "CodeSinking.h"
#include "ConstPropagation.h"

using namespace llvm;

namespace {
  struct CombinedPass : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    CombinedPass() : FunctionPass(ID) {}

    bool Changed = false;

    bool runOnFunction(Function &F) override {
        bool changed = false;

        CodeSinking *CD = nullptr;
        ConstPropagation *CP = nullptr;

        while(true) {
            bool IRChanged = false;

            CD = new CodeSinking();
            CP = new ConstPropagation();

            if(CD->runOnFunction(F)) {
                IRChanged = true;
                changed = true;
            }

            if(CP->runOnFunction(F)) {
                IRChanged = true;
                changed = true;
            }

            delete CD;
            delete CP;

            if(!IRChanged) break;
        }

        return changed;
    }
  };
}

char CombinedPass::ID = 0;
static RegisterPass<CombinedPass> X("combined-pass", "Our combined pass");
