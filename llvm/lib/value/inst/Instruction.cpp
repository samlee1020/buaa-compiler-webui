#include "Instruction.hpp"
#include "Value.hpp"
#include "Function.hpp"

#pragma region AllocaInst

AllocaInstPtr AllocaInst::New(TypePtr type) { return type->Context()->SaveValue(new AllocaInst(type)); }

AllocaInst::AllocaInst(TypePtr type) : Instruction(ValueType::AllocaInstTy, type->Context()->GetPointerType(type)) {}

#pragma endregion

#pragma region LoadInst

LoadInstPtr LoadInst::New(ValuePtr address) {
    auto type = address->GetType()->As<PointerType>()->ElementType();
    return address->Context()->SaveValue(new LoadInst(type, address));
}

ValuePtr LoadInst::Address() { return OperandAt(0); }

LoadInst::LoadInst(TypePtr type, ValuePtr address) : UnaryInstruction(ValueType::LoadInstTy, type, address) {}

#pragma endregion

#pragma region GepInst

GepInstPtr GepInst::New(ValuePtr address, ValuePtr index) { return address->Context()->SaveValue(new GepInst(address, index)); }

ValuePtr GepInst::Address() { return OperandAt(0); }
ValuePtr GepInst::Index() { return OperandAt(1); }

GepInst::GepInst(ValuePtr address, ValuePtr index) 
    : BinaryInstruction(ValueType::GepInstTy,
        address->Context()->GetPointerType(address->Context()->GetInt32Ty()), // int32*
        address, index) {}

#pragma endregion

#pragma region StoreInst

StoreInstPtr StoreInst::New(ValuePtr value, ValuePtr address) { return address->Context()->SaveValue(new StoreInst(value, address)); }

StoreInst::StoreInst(ValuePtr value, ValuePtr address) 
        : BinaryInstruction(ValueType::StoreInstTy, value->Context()->GetVoidTy(), value, address) {}

#pragma endregion

#pragma region BranchInst

BranchInstPtr BranchInst::New(ValuePtr condition, BasicBlockPtr trueBlock, BasicBlockPtr falseBlock) {
    return condition->Context()->SaveValue(new BranchInst(condition, trueBlock, falseBlock));
}

BasicBlockPtr BranchInst::SetTrueBlock(BasicBlockPtr block) {
    BasicBlockPtr old = _trueBlock;

    if (_trueBlock) {
        ReplaceOperand(_trueBlock, block);
    }
    _trueBlock = block;
    if (_trueBlock) {
        AddOperand(_trueBlock);
    }

    return old;
}

BasicBlockPtr BranchInst::SetFalseBlock(BasicBlockPtr block) {
    BasicBlockPtr old = _falseBlock;

    if (_falseBlock) {
        ReplaceOperand(_falseBlock, block);
    }
    _falseBlock = block;
    if (_falseBlock) {
        AddOperand(_falseBlock);
    }

    return old;
}

BranchInst::BranchInst(ValuePtr condition, BasicBlockPtr trueBlock, BasicBlockPtr falseBlock)
    : Instruction(ValueType::BranchInstTy, condition->Context()->GetVoidTy()), 
    _condition(condition), _trueBlock(trueBlock), _falseBlock(falseBlock) {
    AddOperand(condition);
    if (trueBlock) {
        AddOperand(trueBlock);
    }
    if (falseBlock) {
        AddOperand(falseBlock);
    }
}

#pragma endregion

#pragma region JumpInst

BasicBlockPtr JumpInst::SetTarget(BasicBlockPtr block) {
    BasicBlockPtr old = _target;

    if (_target) {
        ReplaceOperand(_target, block);
    }
    _target = block;
    if (_target) {
        AddOperand(_target);
    }

    return old;
}

JumpInstPtr JumpInst::New(BasicBlockPtr target) { return target->Context()->SaveValue(new JumpInst(target)); }

JumpInstPtr JumpInst::New(LlvmContextPtr context) { return context->SaveValue(new JumpInst(context)); }

JumpInst::JumpInst(BasicBlockPtr target) : Instruction(ValueType::JumpInstTy, target->Context()->GetVoidTy()), _target(target) {
    AddOperand(target);
}

JumpInst::JumpInst(LlvmContextPtr context) : Instruction(ValueType::JumpInstTy, context->GetVoidTy()), _target(nullptr) {}

#pragma endregion

#pragma region ReturnInst

ValuePtr ReturnInst::ReturnValue() {
    if (OperandCount() == 0) {
        return nullptr;
    }
    return OperandAt(0);
}

ReturnInst::ReturnInst(TypePtr type, ValuePtr value)
    : Instruction(ValueType::ReturnInstTy, type) {
    if (!value->GetType()->IsVoidTy()) {
        AddOperand(value);
    }
}

#pragma endregion

#pragma region CallInst 

CallInst::CallInst(FunctionPtr function, const std::vector<ValuePtr> &parameters)
    : Instruction(ValueType::CallInstTy, function->ReturnType()), _function(function) {
    for (auto param : parameters) {
        AddOperand(param);
    }
}

CallInst::CallInst(FunctionPtr function) : Instruction(ValueType::CallInstTy, function->ReturnType()), _function(function) {}

CallInstPtr CallInst::New(FunctionPtr function, const std::vector<ValuePtr> &params) {
    return function->Context()->SaveValue(new CallInst(function, params));
}
CallInstPtr CallInst::New(FunctionPtr function) {
    return function->Context()->SaveValue(new CallInst(function));
}

#pragma endregion

#pragma region ReturnInst

ReturnInstPtr ReturnInst::New(ValuePtr value) {
    return value->Context()->SaveValue(new ReturnInst(value->Context()->GetVoidTy(), value));
}

ReturnInstPtr ReturnInst::New(LlvmContextPtr context) {
    return context->SaveValue(new ReturnInst(context->GetVoidTy()));
}

#pragma endregion