#ifndef LLVM_CONTEXT_HPP
#define LLVM_CONTEXT_HPP

#include "IrForward.hpp"
#include "Type.hpp"
#include <vector>

/*
    模块上下文类。
    每一个Module拥有一个上下文对象。
    它有如下作用：
        1. 提供统一的值类型type
        2. 管理value的生命周期
        3. 记录全局的uses关系
*/
class LlvmContext {
    // 只有Module类可以创建LlvmContext对象
    friend class Module;

    public:
        ~LlvmContext();

        // 阻止拷贝
        LlvmContext(const LlvmContext&) = delete;
        LlvmContext& operator=(const LlvmContext&) = delete;

        // 提供统一的值类型
        TypePtr GetVoidTy() { return &_voidTy; }
        TypePtr GetLabelTy() { return &_labelTy; }
        IntegerTypePtr GetInt1Ty() { return &_int1Ty; }
        IntegerTypePtr GetInt32Ty() { return &_int32Ty; }

        // 提供复杂类型
        FunctionTypePtr GetFunctionType(TypePtr returnType, const std::vector<TypePtr> &paramTypes);
        FunctionTypePtr GetFunctionType(TypePtr returnType);
        PointerTypePtr GetPointerType(TypePtr elementType);
        ArrayTypePtr GetArrayType(TypePtr elementType, unsigned numElements);

        // 保存管理所有的value指针，当LlvmContext销毁时，自动释放所有value
        template <typename _Ty> _Ty *SaveValue(_Ty *value) {
            _values.push_back(value);
            return value->template As<_Ty>();
        }

        // 记录全局的uses关系
        UsePtr SaveUse(UsePtr use) {
            _uses.push_back(use);
            return use;
        }
        
    private:
        LlvmContext() : _voidTy(this, Type::VoidTyID), _labelTy(this, Type::LabelTyID), _int1Ty(this, 1), _int32Ty(this, 32) {}

        // 预设的类型，可以直接初始化
        Type _voidTy;
        Type _labelTy;
        IntegerType _int1Ty;
        IntegerType _int32Ty;

        // 派生类型，需要在遇到时创建
        std::vector<FunctionTypePtr> _functionTypes;
        std::vector<PointerTypePtr> _pointerTypes;
        std::vector<ArrayTypePtr> _arrayTypes;

        // 全局valuse和use列表
        std::vector<ValuePtr> _values;
        std::vector<UsePtr> _uses;
};

#endif