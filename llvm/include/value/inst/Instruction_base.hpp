#ifndef LLVM_VALUE_INST_INSTRUCTION_BASE_HPP
#define LLVM_VALUE_INST_INSTRUCTION_BASE_HPP

#include "User.hpp"
/*
    Instruction是所有指令的基类，继承自User，被BasicBlock持有
*/
class Instruction : public User, public HasParent<BasicBlock> {
    public:
        ~Instruction() override = default;

        void PrintName(AsmWriterSmartPtr out) override;
        void PrintUse(AsmWriterSmartPtr out) override;
        
        void setNotToMips() { _not_to_mips = true; } // 设置不生成mips汇编代码
        bool isNotToMips() { return _not_to_mips; } // 判断是否生成mips汇编代码

    protected:
        Instruction(ValueType valueType, TypePtr type) : User(valueType, type) {}
        bool _not_to_mips = false; // 用于判断是否生成mips汇编代码，默认为false，即生成汇编代码
};

#endif