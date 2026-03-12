#include "translator.hpp"
#include "Module.hpp"
#include "IrForward.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include "Use.hpp"
#include <iostream>
#include <set>
#include <vector>
#include <string>
#include "GlobalVar.hpp"

void Translator::translate(ModuleSmartPtr &modulePtr) {
    // 翻译GlobalVariable
    for (auto it = modulePtr->GlobalVariableBegin(); it!= modulePtr->GlobalVariableEnd(); it++) {
        translate(*it);
    }

    // 翻译Function
    for (auto it = modulePtr->FunctionBegin(); it!= modulePtr->FunctionEnd(); it++) {
        if ((*it)->GetName() == "getint" || (*it)->GetName() == "putint" || (*it)->GetName() == "putch") {
            continue;
        }
        translate(*it);
    }

    // 翻译main函数
    translate(modulePtr->MainFunction());
}

void Translator::print(std::ostream &_out) {
    manager->PrintMips(_out);
}

void Translator::translate(GlobalVariablePtr globalVariablePtr) {
    if (!globalVariablePtr->IsArray()) {
        // 翻译普通全局变量
        auto name = globalVariablePtr->GetName();
        auto wordDataPtr = new WordData(name, globalVariablePtr->GetInitialInt()->GetIntValue());
        manager->addData(wordDataPtr);
    } else {
        // 翻译数组全局变量
        auto name = globalVariablePtr->GetName();
        std::vector<int> int_values;
        auto array_value = globalVariablePtr->GetInitialArray();
        auto constantInts = array_value->GetConstantInts();
        if (constantInts.empty()) {
            int size = globalVariablePtr->GetType()->As<PointerType>()->ElementType()->As<ArrayType>()->NumElements();
            for (int i = 0; i < size; i++) {
                int_values.push_back(0);
            }
        }
        for (auto constantInt : constantInts) {
            int_values.push_back(constantInt->GetIntValue());
        }
        auto arrayDataPtr = new ArrayData(name, int_values);
        manager->addData(arrayDataPtr);
    }
}

void Translator::translate(FunctionPtr functionPtr) {
    // 重置帧
    manager->resetFrame(functionPtr->GetName());
    // 翻译函数
    for (auto it = functionPtr->BasicBlockBegin(); it!= functionPtr->BasicBlockEnd(); it++) {
        translate(*it);
    }
}

void Translator::translate(BasicBlockPtr basicBlockPtr) {
    // 添加基本块标签f
    auto name = manager->getLabelName(basicBlockPtr);
    manager->addCode(new MipsLabel(name));
    // 翻译基本块
    int str_count = 0;
    for (auto it = basicBlockPtr->InstructionBegin(); it != basicBlockPtr->InstructionEnd(); it++) {
        if (OPTIMIZE) {
            // 优化：尝试合并连续的putch
            if ((*it)->Is<CallInst>() && (*it)->As<CallInst>()->GetFunction()->GetName() == "putch") { 
                std::string str;
                int ch = (*it)->As<CallInst>()->OperandAt(0)->As<ConstantInt>()->GetIntValue();
                str += (char) ch;
                while (true) {
                    auto next_it = it; 
                    next_it++;
                    if (next_it == basicBlockPtr->InstructionEnd() ||
                        !(*next_it)->Is<CallInst>() ||
                        (*next_it)->As<CallInst>()->GetFunction()->GetName() != "putch") {
                        break;
                    } 
                    int ch = (*next_it)->As<CallInst>()->OperandAt(0)->As<ConstantInt>()->GetIntValue();
                    str += (char) ch;
                    it = next_it;
                }
                // 创建字符串常量
                auto str_name = name + "_str_" + std::to_string(str_count++);
                auto asciiz_data_ptr = new AsciizData(str_name, str);
                manager->addData(asciiz_data_ptr);
                // 把字符串地址la到寄存器
                manager->addCode(new ICode(MipsCodeType::LA, manager->a0, str_name));
                // 系统调用
                manager->addCode(new ICode(MipsCodeType::Addiu, manager->v0, manager->zero, 4));
                manager->addCode(new RCode(MipsCodeType::Syscall));
                continue;
            } 
        }

        translate(*it);
    }
    // 跨基本块的非offset寄存器push到栈上 （这里是tolangc的写法，不知道是不是正确的）
    // TODO：确认是否正确
    manager->pushAll();
}

