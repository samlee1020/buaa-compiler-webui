#ifndef LLVM_IR_INSTRUCTIONTYPE_HPP
#define LLVM_IR_INSTRUCTIONTYPE_HPP

#include "Value.hpp"
#include "Instruction_base.hpp"
#include "LlvmContext.hpp"

/*
    一元指令基类，即只含有一个操作数的指令的基类，如：LoadInst、UnaryOperator
*/
class UnaryInstruction : public Instruction {
    public:
        ~UnaryInstruction() override = default;

        static bool classof(const ValueType type) { return type == ValueType::LoadInstTy || type == ValueType::UnaryOperatorTy; }

        ValuePtr Operand(); // 获取操作数
    
    protected:
        UnaryInstruction(ValueType valueType, TypePtr type, ValuePtr operand);
};

enum class UnaryOpType { Not, Neg, Pos};
/*
    一元运算符，即一元运算符加上一个操作数，如：!1、-2、+3
*/
class UnaryOperator : public UnaryInstruction {
    public:
        ~UnaryOperator() override = default;

        static bool classof(const ValueType type) { return type == ValueType::UnaryOperatorTy; }

        void PrintAsm(AsmWriterSmartPtr out) override; 

        static UnaryOperatorPtr New(UnaryOpType opType, ValuePtr operand);

        UnaryOpType GetOpType() const { return _opType; }

    private:
        UnaryOperator(TypePtr type, ValuePtr operand, UnaryOpType opType) 
            : UnaryInstruction(ValueType::UnaryOperatorTy, type, operand), _opType(opType) {}

        UnaryOpType _opType;
};

/*
    zext可以把一个i1类型扩展为i32类型，形如：
        %1 = zext i1 %0 to i32
*/ 
class ZextInst final : public UnaryInstruction {
    public:
        ~ZextInst() override = default;
        static bool classof(const ValueType type) { return type == ValueType::ZextInstTy; }
        void PrintAsm(AsmWriterSmartPtr out) override;
        static ZextInstPtr New(ValuePtr value);
    private:
        ZextInst(ValuePtr value);
};

/*
    二元表达式基类，即含有两个操作数的表达式的基类，如：BinaryOperator、StoreInst
*/
class BinaryInstruction : public Instruction {
    public:
        ~BinaryInstruction() override = default;

        static bool classof(const ValueType type) { return type == ValueType::BinaryOperatorTy || type == ValueType::StoreInstTy; }

        ValuePtr LeftOperand();
        ValuePtr RightOperand();

    protected:
        BinaryInstruction(ValueType valueType, TypePtr type, ValuePtr lhs, ValuePtr rhs);
};

enum class BinaryOpType { Add, Sub, Mul, Div, Mod };
/*
    二元运算符，即两个操作数的运算符，如：a + b、a - b、a * b、a / b、a % b
*/
class BinaryOperator : public BinaryInstruction {
public:
    ~BinaryOperator() override = default;

    static bool classof(const ValueType type) {
        return type == ValueType::BinaryOperatorTy;
    }

    void PrintAsm(AsmWriterSmartPtr out) override;

    static BinaryOperatorPtr New(BinaryOpType opType, ValuePtr lhs, ValuePtr rhs);

    BinaryOpType OpType() const { return _opType; }

private:
    BinaryOperator(TypePtr type, ValuePtr lhs, ValuePtr rhs, BinaryOpType opType)
        : BinaryInstruction(ValueType::BinaryOperatorTy, type, lhs, rhs), _opType(opType) {}

    BinaryOpType _opType;
};

enum class CompareOpType {
    Equal,
    NotEqual,
    GreaterThan,
    GreaterThanOrEqual,
    LessThan,
    LessThanOrEqual
};
/*
    比较运算符，即两个操作数的比较运算符，如：a == b、a!= b、a > b、a >= b、a < b、a <= b
*/
class CompareInstruction : public BinaryInstruction {
    public:
        ~CompareInstruction() override = default;

        static bool classof(const ValueType type) { return type == ValueType::CompareInstTy;}

        void PrintAsm(AsmWriterSmartPtr out) override;

        static CompareInstructionPtr New(CompareOpType opType, ValuePtr lhs, ValuePtr rhs);

        CompareOpType OpType() const { return _opType; }

    protected:
        CompareInstruction(TypePtr type, ValuePtr lhs, ValuePtr rhs, CompareOpType opType)
            : BinaryInstruction(ValueType::CompareInstTy, type, lhs, rhs), _opType(opType) {}

    private:
        CompareOpType _opType;
};

#pragma endregion


#endif