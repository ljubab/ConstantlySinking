#include "ConstPropagation.h"
void ConstPropagation::findAllInstructions(Function &F){
    for(BasicBlock &BB : F){
        for(Instruction &I: BB){
            ConstPropagationInstruction *CPI_instance = new ConstPropagationInstruction(&I, Variables);
            Instructions.push_back(CPI_instance);

            Instruction* Previous = I.getPrevNonDebugInstruction();

            if(Previous == nullptr){ // na pocetku basic bloka
                for(BasicBlock* BBPred : predecessors(&BB)){
                    Instruction* Terminator = BBPred->getTerminator();
                    CPI_instance->addPredecessor( *std::find_if(Instructions.begin(), Instructions.end(),
                                                [Terminator](ConstPropagationInstruction *CPI) { return CPI->getInstruction() == Terminator; } ) );
                                            // ovde poredimo bas pokazivace pa znamo da je stvarno ista instrukcija, ne samo sa npr. istim imenom
                }
            }
            else{
                CPI_instance->addPredecessor(Instructions[Instructions.size()-2]);
            }
        }
    }
}

void ConstPropagation::findAllVariables(Function &F){
    for(BasicBlock &BB : F){
        for(Instruction &I : BB){
            if(isa<AllocaInst>(&I)){
                Variables.push_back(&I);
            }
        }
    }
}


// C(pi,x,out) = T => C(s,x,in) = T
bool ConstPropagation::checkRule1(ConstPropagationInstruction* CPI_instance, Value* Variable){
    for(ConstPropagationInstruction* Predecessor : CPI_instance->getPredecessors()){
        if(Predecessor->getStatusAfter(Variable) == Top){
            return CPI_instance->getStatusBefore(Variable) == Top;
        }
    }

    return true;
}

void ConstPropagation::applyRule1(ConstPropagationInstruction* CPI_instance, Value* Variable){
    CPI_instance->setStatusBefore(Variable, Top);
}

// C(pi,x,out)==c && C(pj,x,out)==d && c!=d => C(pi,x,in) = T
bool ConstPropagation::checkRule2(ConstPropagationInstruction* CPI_instance, Value* Variable){
    std::unordered_set<int> ConstValues;

    for(ConstPropagationInstruction* Predecessor : CPI_instance->getPredecessors()){
        if(Predecessor->getStatusAfter(Variable) == Const){
            ConstValues.insert(Predecessor->getValueAfter(Variable));
        }
    }

    if(ConstValues.size() > 1){
        return CPI_instance->getStatusBefore(Variable) == Top;
    }

    return true;
}

void ConstPropagation::applyRule2(ConstPropagationInstruction* CPI_instance, Value* Variable){
    CPI_instance->setStatusAfter(Variable, Top);
}

// C(pi,x,out)==c v ~T => C(pi,x,in) = c    ;gde barem jedna instrukcija vraca const c;
bool ConstPropagation::checkRule3(ConstPropagationInstruction* CPI_instance, Value* Variable){
    std::unordered_set<int> ConstValues;

    for(ConstPropagationInstruction* Predecessor : CPI_instance->getPredecessors()){
        if(Predecessor->getStatusAfter(Variable) == Const){
            ConstValues.insert(Predecessor->getValueAfter(Variable));
        }
        if(Predecessor->getStatusAfter(Variable) == Top){
            return true;
        }
    }

    if(ConstValues.size() == 1){
        return CPI_instance->getStatusBefore(Variable) == Const && CPI_instance->getValueBefore(Variable) == *ConstValues.begin() ;
    }

    return true;
}

void ConstPropagation::applyRule3(ConstPropagationInstruction* CPI_instance, Value* Variable, int Value){
    CPI_instance->setStatusBefore(Variable, Const, Value);
}


