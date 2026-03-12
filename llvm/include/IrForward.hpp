#ifndef IrFORWARD_HPP
#define IrFORWARD_HPP

#include <memory>
#include <vector>


/*
    该文件负责：
        1. 前向声明
        2. 提供指针别名
        3. 提供获取父节点信息的基类模版
*/

class Module;
using ModuleSmartPtr = std::shared_ptr<Module>;

class LlvmContext;
using LlvmContextPtr = LlvmContext *;

class AsmWriter;
using AsmWriterSmartPtr = std::shared_ptr<AsmWriter>;

#pragma region TypeForward

    class Type;
    using TypePtr = Type *;

    class IntegerType; // Type -> IntegerType 整数类型
    using IntegerTypePtr = IntegerType *;

    class PointerType; // Type -> PointerType 指针类型
    using PointerTypePtr = PointerType *;

    class ArrayType; // Type -> ArrayType 数组类型
    using ArrayTypePtr = ArrayType *;

    class FunctionType; // Type -> FunctionType 函数类型
    using FunctionTypePtr = FunctionType *;


#pragma endregion

#pragma region ValueForward

    class Value;
    using ValuePtr = Value *;

    class User;
    using UserPtr = User *;

    class Constant; // Value -> Constant 常量
    using ConstantPtr = Constant *;

    class GlobalValue; // Constant -> GlobalValue 全局value（包含全局变量和函数）
    using GlobalValuePtr = GlobalValue *;

    class ConstantInt; // Constant -> ConstantInt 整数常量
    using ConstantIntPtr = ConstantInt *;

    class ConstantArray; // Constant -> ConstantArray 数组常量
    using ConstantArrayPtr = ConstantArray *;

    class Function; // GlovalValue -> Function 函数（自定义函数和main函数）
    using FunctionPtr = Function *;

    class GlobalVariable; // GlobalValue -> GlobalVariable 全局变量（main函数之外的变量）
    using GlobalVariablePtr = GlobalVariable *;

    class Argument; // Function 的子节点，表示函数的输入参数
    using ArgumentPtr = Argument *;

    class BasicBlock; // Function 的子节点，表示基本块  
    using BasicBlockPtr = BasicBlock *;

#pragma endregion

#pragma region UserForward

    class Use;
    using UsePtr = Use *;
    using UseList = std::vector<UsePtr>;
    using UseListPtr = UseList *;

#pragma endregion

#pragma region InstructionForward

    class Instruction;
    using InstructionPtr = Instruction *;

    class UnaryInstruction;
    using UnaryInstructionPtr = UnaryInstruction *;

    class UnaryOperator;
    using UnaryOperatorPtr = UnaryOperator *;

    class BinaryOperator;
    using BinaryOperatorPtr = BinaryOperator *;

    class CompareInstruction;
    using CompareInstructionPtr = CompareInstruction *;

    class AllocaInst;
    using AllocaInstPtr = AllocaInst *;

    class LoadInst;
    using LoadInstPtr = LoadInst *;

    class StoreInst;
    using StoreInstPtr = StoreInst *;

    class CallInst;
    using CallInstPtr = CallInst *;

    class BranchInst;
    using BranchInstPtr = BranchInst *;

    class JumpInst;
    using JumpInstPtr = JumpInst *;

    class GepInst;
    using GepInstPtr = GepInst *;

    class ReturnInst;
    using ReturnInstPtr = ReturnInst *;

    class ZextInst;
    using ZextInstPtr = ZextInst *;

#pragma endregion


#pragma region HasParent
/*
    该类模版用于获取父节点信息
*/
template <typename _Ty> class HasParent {
    public:
        using _TyPtr = _Ty *;

        virtual ~HasParent() = default;

        void SetParent(_TyPtr parent) { _parent = parent; }
        void RemoveParent() { _parent = nullptr; }
        _TyPtr Parent() const { return _parent; }

    protected:
        HasParent(_TyPtr parent = nullptr) : _parent(parent) {}

    private:
        _TyPtr _parent;
};
#pragma endregion

#endif