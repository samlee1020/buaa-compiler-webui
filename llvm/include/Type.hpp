#ifndef LLVM_TYPE_HPP
#define LLVM_TYPE_HPP

#include "IrForward.hpp"
#include <vector>

/*
    Type是值类型的基类，所有值类型只能有LlvmContext获取
    注意：Type管理的是返回值类型系统，ValueType管理的是Value类型系统
    两个系统完全不同，不要混淆！
*/
class Type {
    friend class LlvmContext;

    public:
        enum TypeID {
            // 原始类型
            VoidTyID,
            LabelTyID,

            // 派生类型
            IntegerTyID,
            PointerTyID,
            FunctionTyID,
            ArrayTyID,
        };

        virtual ~Type() = default; // 基类的虚析构函数

        static TypePtr GetVoidTy(LlvmContextPtr context);
        static TypePtr GetLabelTy(LlvmContextPtr context);

        TypeID TypeId() const { return _typeId; }
        LlvmContextPtr Context() const { return _context; }

        // 判断类型
        bool IsVoidTy() const { return _typeId == VoidTyID; }
        bool IsLabelTy() const { return _typeId == LabelTyID; }
        bool IsIntegerTy() const { return _typeId == IntegerTyID; }
        bool IsPointerTy() const { return _typeId == PointerTyID; }
        bool IsFunctionTy() const { return _typeId == FunctionTyID; }
        bool IsArrayTy() const { return _typeId == ArrayTyID; }

        // 类型转化模版
        template <typename _Ty> _Ty *As() { return static_cast<_Ty *>(this); }

        // 输出类型信息
        virtual void PrintAsm(AsmWriterSmartPtr out);

    protected:
        // 阻止外部构造
        Type(LlvmContextPtr context, TypeID typeId) : _typeId(typeId), _context(context) {}
    
    private:
        TypeID _typeId;
        LlvmContextPtr _context;
};

/*
    整数类型，包含位宽信息，如i1，i32, i64
*/
class IntegerType : public Type {
    friend class LlvmContext;

    public:
        ~IntegerType() override = default;

        // 从LlvmContext获取唯一的类型指针
        static IntegerTypePtr Get(LlvmContextPtr context, unsigned bitWidth);

        unsigned BitWidth() const { return _bitWidth; }

        void PrintAsm(AsmWriterSmartPtr out) override;

    private:
        IntegerType(LlvmContextPtr context, unsigned bitWidth) : Type(context, IntegerTyID), _bitWidth(bitWidth) {}

        unsigned _bitWidth;
};

/*
    函数类型，包含返回值类型 和 参数类型列表
*/
class FunctionType : public Type {
    friend class LlvmContext;

    public:
        ~FunctionType() override = default;

        // 从LlvmContext获取唯一的类型指针
        static FunctionTypePtr Get(TypePtr returnType, const std::vector<Type *> &paramTypes);
        static FunctionTypePtr Get(TypePtr returnType);

        // 获取返回值类型 和 参数类型列表
        TypePtr ReturnType() const { return _returnType; }
        const std::vector<TypePtr> &ParamTypes() const { return _paramTypes; }

        // 判断两个函数类型是否相同
        bool Equals(TypePtr returnType, const std::vector<TypePtr> &paramTypes) const;
        bool Equals(TypePtr returnType) const { return _returnType == returnType && _paramTypes.empty(); }

        void PrintAsm(AsmWriterSmartPtr out) override;

    private:
        FunctionType(TypePtr returnType, const std::vector<TypePtr> &paramTypes) 
            : Type(returnType->Context(), FunctionTyID), _returnType(returnType), _paramTypes(paramTypes) {};
        FunctionType(TypePtr returnType)
            : Type(returnType->Context(), FunctionTyID), _returnType(returnType) {};

        TypePtr _returnType;
        std::vector<TypePtr> _paramTypes;
};

/*
    指针类型，包含指向的类型
*/
class PointerType : public Type {
    friend class LlvmContext;

    public:
        ~PointerType() override = default;

        static PointerTypePtr Get(TypePtr elementType);

        TypePtr ElementType() const { return _elementType; }

        void PrintAsm(AsmWriterSmartPtr out) override;

    private:
        PointerType(TypePtr elementType) : Type(elementType->Context(), PointerTyID), _elementType(elementType) {}

        TypePtr _elementType;
};

/*
    数组类型，包含元素类型和元素个数
*/
class ArrayType : public Type {
    friend class LlvmContext;

    public:
        ~ArrayType() override = default;

        static ArrayTypePtr Get(TypePtr elementType, unsigned numElements) ;

        TypePtr ElementType() const { return _elementType; }
        unsigned NumElements() const { return _numElements; }

        void PrintAsm(AsmWriterSmartPtr out) override;

    private:
        ArrayType(TypePtr elementType, unsigned numElements) : Type(elementType->Context(), ArrayTyID), _elementType(elementType), _numElements(numElements) {}

        TypePtr _elementType;
        unsigned _numElements;
};

#endif