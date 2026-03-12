#ifndef LLVM_IR_USER_HPP
#define LLVM_IR_USER_HPP

#include "Value.hpp"

/*
    User继承自Value，所有有操作数的指令都继承自User
*/
class User : public Value {
    public:
        ~User() override = default;

        // 判断是否为User类型
        static bool classof(ValueType type) {
            return type >= ValueType::BinaryOperatorTy;
        }

        // 添加/移除/替换操作数
        void AddOperand(ValuePtr value);
        ValuePtr RemoveOperand(ValuePtr value);
        ValuePtr ReplaceOperand(ValuePtr oldValue, ValuePtr newValue);
        ValuePtr OperandAt(int index);
        int OperandCount() const;

        use_iterator UseBegin() { return _useList.begin(); }
        use_iterator UseEnd() { return _useList.end(); }
        UseListPtr GetUseList() { return &_useList; }

    protected:
        void AddUse(ValuePtr use);
        ValuePtr RemoveUse(ValuePtr use);
        ValuePtr ReplaceUse(ValuePtr oldValue, ValuePtr newValue);

        User(ValueType valueType, TypePtr type) : Value(valueType, type) {}

        UseList _useList;
};

#endif