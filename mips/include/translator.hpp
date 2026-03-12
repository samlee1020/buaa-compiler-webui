#ifndef MIPS_TRANSLATOR_HPP
#define MIPS_TRANSLATOR_HPP

#include "MipsManager.hpp"
#include "Instruction.hpp"
#include "InstructionType.hpp"
#include "IrForward.hpp"
#include <iostream>

class Translator {
    public:
        Translator() { manager = new MipsManager(); };
        ~Translator() { delete manager; };

        void translate(ModuleSmartPtr &modulePtr);
        void print(std::ostream &_out);

    private:
        MipsManager *manager;
        void translate(GlobalVariablePtr globalVariablePtr);
        void translate(FunctionPtr functionPtr);
        void translate(BasicBlockPtr basicBlockPtr);
        void translate(InstructionPtr instructionPtr);

        // Instruction.hpp
        void translate(AllocaInstPtr allocaInstPtr);
        void translate(LoadInstPtr loadInstPtr);
        void translate(GepInstPtr gepInstPtr);
        void translate(StoreInstPtr storeInstPtr);
        void translate(BranchInstPtr branchInstPtr);
        void translate(JumpInstPtr jumpInstPtr);
        void translate(ReturnInstPtr returnInstPtr);
        void translate(CallInstPtr callInstPtr);

        // InstructionType.hpp
        void translate(UnaryOperatorPtr unaryOperatorPtr);
        void translate(ZextInstPtr zextInstPtr);
        void translate(BinaryOperatorPtr binaryOperatorPtr);
        void translate(CompareInstructionPtr compareInstructionPtr);
        
};

#endif