#ifndef LLVM_FUNCTION_HPP
#define LLVM_FUNCTION_HPP

#include "IrForward.hpp"
#include "Value.hpp"
#include "Constant.hpp"
#include "SlotTracker.hpp"
#include <vector>
#include <list>

/*
    Function继承自GlobalValue，包含Argument和BasicBlock两种子节点
*/
class Function : public GlobalValue {
    public:
        ~Function() override = default;

        static bool classof(const ValueType type) { return type == ValueType::FunctionTy; }

        void PrintAsm(AsmWriterSmartPtr out) override;

        static FunctionPtr New(TypePtr returnType, const std::string &name) {
            return returnType->Context()->SaveValue(new Function(FunctionType::Get(returnType), name));
        }
        static FunctionPtr New(TypePtr returnType, const std::string &name, std::vector<ArgumentPtr> args);
        
        BasicBlockPtr NewBasicBlock(); // 创建一个新的BasicBlock

        TypePtr ReturnType() const { return GetType()->As<FunctionType>()->ReturnType(); }// 获取函数的返回类型

        // 遍历基本块和参数的接口
        using block_iterator = std::list<BasicBlockPtr>::iterator;
        using argument_iterator = std::vector<ArgumentPtr>::iterator;
        int ArgCount() const { return static_cast<int>(_args.size()); }
        ArgumentPtr GetArg(int argNo) const { return _args[argNo]; }
        argument_iterator ArgBegin() { return _args.begin(); }
        argument_iterator ArgEnd() { return _args.end(); }
        int BasicBlockCount() const { return static_cast<int>(_basicBlocks.size()); }
        block_iterator BasicBlockBegin() { return _basicBlocks.begin(); }
        block_iterator BasicBlockEnd() { return _basicBlocks.end(); }

        // 基本块操作接口
        FunctionPtr InsertBasicBlock(BasicBlockPtr block); // 在函数末尾插入一个BasicBlock
        FunctionPtr InsertBasicBlock(block_iterator iter, BasicBlockPtr block); // 在指定位置插入一个BasicBlock
        FunctionPtr RemoveBasicBlock(BasicBlockPtr block); // 从函数中移除一个BasicBlock

        // 插入alloca指令（alloca指令要在函数第一个基本块中）
        void InsertAllocaInstToEntryBlock(InstructionPtr allocaInst);

        // 获取SlotTracker
        SlotTrackerPtr GetSlotTracker() { return &_slotTracker; }


    private:
        Function(TypePtr type, const std::string &name) : GlobalValue(ValueType::FunctionTy, type, name) {}
        Function(TypePtr type, const std::string &name, std::vector<ArgumentPtr> args);

        std::vector<ArgumentPtr> _args;
        std::list<BasicBlockPtr> _basicBlocks;
        SlotTracker _slotTracker;
        
        
};

/*
    Argument继承自Value，表示函数的输入参数
*/
class Argument : public Value, public HasParent<Function> {
    public:
        ~Argument() override = default;

        static bool classof(const ValueType type) { return type == ValueType::ArgumentTy; }

        void PrintAsm(AsmWriterSmartPtr out) override;
        void PrintUse(AsmWriterSmartPtr out) override;

        static ArgumentPtr New(TypePtr type, const std::string &name) { return type->Context()->SaveValue(new Argument(type, name)); }
        
    private:
        Argument(TypePtr type, const std::string &name) : Value(ValueType::ArgumentTy, type, name) {}
};

/*
    BasicBlock继承自Value，表示函数的基本块
    包含多个Instruction指令
*/
class BasicBlock : public Value , public HasParent<Function> {
    public:
        ~BasicBlock() override = default;

        static bool classof(const ValueType type) { return type == ValueType::BasicBlockTy; }

        void PrintAsm(AsmWriterSmartPtr out) override;
        void PrintName(AsmWriterSmartPtr out) override;
        void PrintUse(AsmWriterSmartPtr out) override;

        static BasicBlockPtr New(FunctionPtr parent) { return parent->Context()->SaveValue(new BasicBlock(parent)); }

        // 遍历指令的接口
        using instruction_iterator = std::list<InstructionPtr>::iterator;
        int InstructionCount() const { return static_cast<int>(_instructions.size()); }
        BasicBlockPtr InsertInstruction(InstructionPtr instruction); // 在基本块末尾插入一个指令
        BasicBlockPtr InsertInstruction(instruction_iterator iter, InstructionPtr inst); // 在指定位置插入一个指令
        BasicBlockPtr RemoveInstruction(InstructionPtr instruction); // 从基本块中移除一个指令
        instruction_iterator InstructionBegin() { return _instructions.begin(); }
        instruction_iterator InstructionEnd() { return _instructions.end(); }
    
    private:
        BasicBlock(FunctionPtr parent) : Value(ValueType::BasicBlockTy, parent->Context()->GetLabelTy()), HasParent(parent) {}

        std::list<InstructionPtr> _instructions;
};

#endif