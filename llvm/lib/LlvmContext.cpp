#include "LlvmContext.hpp"
#include "Type.hpp"
#include "Use.hpp"

LlvmContext::~LlvmContext() {
    for (auto ty : _functionTypes) {
        delete ty;
    }

    for (auto ty : _pointerTypes) {
        delete ty;
    }

    for (auto ty : _arrayTypes) {
        delete ty;
    }

    for (auto v : _values) {    
        delete v;
    }

    for (auto u : _uses) {
        delete u;
    }
}

FunctionTypePtr LlvmContext::GetFunctionType(TypePtr returnType, const std::vector<TypePtr> &paramTypes) {
    // 已经存在则返回
    for (auto ty : _functionTypes) {
        if (ty->Equals(returnType, paramTypes)) {
            return ty;
        }
    }

    // 否则创建
    auto ty = new FunctionType(returnType, paramTypes);
    _functionTypes.push_back(ty);
    return ty;
}

FunctionTypePtr LlvmContext::GetFunctionType(TypePtr returnType) {
    // 已经存在则返回
    for (auto ty : _functionTypes) {
        if (ty->Equals(returnType)) {
            return ty;
        }
    }

    // 否则创建
    auto ty = new FunctionType(returnType);
    _functionTypes.push_back(ty);
    return ty;
}

PointerTypePtr LlvmContext::GetPointerType(TypePtr elementType) {
    // 已经存在则返回
    for (auto type : _pointerTypes) {
        if (type->ElementType() == elementType) {
            return type;
        }
    }

    // 否则创建
    auto pointerType = new PointerType(elementType);
    _pointerTypes.push_back(pointerType);
    return pointerType;
}

ArrayTypePtr LlvmContext::GetArrayType(TypePtr elementType, unsigned numElements) {
    // 已经存在则返回
    for (auto type : _arrayTypes) {
        if (type->ElementType() == elementType && type->NumElements() == numElements) {
            return type;
        }
    }

    // 否则创建
    auto arrayType = new ArrayType(elementType, numElements);
    _arrayTypes.push_back(arrayType);
    return arrayType;
}