// C(pi,x,out) == ~T => C(s,x,in) = ~T
bool ConstPropagation::checkRule4(ConstPropagationInstruction* CPI_instance, Value* Variable){
    for(ConstPropagationInstruction* Predecessor : CPI_instance->getPredecessors()){
        if(Predecessor->getStatusAfter(Variable) == Top || Predecessor->getStatusAfter(Variable) == Const){
            return true;
        }
    }

    //specijalan slucaj, samo za prvu instrukciju koja nema predake i postavili smo je na Top da bi algoritam poceo;
    if(CPI_instance->getPredecessors().size() == 0){
        return true;
    }

    return CPI_instance->getStatusBefore(Variable) == Bottom;
}

void ConstPropagation::applyRule4(ConstPropagationInstruction* CPI_instance, Value* Variable){
    CPI_instance->setStatusBefore(Variable, Bottom);
}

// C(s,x,in) == ~T => C(s,x,out) = ~T
bool ConstPropagation::checkRule5(ConstPropagationInstruction* CPI_instance, Value* Variable){
    if(CPI_instance->getStatusBefore(Variable) == Bottom){
        return CPI_instance->getStatusAfter(Variable) == Bottom;
    }

    return true;
}

void ConstPropagation::applyRule5(ConstPropagationInstruction* CPI_instance, Value* Variable){
    CPI_instance->setStatusAfter(Variable, Bottom);
}

// C(x:=c, x, out) = c, c is const. ;dodatak za poziv fje koja garantovano vraca const;
bool ConstPropagation::checkRule6(ConstPropagationInstruction* CPI_instance, Value* Variable){
    Instruction* Instr = CPI_instance->getInstruction();
    if((isa<StoreInst>(Instr) && Instr->getOperand(1)==Variable)){
        if(ConstantInt* ConstInt = dyn_cast<ConstantInt>(Instr->getOperand(0))){ //const ubacujemo u nasu promenljivu
            return CPI_instance->getStatusAfter(Variable)==Const && CPI_instance->getValueAfter(Variable)==ConstInt->getSExtValue();
        }
    }

    return true;
}

void ConstPropagation::applyRule6(ConstPropagationInstruction* CPI_instance, Value* Variable, int Value){
    CPI_instance->setStatusAfter(Variable, Const, Value);
}

// C(x := f(), x, out) = T   ;dodeljuje se vrednost koja nije const;
bool ConstPropagation::checkRule7(ConstPropagationInstruction* CPI_instance, Value* Variable){
    Instruction* Instr = CPI_instance->getInstruction();
    if( isa<StoreInst>(Instr) && Instr->getOperand(1)==Variable ){
        if( !isa<ConstantInt> (Instr->getOperand(0))){
            return CPI_instance->getStatusAfter(Variable)==Top;
        }
    }

    return true;
}

void ConstPropagation::applyRule7(ConstPropagationInstruction* CPI_instance, Value* Variable){
    CPI_instance->setStatusAfter(Variable, Top);
}

// C(y op ... , x, out) = C(y op ... , x, in) if x!=y   ;opearcija ne menja nikako x; mora ovako, da bi se promenio inicijalizovano Bottom stanje;
bool ConstPropagation::checkRule8(ConstPropagationInstruction* CPI_instance, Value* Variable){
    Instruction* Instr = CPI_instance->getInstruction();
    if(isa<StoreInst>(Instr) && Instr->getOperand(1) == Variable)
        return true;

    return CPI_instance->getStatusBefore(Variable) == CPI_instance->getStatusAfter(Variable);
}

void ConstPropagation::applyRule8(ConstPropagationInstruction* CPI_instance, Value* Variable){
    CPI_instance->setStatusAfter(Variable, CPI_instance->getStatusBefore(Variable), CPI_instance->getValueBefore(Variable));
}


