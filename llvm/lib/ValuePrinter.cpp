#include "IrForward.hpp"
#include "Value.hpp"
#include "Asm.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include "LlvmContext.hpp"
#include "Instruction.hpp"
#include "Instruction_base.hpp"
#include "Use.hpp"

#include <algorithm>

#pragma region Value.hpp

void Value::PrintAsm(AsmWriterSmartPtr out) {
    // 暂未实现
}

void Value::PrintName(AsmWriterSmartPtr out) {
    out->Push(this->GetName());
}

void Value::PrintUse(AsmWriterSmartPtr out) {
    // 形如 "i32 %a"
    GetType()->PrintAsm(out);
    out->PushSpace();
    PrintName(out);
}

#pragma endregion

#pragma region Constant.hpp

void ConstantInt::PrintAsm(AsmWriterSmartPtr out) {
    // 形如"i32 520"
    GetType()->PrintAsm(out);
    out->PushSpace();
    PrintName(out);
}

void ConstantInt::PrintName(AsmWriterSmartPtr out) {
    // 形如"520"
    out->Push(std::to_string(_intValue)); 
}

void ConstantArray::PrintAsm(AsmWriterSmartPtr out) {
    // 形如"[3 x i32] [i32 1, i32 2, i32 3]""
    GetType()->PrintAsm(out);
    out->PushSpace();
    PrintName(out);
}

void ConstantArray::PrintName(AsmWriterSmartPtr out) {
    // 形如"[i32 1, i32 2, i32 3]"
    auto type = GetType()->As<ArrayType>();
    int size = type->NumElements();

    out->Push('[');
    for (auto value : _constantInts) {
        value->PrintAsm(out);
        if (value != _constantInts.back()) {
            out->Push(", ");
        }
    }
    out->Push(']');
}

void GlobalValue::PrintName(AsmWriterSmartPtr out) {
    // 形如"@main" "@var"
    out->Push('@').Push(GetName());
}

void GlobalVariable::PrintAsm(AsmWriterSmartPtr out) {
    // 形如 "@var = dso_local constant i32 123" "@array = dso_local global [3 x i32] [i32 1, i32 2, i32 3]"
    // 空行
    out->PushNewLine();

    // 变量名
    PrintName(out);

    // 等号
    out->PushNext("=");

    // global或constant
    out->PushNext("dso_local").PushSpace().Push((_isConstant) ? "constant" : "global").PushSpace();

    // 初值
    if (GetType()->As<PointerType>()->ElementType()->IsIntegerTy()) {
        GetInitialInt()->PrintAsm(out);
    } else if (GetType()->As<PointerType>()->ElementType()->IsArrayTy()) {
        if (GetInitialArray()->isZero()) {
            GetInitialArray()->GetType()->PrintAsm(out);
            out->PushNext("zeroinitializer");
        } else {
            GetInitialArray()->PrintAsm(out);
        }
    } else {
        out->Push("未知初值类型");
    }
    out->PushNewLine();
}

#pragma endregion

#pragma region Function.hpp

void Function::PrintAsm(AsmWriterSmartPtr out) {
    // 首先，利用SlotTrancker给每个值分配编号
    GetSlotTracker()->Trace(this);

    // 空行
    out->PushNewLine();

    // 一些用于调试的信息
    out->CommentBegin().Push("函数类型: ");
    GetType()->PrintAsm(out);
    out->CommentEnd();

    // 函数头
    out->Push("define dso_local ");
    GetType()->As<FunctionType>()->ReturnType()->PrintAsm(out);

    // 函数名
    out->PushSpace();
    PrintName(out);

    // 函数参数
    out->Push('(');
    for (auto it = ArgBegin(); it != ArgEnd(); ++it) {
        auto arg = *it;
        if (it != ArgBegin()) {
            out->Push(", ");
        }
        arg->PrintUse(out);
    }
    out->Push(')');

    // 函数体
    out->PushNext('{').PushNewLine();
    for (auto it = BasicBlockBegin(); it != BasicBlockEnd(); ++it) {
        (*it)->PrintAsm(out);
    }
    out->Push('}').PushNewLine();
}

void Argument::PrintAsm(AsmWriterSmartPtr out) {
    // 形如"i32 %2"
    GetType()->PrintAsm(out);
    out->PushNext('%').Push(std::to_string(Parent()->GetSlotTracker()->Slot(this)));
}

void Argument::PrintUse(AsmWriterSmartPtr out) { PrintAsm(out); }