void Translator::translate(InstructionPtr instructionPtr) {
    if (instructionPtr->isNotToMips()) {
        // 该指令不需要翻译
    } else if (instructionPtr->Is<AllocaInst>()) {
        translate(instructionPtr->As<AllocaInst>());
    } else if (instructionPtr->Is<LoadInst>()) {
        if (OPTIMIZE && instructionPtr->GetUserList()->empty()) {
            // 该指令没有被使用，不需要翻译
            return;
        }
        translate(instructionPtr->As<LoadInst>());
    } else if (instructionPtr->Is<GepInst>()) {
        if (OPTIMIZE && instructionPtr->GetUserList()->empty()) {
            // 该指令没有被使用，不需要翻译
            return;
        }
        translate(instructionPtr->As<GepInst>());
    } else if (instructionPtr->Is<StoreInst>()) {
        translate(instructionPtr->As<StoreInst>());
    } else if (instructionPtr->Is<BranchInst>()) {
        translate(instructionPtr->As<BranchInst>());
    } else if (instructionPtr->Is<JumpInst>()) {
        translate(instructionPtr->As<JumpInst>());
    } else if (instructionPtr->Is<ReturnInst>()) {
        translate(instructionPtr->As<ReturnInst>());
    } else if (instructionPtr->Is<CallInst>()) {
        translate(instructionPtr->As<CallInst>());
    } else if (instructionPtr->Is<UnaryOperator>()) {
        if (OPTIMIZE && instructionPtr->GetUserList()->empty()) {
            // 该指令没有被使用，不需要翻译
            return;
        }
        translate(instructionPtr->As<UnaryOperator>());
    } else if (instructionPtr->Is<ZextInst>()) {
        if (OPTIMIZE && instructionPtr->GetUserList()->empty()) {
            // 该指令没有被使用，不需要翻译
            return;
        }
        translate(instructionPtr->As<ZextInst>());
    } else if (instructionPtr->Is<BinaryOperator>()) {
        if (OPTIMIZE && instructionPtr->GetUserList()->empty()) {
            // 该指令没有被使用，不需要翻译
            return;
        }
        translate(instructionPtr->As<BinaryOperator>());
    } else if (instructionPtr->Is<CompareInstruction>()) {
        if (OPTIMIZE && instructionPtr->GetUserList()->empty()) {
            // 该指令没有被使用，不需要翻译
            return;
        }
        translate(instructionPtr->As<CompareInstruction>());
    } 
    // 释放使用完的value对应的寄存器
    manager->tryRelease(instructionPtr);
}

void Translator::translate(AllocaInstPtr allocaInstPtr) {
    if (allocaInstPtr->AllocatedType()->IsIntegerTy() || allocaInstPtr->AllocatedType()->IsPointerTy()) {
        // 整数和指针类型，分配1 x 4字节内存
        manager->allocMem(allocaInstPtr, 1);
    } else if (allocaInstPtr->AllocatedType()->IsArrayTy()) {
        // 数组类型，分配size x 4字节内存
        auto size = allocaInstPtr->AllocatedType()->As<ArrayType>()->NumElements();
        manager->allocMem(allocaInstPtr, size);
    } 
}