void ConstPropagation::propagateVariable(Value* Variable){
    bool RuleApplied;

    while(true){
        RuleApplied=false;

        for(ConstPropagationInstruction* CPI_instance : Instructions){
            if(!checkRule1(CPI_instance, Variable)){
                applyRule1(CPI_instance, Variable);
                RuleApplied=true;
                break;
            }

            if(!checkRule2(CPI_instance, Variable)){
                applyRule2(CPI_instance, Variable);
                RuleApplied=true;
                break;
            }

            if(!checkRule3(CPI_instance, Variable)){
                int Value;
                for(ConstPropagationInstruction* Predecessor : CPI_instance->getPredecessors()){
                    if(Predecessor->getStatusAfter(Variable)==Const){
                        Value = Predecessor->getValueAfter(Variable);
                        break;
                    }
                }
                applyRule3(CPI_instance, Variable, Value);
                RuleApplied=true;
                break;
            }

            if(!checkRule4(CPI_instance, Variable)){
                applyRule4(CPI_instance, Variable);
                RuleApplied=true;
                break;
            }

            if(!checkRule5(CPI_instance, Variable)){
                applyRule5(CPI_instance, Variable);
                RuleApplied=true;
                break;
            }

            if(!checkRule6(CPI_instance, Variable)){
                ConstantInt* ConstInt = dyn_cast<ConstantInt>(CPI_instance->getInstruction()->getOperand(0));
                applyRule6(CPI_instance, Variable, ConstInt->getSExtValue());
                RuleApplied=true;
                break;
            }

            if(!checkRule7(CPI_instance, Variable)){
                applyRule7(CPI_instance, Variable);
                RuleApplied=true;
                break;
            }

            if(!checkRule8(CPI_instance, Variable)){
                applyRule8(CPI_instance, Variable);
                RuleApplied=true;
                break;
            }
        }

        if(RuleApplied==false)
            break;
    }
}

bool ConstPropagation::modifyIR(){
    std::unordered_map<Value*, Value*> VariablesMap;
    bool changed = false;

    for(ConstPropagationInstruction* CPI_instance : Instructions){
        if(isa<LoadInst>(CPI_instance->getInstruction())){
            VariablesMap[CPI_instance->getInstruction()] = CPI_instance->getInstruction()->getOperand(0);
        }
    }

    for(ConstPropagationInstruction* CPI_instance : Instructions){
        Instruction* Instr = CPI_instance->getInstruction();

        if(isa<StoreInst>(Instr)){
            if(CPI_instance->getStatusBefore(VariablesMap[Instr->getOperand(0)])==Const){
                int Value = CPI_instance->getValueBefore(VariablesMap[Instr->getOperand(0)]);
                ConstantInt* ConstInt = ConstantInt::get(Type::getInt32Ty(Instr->getContext()), Value);
                Instr->getOperand(0)->replaceAllUsesWith(ConstInt);
                changed = true;
            }
        }
        else if(isa<BinaryOperator>(Instr) || isa<ICmpInst>(Instr) ){
            Value *Left = Instr->getOperand(0), *Right = Instr->getOperand(1);
            Value *LeftToVar = VariablesMap[Left], *RightToVar = VariablesMap[Right];
            Value *NewValueLeft = nullptr, *NewValueRight = nullptr;

            if(LeftToVar != nullptr && CPI_instance->getStatusBefore(LeftToVar)==Const)
                NewValueLeft = ConstantInt::get(Type::getInt32Ty(Instr->getContext()), CPI_instance->getValueBefore(LeftToVar));

            if(RightToVar != nullptr && CPI_instance->getStatusBefore(RightToVar)==Const)
                NewValueRight = ConstantInt::get(Type::getInt32Ty(Instr->getContext()), CPI_instance->getValueBefore(RightToVar));

            if(NewValueLeft != nullptr) {
                Left->replaceAllUsesWith(NewValueLeft);
                changed = true;
            }


            if(NewValueRight != nullptr) {
                Right->replaceAllUsesWith(NewValueRight);
                changed = true;
            }

        }
    }

    return changed;
}

bool ConstPropagation::runOnFunction(Function &F){
    findAllInstructions(F);
    findAllVariables(F);

    // postavljanje vrednosti promenljivih na Top za prvu instrukciju kako bi algoritam uopste poceo
    for(Value* var : Variables){
        Instructions.front()->setStatusBefore(var, Top);
    }

    // glavni algoritam
    for(Value* Variable : Variables){
        propagateVariable(Variable);
    }

    return modifyIR();
}

char ConstPropagation::ID = 0;
static RegisterPass<ConstPropagation> X("const-propagation", "basic constant propagation; supports only intigers, basic arithmetic, assignment, if/else branches.",
                                    false, false);
