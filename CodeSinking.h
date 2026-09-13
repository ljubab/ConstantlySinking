#ifndef LLVM_CODESINKING_H
#define LLVM_CODESINKING_H

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Pass.h"

using namespace llvm;

class CodeSinking : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  CodeSinking() : FunctionPass(ID) {}
  BasicBlock* getDestination(Instruction &I, BasicBlock *IfBlock, BasicBlock *ElseBlock);
  bool runOnFunction(Function &F) override;
};

#endif