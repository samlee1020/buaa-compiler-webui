#ifndef LLVM_CONSTANT_HPP
#define LLVM_CONSTANT_HPP

#include "Value.hpp"
#include "IrForward.hpp"
#include "LlvmContext.hpp"
#include "Type.hpp"
/*
    Constant继承自Value类，包含ConstantInt、ConstantArray、GlobalValue
*/
class Constant : public Value {
    public:
        ~Constant() override = default;

        static bool classof(const ValueType type) { return type >= ValueType::ConstantTy; }

    protected:
        Constant(ValueType valueType, TypePtr type) : Value(valueType, type) {}
};

/*
    ConstantInt继承自Constant类，包含整数常量
*/
class ConstantInt : public Constant {
    public:
        ~ConstantInt() override = default;

        static bool classof(const ValueType type) { return type == ValueType::ConstantIntTy; }
        void PrintAsm(AsmWriterSmartPtr out) override;
        void PrintName(AsmWriterSmartPtr out) override;
        static ConstantIntPtr New(TypePtr type, int intValue) {
            return type->Context()->SaveValue(new ConstantInt(type, intValue));
        }
        int GetIntValue() const { return _intValue; }
    
    private:
        ConstantInt(TypePtr type, int intValue) : Constant(ValueType::ConstantIntTy, type), _intValue(intValue) {}
        
        int _intValue;
};

/*
    ConstantArray继承自Constant类，包含数组常量
*/
class ConstantArray : public Constant {
    public:
        ~ConstantArray() override = default;

        static bool classof(const ValueType type) { return type == ValueType::ConstantArrayTy; }
        void PrintAsm(AsmWriterSmartPtr out) override;
        void PrintName(AsmWriterSmartPtr out) override;
        static ConstantArrayPtr New(TypePtr type, std::vector<ConstantIntPtr> constantInts) {
            return type->Context()->SaveValue(new ConstantArray(type, constantInts));
        }
        const std::vector<ConstantIntPtr>& GetConstantInts() const { return _constantInts; }
        bool isZero() { return _constantInts.empty(); }
    
    private:
        ConstantArray(TypePtr type, std::vector<ConstantIntPtr> constantInts) : Constant(ValueType::ConstantArrayTy, type), _constantInts(constantInts) {}
        std::vector<ConstantIntPtr> _constantInts;
};

/*
    GlobalValue继承自Constant类，被GlobalVariable、Function类继承
*/
class GlobalValue : public Constant {
    friend class Module;

    public:
        ~GlobalValue() override = default;

        static bool classof(const ValueType type) {
            return ValueType::FunctionTy <= type && type <= ValueType::GlobalVariableTy;
        }

        void PrintName(AsmWriterSmartPtr out) override;

    protected:
        GlobalValue(ValueType valueType, TypePtr type, const std::string &name) : Constant(valueType, type) {
            SetName(name);
        }
};

/*
    GlobalVariable继承自GlobalValue类，包含全局变量
    考虑到源代码只会有一个文件，所以dso_local和internal属性暂时不考虑
*/
class GlobalVariable : public GlobalValue {
    public:
        ~GlobalVariable() override = default;

        static bool classof(const ValueType type) { return type == ValueType::GlobalVariableTy; }
        void PrintAsm(AsmWriterSmartPtr out) override;

        static GlobalVariablePtr New(TypePtr type, const std::string &name, bool isConstant, ConstantIntPtr initialInt) {
            return type->Context()->SaveValue(new GlobalVariable(type, name, isConstant, initialInt));
        }
        static GlobalVariablePtr New(TypePtr type, const std::string &name, bool isConstant, ConstantArrayPtr initialArray) {
            return type->Context()->SaveValue(new GlobalVariable(type, name, isConstant, initialArray));
        }

        bool IsConstant() const { return _isConstant; }
        bool IsArray() const { return (_initialArray!= nullptr); }
        ConstantIntPtr GetInitialInt() const { return _initialInt; }
        ConstantArrayPtr GetInitialArray() const { return _initialArray; }

    private:
        GlobalVariable(TypePtr type, const std::string &name, bool isConstant, ConstantIntPtr initialInt) 
            : GlobalValue(ValueType::GlobalVariableTy, type, name), _isConstant(isConstant), _initialInt(initialInt) ,_initialArray(nullptr) {}
        GlobalVariable(TypePtr type, const std::string &name, bool isConstant, ConstantArrayPtr initialArray)
            : GlobalValue(ValueType::GlobalVariableTy, type, name), _isConstant(isConstant), _initialInt(nullptr), _initialArray(initialArray) {}

        bool _isConstant; // 若_isConstant为false，则有修饰符global；若_isConstant为true，则有修饰符constant
        ConstantIntPtr _initialInt;
        ConstantArrayPtr _initialArray;
};

#endif