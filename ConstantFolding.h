#ifndef LLVM_CONSTANTFOLDING_H
#define LLVM_CONSTANTFOLDING_H

#include "llvm/IR/Instruction.h"
#include "llvm/Pass.h"

using namespace llvm;

class ConstantFolding : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  ConstantFolding() : FunctionPass(ID) {}
  bool handleBinaryOperator(Instruction &I);
  bool runOnFunction(Function &F) override;
};

#endif