void Translator::translate(LoadInstPtr loadInstPtr) {
    if (OPTIMIZE && loadInstPtr->isLoadFromS7()) {
        // 该指令从s7寄存器读取
        auto resultReg = manager->allocReg(loadInstPtr);
        manager->addCode(new ICode(MipsCodeType::Addiu, resultReg, manager->s7, 0));
        return;
    }

    auto operand = loadInstPtr->Operand();
    if (operand->Is<GlobalValue>()) {
        // 全局变量
        auto resultReg = manager->allocReg(loadInstPtr);
        manager->addCode(new ICode(MipsCodeType::LW, resultReg, loadInstPtr->Operand()->GetName()));
        return;
    }
    // 分配一个寄存器存储结果
    auto resultReg = manager->allocReg(loadInstPtr);
    auto index_reg = manager->getReg(operand, MipsManager::GetRegPurpose::FOR_OFFSET_TO_SP);
    if (index_reg->GetType() == MipsRegType::OffsetTy) {
        manager->addCode(new ICode(MipsCodeType::LW, resultReg, manager->sp, index_reg->GetIndex()));
    } else if (index_reg->GetType() == MipsRegType::TmpRegTy) {
        manager->addCode(new ICode(MipsCodeType::LW, resultReg, index_reg, 0));
    }
}

void Translator::translate(GepInstPtr gepInstPtr) {
    // 获取基地址和偏移对应的寄存器
    auto addr_reg = manager->getReg(gepInstPtr->Address(), MipsManager::GetRegPurpose::FOR_ADDRESS);
    auto index_reg = manager->loadConst(gepInstPtr->Index(), MipsRegType::TmpRegTy);
    // 分配一个寄存器存储结果
    auto resultReg = manager->allocReg(gepInstPtr);
    if (gepInstPtr->Index()->GetType()->IsIntegerTy()) {
        // 非0偏移，需要计算偏移地址
        if (gepInstPtr->Index()->Is<ConstantInt>() && gepInstPtr->Index()->As<ConstantInt>()->GetIntValue() == 0) {
            ;
        } else {
            manager->addCode(new RCode(MipsCodeType::Sll, index_reg, index_reg, 2));
        }
    } 
    manager->addCode(new RCode(MipsCodeType::Addu, resultReg, addr_reg, index_reg));
}

void Translator::translate(StoreInstPtr storeInstPtr) {
    if (OPTIMIZE && storeInstPtr->isStoreToS7()) {
        // 该指令存入s7寄存器
        auto value_reg = manager->loadConst(storeInstPtr->LeftOperand(), MipsRegType::TmpRegTy);
        manager->addCode(new ICode(MipsCodeType::Addiu, manager->s7, value_reg, 0));
        return;
    }

    // 获取被存储的值的寄存器
    auto store_value_reg = manager->loadConst(storeInstPtr->LeftOperand(), MipsRegType::TmpRegTy);
    // 获取存储地址的寄存器
    auto store_addr_reg = manager->getReg(storeInstPtr->RightOperand(), MipsManager::GetRegPurpose::FOR_OFFSET_TO_SP);
    if (store_addr_reg->GetType() == MipsRegType::OffsetTy) {
        manager->addCode(new ICode(MipsCodeType::SW, store_value_reg, manager->sp, store_addr_reg->GetIndex()));
    } else if (store_addr_reg->GetType() == MipsRegType::TmpRegTy) {
        manager->addCode(new ICode(MipsCodeType::SW, store_value_reg, store_addr_reg, 0));
    }
}

void Translator::translate(BranchInstPtr branchInstPtr) {
    // 获取条件值和标签
    auto cond_reg = manager->loadConst(branchInstPtr->Condition(), MipsRegType::TmpRegTy);
    auto ture_label_name = manager->getLabelName(branchInstPtr->TrueBlock());
    auto false_label_name = manager->getLabelName(branchInstPtr->FalseBlock());
    // 跳转到ture
    manager->addCode(new ICode(MipsCodeType::Bnez, cond_reg, ture_label_name));
    manager->addCode(new RCode(MipsCodeType::Nop)); // 延迟槽
    // 跳转到flase
    manager->addCode(new JCode(MipsCodeType::J, false_label_name));
    manager->addCode(new RCode(MipsCodeType::Nop)); // 延迟槽
}

