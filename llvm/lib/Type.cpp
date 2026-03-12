#include "Type.hpp"
#include "LlvmContext.hpp"
#include <cstdio>

#pragma region Type

TypePtr Type::GetVoidTy(LlvmContextPtr context) { return context->GetVoidTy(); };

TypePtr Type::GetLabelTy(LlvmContextPtr context) { return context->GetLabelTy(); };

#pragma endregion

#pragma region IntegerType

IntegerTypePtr IntegerType::Get(LlvmContextPtr context, unsigned bitWidth) {
    switch (bitWidth) {
        case 1:
            return context->GetInt1Ty();
        case 32:
            return context->GetInt32Ty();
        default:
            printf("Unsupported integer bit width: %d\n", bitWidth);
    }
    return nullptr;
}

#pragma endregion

#pragma region FunctionType

bool FunctionType::Equals(TypePtr returnType, const std::vector<TypePtr> &paramTypes) const {
    if (_returnType != returnType) {
        return false;
    }
    if (_paramTypes.size() != paramTypes.size()) {
        return false;
    }
    for (int i = 0; i < _paramTypes.size(); i++) {
        if (_paramTypes[i] != paramTypes[i]) {
            return false;
        }
    }
    return true;
}

FunctionTypePtr FunctionType::Get(TypePtr returnType, const std::vector<Type *> &paramTypes) { return returnType->Context()->GetFunctionType(returnType, paramTypes); }

FunctionTypePtr FunctionType::Get(TypePtr returnType) { return returnType->Context()->GetFunctionType(returnType); }

#pragma endregion

#pragma region PointerType

PointerTypePtr PointerType::Get(TypePtr elementType) { return elementType->Context()->GetPointerType(elementType); }

#pragma endregion

#pragma region ArrayType

ArrayTypePtr ArrayType::Get(TypePtr elementType, unsigned numElements) { return elementType->Context()->GetArrayType(elementType, numElements); }

#pragma endregion