void BasicBlock::PrintAsm(AsmWriterSmartPtr out) {
    // 不打印第一个基本块
    if (this != *Parent()->BasicBlockBegin()) {
        auto tracker = Parent()->GetSlotTracker();
        std::string slot = std::to_string(tracker->Slot(this));
        out->Push(slot).Push(':');

        // 打印注释，显示该基本块的前继基本块
        if (!this->GetUserList()->empty()) {
            int padding = 50 - static_cast<int>(slot.length()) - 1;
            out->PushSpaces(padding).Push("; preds = ");
            std::vector<int> preds;
            for (auto it = this->UserBegin(); it != UserEnd(); ++it) {
                // The user MUST be a BranchInst or JumpInst.
                auto user = dynamic_cast<HasParent<BasicBlock> *>((*it)->GetUser());
                preds.push_back(tracker->Slot(user->Parent()));
            }
            std::sort(preds.begin(), preds.end(), std::less<int>());
            for (auto it = preds.begin(); it != preds.end(); ++it) {
                out->Push('%').Push(std::to_string(*it));
                if (it + 1 != preds.end()) {
                    out->Push(", ");
                }
            }
        }
        out->PushNewLine();
    }
    // 打印指令
    for (auto it = InstructionBegin(); it != InstructionEnd(); ++it) {
        
        out->PushSpaces(4); // 缩进4个空格
        (*it)->PrintAsm(out);

        // ret和br后面不再打印指令
        if ((*it)->GetValueType() == ValueType::ReturnInstTy || 
            (*it)->GetValueType() == ValueType::BranchInstTy || 
            (*it)->GetValueType() == ValueType::JumpInstTy) {
            break;
        }
    }
}

void BasicBlock::PrintName(AsmWriterSmartPtr out) {
    out->Push('%').Push(std::to_string(Parent()->GetSlotTracker()->Slot(this)));
}

void BasicBlock::PrintUse(AsmWriterSmartPtr out) {
    GetType()->PrintAsm(out);
    out->PushSpace();
    PrintName(out);
}


#pragma endregion

#pragma region Instruction.hpp

void Instruction::PrintName(AsmWriterSmartPtr out) {
    if (GetType()->IsVoidTy()) {
        out->Push(";只有产生值的指令才能调用Instruction::PrintName").PushNewLine();
    }

    out->Push('%').Push(std::to_string(Parent()->Parent()->GetSlotTracker()->Slot(this)));
}   

void Instruction::PrintUse(AsmWriterSmartPtr out) {
    if (GetType()->IsVoidTy()) {
        out->Push(";只有产生值的指令才能调用Instruction::PrintUse").PushNewLine();
    }

    GetType()->PrintAsm(out);
    out->PushNext('%').Push(std::to_string(Parent()->Parent()->GetSlotTracker()->Slot(this)));
}

void AllocaInst::PrintAsm(AsmWriterSmartPtr out) {
    PrintName(out);
    out->PushNext("=").PushNext("alloca").PushSpace();
    AllocatedType()->PrintAsm(out);
    out->PushNewLine();
}

void StoreInst::PrintAsm(AsmWriterSmartPtr out) {
    out->Push("store").PushSpace();
    LeftOperand()->PrintUse(out);
    out->Push(", ");
    RightOperand()->PrintUse(out);
    out->PushNewLine();
}

void LoadInst::PrintAsm(AsmWriterSmartPtr out) {
    PrintName(out);
    out->PushNext('=').PushNext("load").PushSpace();
    GetType()->PrintAsm(out);
    out->Push(", ");
    Address()->PrintUse(out);
    out->PushNewLine();
}

void GepInst::PrintAsm(AsmWriterSmartPtr out) {
    PrintName(out);
    out->PushNext('=').PushNext("getelementptr").PushNext("inbounds").PushSpace();
    /*
        三种情况：
            1. addr为GlobalVariable全局数组，返回类型是数组[n x i32]*
            2. addr为AllocaInst分配的局部数组，返回类型是数组指针[n x i32]*
            3. addr为函数中数组参数退化的指针，返回类型是整数指针i32*
            4. 函数初始化时，需要alloca数组参数，类型为i32**，可能作为load的地址
        若addr为整数指针：%idx = getelementptr inbounds i32, i32* %addr, i32 %index
        若addr为数组指针：%idx = getelementptr inbounds [n x i32], [n x i32]* %addr, i32 0, i32 %index
    */
    auto pointer_type = Address()->GetType()->As<PointerType>();
    if (pointer_type->ElementType()->IsArrayTy()) {
        // 数组指针
        auto array_type = pointer_type->ElementType()->As<ArrayType>();
        array_type->PrintAsm(out);
        out->Push(",").PushSpace();
        Address()->PrintUse(out);
        out->Push(", i32 0, ");
        Index()->PrintUse(out);
    }
    else if (pointer_type->ElementType()->IsPointerTy()) {
        // 二重指针
        out->Push("i32*, ");
        Address()->PrintUse(out);
        out->Push(", ");
        Index()->PrintUse(out);
    } else {
        // 整数指针
        out->Push("i32, ");
        Address()->PrintUse(out);
        out->Push(", ");
        Index()->PrintUse(out);
    }
    out->PushNewLine();
}

