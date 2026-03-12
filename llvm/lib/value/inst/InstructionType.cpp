#include "InstructionType.hpp"

ValuePtr UnaryInstruction::Operand() { return OperandAt(0); }

UnaryInstruction::UnaryInstruction(ValueType valueType, TypePtr type, ValuePtr operand) : Instruction(valueType, type) {
    AddOperand(operand);
}

UnaryOperatorPtr UnaryOperator::New(UnaryOpType opType, ValuePtr operand) {
    auto type = operand->GetType();
    return operand->Context()->SaveValue(
        new UnaryOperator(type, operand, opType));
}

ZextInstPtr ZextInst::New(ValuePtr value) {
    return value->Context()->SaveValue(new ZextInst(value));
}

ZextInst::ZextInst(ValuePtr value) : UnaryInstruction(ValueType::ZextInstTy, value->Context()->GetInt32Ty(), value) {}

ValuePtr BinaryInstruction::LeftOperand() { return OperandAt(0); }

ValuePtr BinaryInstruction::RightOperand() { return OperandAt(1); }

BinaryInstruction::BinaryInstruction(ValueType valueType, TypePtr type, ValuePtr lhs, ValuePtr rhs) : Instruction(valueType, type) {
    AddOperand(lhs);
    AddOperand(rhs);
}

BinaryOperatorPtr BinaryOperator::New(BinaryOpType opType, ValuePtr lhs, ValuePtr rhs) {
    auto type = lhs->GetType();
    return lhs->Context()->SaveValue(new BinaryOperator(type, lhs, rhs, opType));
}

CompareInstructionPtr CompareInstruction::New(CompareOpType opType, ValuePtr lhs, ValuePtr rhs) {
    auto type = lhs->Context()->GetInt1Ty();
    return lhs->Context()->SaveValue(new CompareInstruction(type, lhs, rhs, opType));
}