void Translator::translate(JumpInstPtr jumpInstPtr) {
    auto label_name = manager->getLabelName(jumpInstPtr->Target());
    manager->addCode(new JCode(MipsCodeType::J, label_name));
    manager->addCode(new RCode(MipsCodeType::Nop)); // 延迟槽
}

void Translator::translate(ReturnInstPtr returnInstPtr) {
    if (returnInstPtr->ReturnValue()) {
        if (manager->functionName == "main") {
            // main函数返回，调用syscall 10
            manager->addCode(new ICode(Addiu, manager->v0, manager->zero, 10));
            manager->addCode(new RCode(MipsCodeType::Syscall));
            return;
        }
        if (returnInstPtr->ReturnValue()->GetType()->IsIntegerTy()) {
            // 整数类型，返回值存入v0
            auto reg = manager->loadConst(returnInstPtr->ReturnValue(), TmpRegTy);
            manager->addCode(new RCode(Addu, manager->v0, reg, manager->zero));
        }
        manager->addCode(new RCode(MipsCodeType::Jr, manager->ra));
        manager->addCode(new RCode(MipsCodeType::Nop));
    } else {
        // 没有返回值
        manager->addCode(new RCode(MipsCodeType::Jr, manager->ra));
        manager->addCode(new RCode(MipsCodeType::Nop));
    }
}

void Translator::translate(CallInstPtr callInstPtr) {
    if (callInstPtr->GetFunction()->GetName() == "getint") {
        // 调用syscall 5
        manager->addCode(new ICode(Addiu, manager->v0, manager->zero, 5));
        manager->addCode(new RCode(MipsCodeType::Syscall));
        auto resultReg = manager->allocReg(callInstPtr);
        // 返回值由v0存入resultReg
        manager->addCode(new RCode(Addu, resultReg, manager->v0, manager->zero));
        return;
    } else if (callInstPtr->GetFunction()->GetName() == "putint") {
        // 把要输出的整数存到a0
        auto operand_reg = manager->loadConst(callInstPtr->OperandAt(0), TmpRegTy);
        manager->addCode(new RCode(Addu, manager->a0, operand_reg, manager->zero));
        // 调用syscall 1
        manager->addCode(new ICode(Addiu, manager->v0, manager->zero, 1));
        manager->addCode(new RCode(MipsCodeType::Syscall));
        return;
    } else if (callInstPtr->GetFunction()->GetName() == "putch") {
        // 把要输出的字符存到a0
        int ch = callInstPtr->OperandAt(0)->As<ConstantInt>()->GetIntValue();
        manager->addCode(new ICode(Addiu, manager->a0, manager->zero, ch));
        // 调用syscall 11
        manager->addCode(new ICode(Addiu, manager->v0, manager->zero, 11));
        manager->addCode(new RCode(MipsCodeType::Syscall));
        return;
    }
    // 把所有不在栈上的非参数活跃变量存到栈上
    std::set<ValuePtr> pushSet;
    for (auto occ : manager->occupation) {
        if (occ.second->GetType() != OffsetTy) {
            pushSet.insert(occ.first);
        }
    }
    for (UsePtr use : *(callInstPtr->GetUseList())) {
        pushSet.erase(use->GetValue());
    }

    // 计算参数传递的栈位
    int pos = manager->currentOffset - 4 * pushSet.size() - 4;  // 预留ra 和 保存变量的空间
    for (auto valuePtr : pushSet) {
        if (valuePtr->Is<GlobalValue>() || valuePtr->Is<AllocaInst>()) {
            // 全局变量和alloca，不用push到栈上
            pos += 4;
        }
    }

    // 参数传递
    auto& list = *callInstPtr->GetUseList();
    for (int i = list.size() - 1; i >= 0; i--) {
        /*  从右往左传递参数，理由：
            llvm处理后，是先准备好所有的参数value操作数，然后再调用CallInst。
            如果操作数很多，那么最前面（aka最左边）的操作数会被溢出到栈上。
            也就是说，参数多的情况下，list里的value通过loadConst的到的寄存器会呈现以下布局：
            最右边的几个会在tmpReg中，剩下的在OffsetReg中（因为左边的被溢出到栈上了）
            所以从右边开始传参数可以避免寄存器循环溢出
        */
        int position = pos - 4 * i; // 计算参数在栈上的位置，0号参数在栈顶，i号参数在pos - 4i
        auto reg = manager->loadConst(list[i]->GetValue(), TmpRegTy);
        manager->addCode(new ICode(MipsCodeType::SW, reg, manager->sp, position));
        manager->release(list[i]->GetValue()); // 参数传递完就可以release了
    }
    // for (UsePtr use : *(callInstPtr->GetUseList())) {
    //     auto reg = manager->loadConst( use->GetValue(), TmpRegTy);
    //     manager->addCode(new ICode(MipsCodeType::SW, reg, manager->sp, pos));
    //     pos -= 4;
    // }

    // 保存变量
    for (auto valuePtr : pushSet) {
        manager->push(valuePtr);
    }

    // 保存ra
    manager->addCode(new ICode(SW, manager->ra, manager->sp, manager->currentOffset));
    manager->currentOffset -= 4;

    // 调整sp位置
    manager->addCode(new ICode(Addiu, manager->sp, manager->sp, manager->currentOffset));

    // 跳转函数
    manager->addCode(new JCode(Jal, callInstPtr->GetFunction()->GetName()));
    manager->addCode(new RCode(Nop)); // 延迟槽

    // 恢复sp和ra
    manager->addCode(new ICode(Subiu, manager->sp, manager->sp, manager->currentOffset));
    manager->currentOffset += 4;
    manager->addCode(new ICode(LW, manager->ra, manager->sp, manager->currentOffset));

    if (!callInstPtr->GetType()->IsVoidTy()) {
        auto resultReg = manager->allocReg(callInstPtr);
        // 返回值存入v0
        manager->addCode(new RCode(Addu, resultReg, manager->v0, manager->zero));
    }
}