void BranchInst::PrintAsm(AsmWriterSmartPtr out) {
    out->Push("br").PushSpace();
    Condition()->PrintUse(out);
    out->Push(", ");
    TrueBlock()->PrintUse(out);
    out->Push(", ");
    FalseBlock()->PrintUse(out);
    out->PushNewLine();
}

void JumpInst::PrintAsm(AsmWriterSmartPtr out) {
    out->Push("br").PushSpace();
    Target()->PrintUse(out);
    out->PushNewLine();
}

void ReturnInst::PrintAsm(AsmWriterSmartPtr out) {
    out->Push("ret");
    ValuePtr ret = ReturnValue();
    if (!ret || ret->GetType()->IsVoidTy()) {
        out->PushNext("void");
    } else {
        out->PushSpace();
        ret->PrintUse(out);
    }
    out->PushNewLine();
}

void CallInst::PrintAsm(AsmWriterSmartPtr out) {
    if (!GetType()->IsVoidTy()) {
        PrintName(out);
        out->Push(" = ");
    }

    out->Push("call").PushSpace();
    // 返回值类型
    GetFunction()->ReturnType()->PrintAsm(out);
    out->PushSpace();
    // 函数名
    GetFunction()->PrintName(out);
    // 参数
    out->Push('(');
    for (auto it = UseBegin(); it != UseEnd(); ++it) {
        if (it != UseBegin()) {
            out->Push(", ");
        }
        (*it)->GetValue()->PrintUse(out);
    }
    out->Push(')').PushNewLine();
}

#pragma endregion

#pragma region InstructionType.hpp

void UnaryOperator::PrintAsm(AsmWriterSmartPtr out) {
    if (GetOpType() == UnaryOpType::Not) {
        // 逻辑非的运算应该被转化成x==0的比较运算，此处不该有此情况
        return;
    }

    std::string op;
    switch (GetOpType()) {
        case UnaryOpType::Pos:
            op = "add nsw";
            break;
        case UnaryOpType::Neg:
            op = "sub nsw";
            break;
        default:
            op = "未知操作";
    }

    PrintName(out);
    out->PushNext("=").PushNext(op).PushSpace();

    GetType()->PrintAsm(out);
    out->PushNext("0").Push(',').PushSpace();
    Operand()->PrintName(out);

    out->PushNewLine();
}

void ZextInst::PrintAsm(AsmWriterSmartPtr out) {
    // 形如"i32 %a = zext i1 %b to i32"
    PrintName(out);
    out->PushNext("=").PushNext("zext").PushSpace();
    Operand()->PrintUse(out);
    out->PushNext("to").PushSpace();
    GetType()->PrintAsm(out);
    out->PushNewLine();
}

void BinaryOperator::PrintAsm(AsmWriterSmartPtr out) {
    std::string op;
    switch (OpType()) {
    case BinaryOpType::Add:
        op = "add nsw";
        break;
    case BinaryOpType::Sub:
        op = "sub nsw";
        break;
    case BinaryOpType::Mul:
        op = "mul nsw";
        break;
    case BinaryOpType::Div:
        op = "sdiv";
        break;
    case BinaryOpType::Mod:
        op = "srem";
        break;
    }

    PrintName(out);
    out->PushNext("=").PushNext(op).PushSpace();

    GetType()->PrintAsm(out);
    out->PushSpace();

    LeftOperand()->PrintName(out);
    out->Push(", ");
    RightOperand()->PrintName(out);

    out->PushNewLine();
}

void CompareInstruction::PrintAsm(AsmWriterSmartPtr out) {
    std::string op;
    switch (OpType()) {
    case CompareOpType::Equal:
        op = "eq";
        break;
    case CompareOpType::NotEqual:
        op = "ne";
        break;
    case CompareOpType::GreaterThan:
        op = "sgt";
        break;
    case CompareOpType::GreaterThanOrEqual:
        op = "sge";
        break;
    case CompareOpType::LessThan:
        op = "slt";
        break;
    case CompareOpType::LessThanOrEqual:
        op = "sle";
        break;
    }

    PrintName(out);
    out->PushNext("=");
    out->PushNext("icmp");
    out->PushNext(op).PushSpace();
    LeftOperand()->GetType()->PrintAsm(out);
    out->PushSpace();
    LeftOperand()->PrintName(out);
    out->Push(',').PushSpace();
    RightOperand()->PrintName(out);
    out->PushNewLine();
}

#pragma endregion