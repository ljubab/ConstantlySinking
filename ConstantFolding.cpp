#include "ConstantFolding.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Function.h"

char ConstantFolding::ID = 0;

bool ConstantFolding::handleBinaryOperator(Instruction &I) {
    if(!isa<BinaryOperator>(&I)) {
        return false;
    }

    Value *Lhs = I.getOperand(0), *Rhs = I.getOperand(1);
    ConstantInt *LhsVal, *RhsVal;
    int Val;

    if(!(LhsVal = dyn_cast<ConstantInt>(Lhs))) {
        return false;
    }

    if(!(RhsVal = dyn_cast<ConstantInt>(Rhs))) {
        return false;
    }

    if(!(LhsVal = dyn_cast<ConstantInt>(Lhs))) {
        return true;
    }

    if(isa<AddOperator>(&I)) {
        Val = LhsVal->getSExtValue() + RhsVal->getSExtValue();
    } else if(isa<SubOperator>(&I)) {
        Val = LhsVal->getSExtValue() - RhsVal->getSExtValue();
    } else if(isa<MulOperator>(&I)) {
        Val = LhsVal->getSExtValue() * RhsVal->getSExtValue();
    } else if(isa<SDivOperator>(&I)) {
        if(RhsVal->getSExtValue() == 0) {
            errs() << "Division by zero!\n";
            exit(1);
        }
        Val =  LhsVal->getSExtValue() / RhsVal->getSExtValue();
    } else {
        return false;
    }

    I.replaceAllUsesWith(ConstantInt::get(Type::getInt32Ty(I.getContext()), Val));
    return true;
}

bool ConstantFolding::runOnFunction(Function &F) {
    bool changed = false;
    std::vector<Instruction*> forDelete;

    for(BasicBlock &BB : F) {
        for(Instruction &I : BB) {
            if(handleBinaryOperator(I)) {
                changed = true;
                forDelete.push_back(&I);
            }
        }
    }

    for(Instruction *I : forDelete)
        I->eraseFromParent();

    return changed;
}