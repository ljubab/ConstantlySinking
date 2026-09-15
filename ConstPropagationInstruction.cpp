#include "ConstPropagationInstruction.h"

ConstPropagationInstruction::ConstPropagationInstruction(Instruction* Instr, std::vector<Value* >& vars) {
    this->Instr = Instr;

    for(Value* var : vars ){
        setStatusBefore(var, Bottom);
        setStatusAfter(var, Bottom);
    }
}

void ConstPropagationInstruction::setStatusAfter(Value* var, Status s, int val){
    StatusAfter[var] = {s, val};
}

void ConstPropagationInstruction::setStatusBefore(Value* var, Status s, int val){
    StatusBefore[var] = {s, val};
}

Status ConstPropagationInstruction::getStatusAfter(Value* var) {
    return StatusAfter[var].first;
}

Status ConstPropagationInstruction::getStatusBefore(Value* var) {
    return StatusBefore[var].first;
}

int ConstPropagationInstruction::getValueAfter(Value* var) {
    return StatusAfter[var].second;
}

int ConstPropagationInstruction::getValueBefore(Value* var) {
    return StatusBefore[var].second;
}

void ConstPropagationInstruction::addPredecessor(ConstPropagationInstruction* predak){
    Predecessors.push_back(predak);
}

Instruction* ConstPropagationInstruction::getInstruction(){
    return Instr;
}

std::vector<ConstPropagationInstruction* > ConstPropagationInstruction::getPredecessors(){
    return Predecessors;
}