void Translator::translate(UnaryOperatorPtr unaryOperatorPtr) {
    auto operand_reg = manager->loadConst(unaryOperatorPtr->Operand(), TmpRegTy);
    if (unaryOperatorPtr->GetOpType() == UnaryOpType::Pos) {
        auto result = manager->allocReg(unaryOperatorPtr);
        manager->addCode(new RCode(MipsCodeType::Addu, result, manager->zero, operand_reg));
    } else if (unaryOperatorPtr->GetOpType() == UnaryOpType::Neg) {
        auto result = manager->allocReg(unaryOperatorPtr);
        manager->addCode(new RCode(MipsCodeType::Subu, result, manager->zero, operand_reg));
    } else if (unaryOperatorPtr->GetOpType() == UnaryOpType::Not) {
        auto result = manager->allocReg(unaryOperatorPtr);
        manager->addCode(new RCode(Seq, result, manager->zero, operand_reg));
    }
}

void Translator::translate(ZextInstPtr zextInstPtr) {
    auto result = manager->allocReg(zextInstPtr);
    auto operand_reg = manager->loadConst(zextInstPtr->Operand(), TmpRegTy);
    manager->addCode(new RCode(MipsCodeType::Addu, result, operand_reg, manager->zero));
}

void Translator::translate(BinaryOperatorPtr binaryOperatorPtr) {
    MipsCodeType op;
    switch (binaryOperatorPtr->OpType()) {
    case BinaryOpType::Add:
        op = MipsCodeType::Addu;
        break;
    case BinaryOpType::Sub:
        op = MipsCodeType::Subu;
        break;
    case BinaryOpType::Mul:
        op = MipsCodeType::Mul;
        break;
    case BinaryOpType::Div:
        op = MipsCodeType::Div;
        break;
    case BinaryOpType::Mod:
        op = MipsCodeType::Rem;
        break;
    }

    if (OPTIMIZE) {
        if (op == MipsCodeType::Addu || op == MipsCodeType::Subu) {
            // 优化：立即数加减运算
            auto left_value = binaryOperatorPtr->LeftOperand();
            auto right_value = binaryOperatorPtr->RightOperand();
            if (left_value->Is<ConstantInt>() && op == MipsCodeType::Addu) {
                op = MipsCodeType::Addiu;
                auto result = manager->allocReg(binaryOperatorPtr);
                int left_value_int = left_value->As<ConstantInt>()->GetIntValue();
                auto right_reg = manager->loadConst(right_value, TmpRegTy);
                manager->addCode(new ICode(op, result, right_reg, left_value_int));
                return;
            } else if (right_value->Is<ConstantInt>()) {
                op = (op == MipsCodeType::Addu) ? MipsCodeType::Addiu : MipsCodeType::Subiu;
                auto result = manager->allocReg(binaryOperatorPtr);
                int right_value_int = right_value->As<ConstantInt>()->GetIntValue();
                auto left_reg = manager->loadConst(left_value, TmpRegTy);
                manager->addCode(new ICode(op, result, left_reg, right_value_int));
                return;
            }
        } else if (op == MipsCodeType::Mul) {
            // 优化：乘法强度削弱
            auto left_value = binaryOperatorPtr->LeftOperand();
            auto right_value = binaryOperatorPtr->RightOperand();
            if (left_value->Is<ConstantInt>() || right_value->Is<ConstantInt>()) {
                int value_int = (left_value->Is<ConstantInt>()) ? 
                                left_value->As<ConstantInt>()->GetIntValue() : 
                                right_value->As<ConstantInt>()->GetIntValue();
                if (value_int % 2 == 0 && value_int != 0) {
                    auto result = manager->allocReg(binaryOperatorPtr);
                    auto value_reg = (left_value->Is<ConstantInt>()) ? 
                                    manager->loadConst(right_value, TmpRegTy) : 
                                    manager->loadConst(left_value, TmpRegTy);
                    int exp = 0;
                    int abs_value_int = (value_int > 0) ? value_int : -value_int;
                    while (abs_value_int != 1) {
                        abs_value_int /= 2;
                        exp++;
                    }
                    manager->addCode(new RCode(MipsCodeType::Sll, result, value_reg, exp));
                    if (value_int < 0) {
                        manager->addCode(new RCode(MipsCodeType::Subu, result, manager->zero, result));
                    }
                    return;
                }
            } 
        }
    } 

    auto left_reg = manager->loadConst(binaryOperatorPtr->LeftOperand(), TmpRegTy);
    auto right_reg = manager->loadConst(binaryOperatorPtr->RightOperand(), TmpRegTy);
    auto result = manager->allocReg(binaryOperatorPtr);
    manager->addCode(new RCode(op, result, left_reg, right_reg));
}

void Translator::translate(CompareInstructionPtr compareInstructionPtr) {
    auto left_reg = manager->loadConst(compareInstructionPtr->LeftOperand(), TmpRegTy);
    auto right_reg = manager->loadConst(compareInstructionPtr->RightOperand(), TmpRegTy);
    auto result = manager->allocReg(compareInstructionPtr);
    MipsCodeType op;
    switch (compareInstructionPtr->OpType()) {
    case CompareOpType::Equal:
        op = MipsCodeType::Seq;
        break;
    case CompareOpType::NotEqual:
        op = MipsCodeType::Sne;
        break;
    case CompareOpType::GreaterThan:
        op = MipsCodeType::Sgt;
        break;
    case CompareOpType::GreaterThanOrEqual:
        op = MipsCodeType::Sge;
        break;
    case CompareOpType::LessThan:
        op = MipsCodeType::Slt;
        break;
    case CompareOpType::LessThanOrEqual:
        op = MipsCodeType::Sle;
        break;
    }
    manager->addCode(new RCode(op, result, left_reg, right_reg));
}
    