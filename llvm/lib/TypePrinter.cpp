#include "Type.hpp"

/*
    负责实现Type的打印
*/
#include "Asm.hpp"
#include <string>

void Type::PrintAsm(AsmWriterSmartPtr out) {
    switch (_typeId) {
    case VoidTyID:
        out->Push("void");
        break;
    case LabelTyID:
        out->Push("label");
        break;
    default:
        out->Push("unknown");
        break;
    }
}

// 整数
void IntegerType::PrintAsm(AsmWriterSmartPtr out) {
    out->Push('i').Push(std::to_string(_bitWidth));
}

void ArrayType::PrintAsm(AsmWriterSmartPtr out) {
    std::string type = (_elementType->IsIntegerTy()) ? "i32" : "暂不支持";
    out->Push('[').Push("%d", _numElements).PushSpace().Push('x').PushSpace().Push(type).Push(']');
}

// 函数返回类型 ( arg1, arg2, ... )
void FunctionType::PrintAsm(AsmWriterSmartPtr out) {
    ReturnType()->PrintAsm(out);
    out->PushNext('(');
    bool first = true;
    for (auto type : _paramTypes) {
        if (!first) {
            out->Push(", ");
        } else {
            first = false;
        }
        type->PrintAsm(out);
    }
    out->Push(')');
}

// 指针
void PointerType::PrintAsm(AsmWriterSmartPtr out) {
    _elementType->PrintAsm(out);
    out->Push('*');
}

