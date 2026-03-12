#include "Function.hpp"
#include "IrForward.hpp"
#include "Instruction.hpp"

#pragma region Function

FunctionPtr Function::New(TypePtr returnType, const std::string &name, std::vector<ArgumentPtr> args) {
    std::vector<TypePtr> argTypes;
    for (auto arg : args) {
        argTypes.push_back(arg->GetType());
    }
    return returnType->Context()->SaveValue(new Function(FunctionType::Get(returnType, argTypes), name, args));
}

Function::Function(TypePtr type, const std::string &name, std::vector<ArgumentPtr> args)
    : GlobalValue(ValueType::FunctionTy, type, name), _args(args) {
    for (auto arg : args) {
        arg->SetParent(this);
    }
}

BasicBlockPtr Function::NewBasicBlock() {
    auto block = BasicBlock::New(this);
    InsertBasicBlock(block);
    return block;
}

FunctionPtr Function::InsertBasicBlock(BasicBlockPtr block) {
    _basicBlocks.push_back(block);
    return this;
}

FunctionPtr Function::InsertBasicBlock(block_iterator iter, BasicBlockPtr block) {
    _basicBlocks.insert(iter, block);
    return this;
}

FunctionPtr Function::RemoveBasicBlock(BasicBlockPtr block) {
    _basicBlocks.remove(block);
    return this;
}

#pragma endregion

#pragma region BasicBlock

BasicBlockPtr BasicBlock::InsertInstruction(InstructionPtr instruction) {
    instruction->SetParent(this);
    _instructions.push_back(instruction);
    return this;
}

BasicBlockPtr BasicBlock::InsertInstruction(instruction_iterator iter,
                                            InstructionPtr instruction) {
    instruction->SetParent(this);
    _instructions.insert(iter, instruction);
    return this;
}

BasicBlockPtr BasicBlock::RemoveInstruction(InstructionPtr instruction) {
    instruction->RemoveParent();
    _instructions.remove(instruction);
    return this;
}

#pragma endregion