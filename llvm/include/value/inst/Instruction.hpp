#ifndef LLVM_IR_INSTRUCTION_HPP
#define LLVM_IR_INSTRUCTION_HPP

#include "IrForward.hpp"
#include "InstructionType.hpp"

/*
    AllocaInst负责分配内存，返回值是指针类型
    形如：
        %1 = alloca i32
        %2 = alloca [10 x i32]
*/
class AllocaInst : public Instruction {
    public:
        ~AllocaInst() override = default;

        static bool classof(const ValueType type) { return type == ValueType::AllocaInstTy; }

        void PrintAsm(AsmWriterSmartPtr out) override;

        static AllocaInstPtr New(TypePtr type);

        TypePtr AllocatedType() const { return GetType()->As<PointerType>()->ElementType(); }

        void SetMipsOffsetToSp(int offset) { _mips_offset_to_sp = offset; }
        int GetMipsOffsetToSp() const { return _mips_offset_to_sp; }

    private:
        AllocaInst(TypePtr type);
        int _mips_offset_to_sp; // 记录alloca变量在mips栈中的位置
};

/*
    LoadInst负责从内存中读取数据，继承自UnaryInstruction
    形如：
        %1 = load i32, i32* %2
*/
class LoadInst : public UnaryInstruction {
    public:
        ~LoadInst() override = default;

        static bool classof(const ValueType type) { return type == ValueType::LoadInstTy; }

        void PrintAsm(AsmWriterSmartPtr out) override;

        static LoadInstPtr New(ValuePtr address);

        ValuePtr Address();

        void setLoadFromS7() { _load_from_s7 = true; } // 记录是否从s7寄存器读取
        bool isLoadFromS7() { return _load_from_s7; } // 是否从s7寄存器读取

    private:
        LoadInst(TypePtr type, ValuePtr address);
        bool _load_from_s7 = false; // 记录是否从s7寄存器读取（短路求值结果存入s7）
};

/*
    GepInst负责计算一维数组的偏移量，继承自Instruction，具有两个操作数addr和index
    形如：
        %1 = getelementptr 数组类型, 数组类型* %addr, i32 0, i32 index
*/
class GepInst final : public BinaryInstruction {
    public:
        ~GepInst() override = default;

        static bool classof(const ValueType type) { return type == ValueType::GepInstTy; }

        void PrintAsm(AsmWriterSmartPtr out) override;

        static GepInstPtr New(ValuePtr address, ValuePtr index);

        ValuePtr Address();
        ValuePtr Index();

    private:
        GepInst(ValuePtr address, ValuePtr index);
};

/*
    StoreInst负责将数据存入内存，继承自BinaryInstruction
    形如：
        store i32 0, i32* %1    存常数
        store i32 %4, i32* %3   存变量
*/
class StoreInst final : public BinaryInstruction {
    public:
        ~StoreInst() override = default;

        static bool classof(const ValueType type) { return type == ValueType::StoreInstTy; }

        void PrintAsm(AsmWriterSmartPtr out) override;

        static StoreInstPtr New(ValuePtr value, ValuePtr address);

        void setStoreToS7() { _store_to_s7 = true; } // 记录是否存入s7寄存器
        bool isStoreToS7() { return _store_to_s7; } // 是否存入s7寄存器

    private:
        StoreInst(ValuePtr value, ValuePtr address);
        bool _store_to_s7 = false; // 记录是否存入s7寄存器（短路求值结果存入s7）
};

/*
    BranchInst负责if-else分支，继承自Instruction
*/
class BranchInst : public Instruction {
    public:
        ~BranchInst() override = default;

        static bool classof(const ValueType type) { return type == ValueType::BranchInstTy; }

        void PrintAsm(AsmWriterSmartPtr out) override;

        static BranchInstPtr New(ValuePtr condition, BasicBlockPtr trueBlock, BasicBlockPtr falseBlock);

        ValuePtr Condition() const { return _condition; }
        BasicBlockPtr TrueBlock() const { return _trueBlock; }
        BasicBlockPtr SetTrueBlock(BasicBlockPtr block);
        BasicBlockPtr FalseBlock() const { return _falseBlock; }
        BasicBlockPtr SetFalseBlock(BasicBlockPtr block);

    private:
        BranchInst(ValuePtr condition, BasicBlockPtr trueBlock, BasicBlockPtr falseBlock);

        ValuePtr _condition;
        BasicBlockPtr _trueBlock;
        BasicBlockPtr _falseBlock;
};

/*
    JumpInst负责无条件跳转，继承自Instruction
*/
class JumpInst final : public Instruction {
    public:
        ~JumpInst() override = default;

        static bool classof(const ValueType type) {
            return type == ValueType::JumpInstTy;
        }

        void PrintAsm(AsmWriterSmartPtr out) override;

        static JumpInstPtr New(BasicBlockPtr target);
        static JumpInstPtr New(LlvmContextPtr context);

        BasicBlockPtr Target() const { return _target; }
        BasicBlockPtr SetTarget(BasicBlockPtr block);

    private:
        JumpInst(BasicBlockPtr target);
        JumpInst(LlvmContextPtr context);
        BasicBlockPtr _target;
};

/*
    ReturnInst负责返回，继承自Instruction
*/
class ReturnInst final : public Instruction {
    public:
        ~ReturnInst() override = default;

        static bool classof(const ValueType type) {
            return type == ValueType::ReturnInstTy;
        }

        void PrintAsm(AsmWriterSmartPtr out) override;

        static ReturnInstPtr New(ValuePtr value);
        static ReturnInstPtr New(LlvmContextPtr context);

        ValuePtr ReturnValue();

    private:
        ReturnInst(TypePtr type, ValuePtr value);
        ReturnInst(TypePtr type) : Instruction(ValueType::ReturnInstTy, type) {}
};

/*
    CallInst负责调用函数，继承自Instruction，具有多个操作数
    形如：
        %6 = call i32 @add(i32 %4, i32 2)
*/

class CallInst final : public Instruction {
    public:
        static bool classof(const ValueType type) { return type == ValueType::CallInstTy; }

        void PrintAsm(AsmWriterSmartPtr out) override;

        static CallInstPtr New(FunctionPtr function, const std::vector<ValuePtr> &params);
        static CallInstPtr New(FunctionPtr function);

        FunctionPtr GetFunction() const { return _function; }

    private:
        CallInst(FunctionPtr function, const std::vector<ValuePtr> &parameters);
        CallInst(FunctionPtr function);

        FunctionPtr _function;
};

#endif