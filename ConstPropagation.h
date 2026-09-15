#ifndef LLVM_CONSTPROPAGATION_H
#define LLVM_CONSTPROPAGATION_H

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Pass.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/IRBuilder.h"
#include <vector>
#include "ConstPropagationInstruction.h"
#include <unordered_set>

using namespace llvm;

class ConstPropagation : public FunctionPass {
private:
    std::vector<Value* > Variables;
    std::vector<ConstPropagationInstruction* > Instructions;

    void findAllInstructions(Function &F);
    void findAllVariables(Function &F);

    bool checkRule1(ConstPropagationInstruction* CPI_instance, Value* Variable);
    void applyRule1(ConstPropagationInstruction* CPI_instance, Value* Variable);

    bool checkRule2(ConstPropagationInstruction* CPI_instance, Value* Variable);
    void applyRule2(ConstPropagationInstruction* CPI_instance, Value* Variable);

    bool checkRule3(ConstPropagationInstruction* CPI_instance, Value* Variable);
    void applyRule3(ConstPropagationInstruction* CPI_instance, Value* Variable, int Value);

    bool checkRule4(ConstPropagationInstruction* CPI_instance, Value* Variable);
    void applyRule4(ConstPropagationInstruction* CPI_instance, Value* Variable);

    bool checkRule5(ConstPropagationInstruction* CPI_instance, Value* Variable);
    void applyRule5(ConstPropagationInstruction* CPI_instance, Value* Variable);

    bool checkRule6(ConstPropagationInstruction* CPI_instance, Value* Variable);
    void applyRule6(ConstPropagationInstruction* CPI_instance, Value* Variable, int Value);

    bool checkRule7(ConstPropagationInstruction* CPI_instance, Value* Variable);
    void applyRule7(ConstPropagationInstruction* CPI_instance, Value* Variable);

    bool checkRule8(ConstPropagationInstruction* CPI_instance, Value* Variable);
    void applyRule8(ConstPropagationInstruction* CPI_instance, Value* Variable);

    void propagateVariable(Value* Variable);
    bool modifyIR();

public:
  static char ID; // Pass identification, replacement for typeid
  ConstPropagation() : FunctionPass(ID) {}
  bool runOnFunction(Function &F) override;
};

#endif
