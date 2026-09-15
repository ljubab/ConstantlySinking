#ifndef CONSTPROPAGATION_CONSTPROPAGATIONINSTRUCTION_H
#define CONSTPROPAGATION_CONSTPROPAGATIONINSTRUCTION_H

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include <unordered_map>
#include <vector>

using namespace llvm;

enum Status {
    Top,
    Bottom,
    Const
};

class ConstPropagationInstruction {
    private:
        Instruction *Instr;

        // mape za pristup vrednosti/statusu promenljive pre i posle instrukcije; ako je const vrednost se cuva u pair.second;
        std::unordered_map<Value *, std::pair<Status, int>> StatusBefore;
        std::unordered_map<Value *, std::pair<Status, int>> StatusAfter;
        std::vector<ConstPropagationInstruction* > Predecessors;

    public:
        ConstPropagationInstruction(Instruction *Instr, std::vector<Value* >&);

        // pristup vrednosti promenljive pre i posle instrukcije
        Status getStatusBefore(Value * var);
        Status getStatusAfter(Value * var);

        int getValueBefore(Value * var);
        int getValueAfter(Value * var);

        void setStatusBefore(Value* var, Status s, int val = -1);
        void setStatusAfter(Value* var, Status s, int val = -1);

        void addPredecessor(ConstPropagationInstruction* );

        Instruction* getInstruction();

        void addVariables(std::vector<Value *>&);  //inicijalizacija promenljivih za instrukciju
        std::vector<ConstPropagationInstruction* > getPredecessors();
};


#endif //CONSTPROPAGATION_CONSTPROPAGATIONINSTRUCTION_H