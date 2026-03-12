#include "MipsManager.hpp"
#include "Value.hpp"
#include "Instruction.hpp"
#include "Constant.hpp"
#include "Use.hpp"
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <string>
#include "GlobalVar.hpp"



MipsManager::MipsManager() {
    if (OPTIMIZE) {
        TMPCOUNT = 10 + 8 - 1; // 开启优化后，可使用的寄存器从t0-t7变成t0-t9，s0-s6，全部作为临时寄存器。s7作为短路寄存器，存放条件判断结果。
    } else {
        TMPCOUNT = 8; // 未开启优化，可使用的寄存器从t0-t7
    }

    // 临时寄存器
    for (int i = 0; i < TMPCOUNT; i++) { 
        tmpRegPool.insert(std::pair<int, TmpRegPtr>(i, new TmpReg(i)));
    }
    
    // 零寄存器
    zero = new ZeroReg();

    // 栈指针寄存器
    sp = new StkPtrReg();

    // 返回地址寄存器
    ra = new RetAddrReg();

    // 出于简单考虑，传递参数时，全部参数都保留到栈上（而非mips规范中的$a0-$a3），此处$a0只在syscall中使用
    a0 = new ArgumentReg(0); 

    // 函数返回值寄存器
    v0 = new ValueReg(0);

    // 短路求值寄存器
    s7 = new TmpReg(17);
}

MipsManager::~MipsManager() {
    for (auto& d: datas) {
        delete d;
    }
    for (auto& c: codes) {
        delete c;
    }
    for (int i = 0; i < TMPCOUNT; i++) {
        delete tmpRegPool[i];
    }
    delete zero;
    delete sp;
    delete ra;
    delete a0;
    delete v0;
}

// Data段添加一个AsciizData
void MipsManager::addAsciiz(std::string value) {
    addData(new AsciizData("str" + std::to_string(asciizCount++), value));
}

// 产生一个新的标签名，格式为"functionName_labelCount"
std::string MipsManager::newLabelName() {
    return std::string(functionName + "_" + std::to_string(labelCount++));
}

// 获取一个基本块的标签名，并存在表blockNames中
std::string MipsManager::getLabelName(BasicBlockPtr basicBlockPtr) {
    if (blockNames.count(basicBlockPtr) == 0) {
        std::string name = newLabelName();
        blockNames.insert(std::pair<BasicBlockPtr, std::string>(basicBlockPtr, name));
    }
    std::string name = blockNames.find(basicBlockPtr)->second;
    return name;
}

// 申请一个临时寄存器
TmpRegPtr MipsManager::getFreeTmp() {
    // 寻找一个空闲的临时寄存器
    for (int i = 0; i < TMPCOUNT; i++) {
        int index = (tmpCount + i) % TMPCOUNT;
        if (tmpRegPool.find(index) != tmpRegPool.end()) {
            // 如果找到了，则返回其指针
            auto tmpPtr = tmpRegPool.find(index);
            tmpCount = (index + 1) % TMPCOUNT;
            return tmpPtr->second;
        }
    }

    // 如果没有空闲的临时寄存器，选择一个溢出到栈上
    // 这里选择了tmpCount作为溢出寄存器的索引，未来可以优化成最远使用的寄存器 TODO
    int index = tmpCount;
    tmpCount = (tmpCount + 1) % TMPCOUNT;
    ValuePtr valuePtr = nullptr;
    for (const auto &pair : occupation) {
        if (pair.second->GetIndex() == index && pair.second->GetType() == TmpRegTy) {
            valuePtr = pair.first;
            break;
        }
    }
    if (valuePtr == nullptr) {
        // std::cout << "找不到应该存在的occupy $t" << index << std::endl;
    }

    // 将溢出寄存器的值存入栈上
    push(valuePtr);

    // push执行完后，原本不存在的tmpRegPool[index]将会回来
    return tmpRegPool.find(index)->second;
}

// 按照类型分配寄存器
MipsRegPtr MipsManager::getFree(Type::TypeID type) {
    if (type == Type::IntegerTyID || type == Type::PointerTyID) {
        // 整数和指针类型，分配一个临时寄存器
        return getFreeTmp();
    } else {
        return nullptr;
    }
}

// 进入新函数时，重置栈帧
void MipsManager::resetFrame(std::string name) {
    this->functionName = name;
    addCode(new MipsLabel(name));
    blockNames.clear();
    labelCount = 0;
    currentOffset = 0;
    occupation.clear();
    tmpRegPool.clear();
    for (int i = 0; i < TMPCOUNT; i++) {
        tmpRegPool.insert(std::pair<int, TmpRegPtr>(i, new TmpReg(i)));
    }
    tmpCount = 0;
}

// 在栈上分配内存存储allocaInst的value
void MipsManager::allocMem(AllocaInstPtr allocaInstPtr, int size) {
    currentOffset -= 4 * size;
    occupy(new OffsetReg(currentOffset + 4), allocaInstPtr);
    allocaInstPtr->SetMipsOffsetToSp(currentOffset + 4); // 记录相对地址位置
}

// 给valuePtr分配一个寄存器
MipsRegPtr MipsManager::allocReg(ValuePtr valuePtr) {
    MipsRegPtr mipsRegPtr = getFree(valuePtr->GetType()->TypeId());
    occupy(mipsRegPtr, valuePtr);
    return mipsRegPtr;
}

// 获得值对应的寄存器
MipsRegPtr MipsManager::getReg(ValuePtr valuePtr, GetRegPurpose purpose) {
    if (occupation.find(valuePtr) == occupation.end()) {
        // 不在occupation中，说明是全局变量
        if (valuePtr->Is<GlobalValue>()) {
            // 用一个寄存器存放全局变量的地址
            auto addr = allocReg(valuePtr);
            addCode(new ICode(LA, addr, valuePtr->GetName()));
            // auto new_reg = getFreeTmp();
            // addCode(new ICode(LA, new_reg, valuePtr->GetName()));
            // return new_reg; // 这里返回了新的寄存器表示全局变量地址，但该寄存器是没有被occupy的，所以随时释放会导致错误
        } else {
            return nullptr;
        }
    }
    auto regPtr = occupation.find(valuePtr)->second;
    if (regPtr->GetType() == OffsetTy) {
        // 在栈上
        if (purpose == FOR_VAlUE) {
            load(valuePtr);
        } else if (purpose == FOR_ADDRESS) { // 处理有问题：如果value是个绝对地址，因函数调用或溢出到了栈上，不应该返回sp - offset，而是应该load
            // 求出地址
            release(valuePtr);
            auto addr = allocReg(valuePtr);
            addCode(new ICode(Addiu, addr, sp, regPtr->GetIndex()));
            // auto new_reg = getFreeTmp();
            // addCode(new ICode(Addiu, new_reg, sp, regPtr->GetIndex()));
            // return new_reg; // 这里返回了新的寄存器表示绝对地址，但该寄存器是没有被occupy的，所以随时释放会导致错误
        }else if (purpose == FOR_OFFSET_TO_SP) {
            // 栈上偏移量，直接返回
        }
    }
    return occupation.find(valuePtr)->second;
}

// 获得值对应的寄存器（考虑了整数常量）
MipsRegPtr MipsManager::loadConst(ValuePtr valuePtr, MipsRegType type) {
    if (!valuePtr->Is<ConstantInt>()) {
        // 不是常量，则寻找寄存器
        return getReg(valuePtr, FOR_VAlUE);
    } 
    // 常量则分配寄存器
    MipsRegPtr newRegPtr = getFree(valuePtr->GetType()->TypeId());
    if (type == TmpRegTy) {
        // 用addiu使得常量值存入临时寄存器
        addCode(new ICode(Addiu, newRegPtr, zero, valuePtr->As<ConstantInt>()->GetIntValue()));
    } 
    occupy(newRegPtr, valuePtr);
    return newRegPtr;
}

// 尝试释放一个user使用的操作数对应的寄存器
void MipsManager::tryRelease(UserPtr userPtr) {
    for (UsePtr use : *(userPtr->GetUseList())) {
        auto use_value = use->GetValue();
        // if (use_value->Is<GlobalValue>() || use_value->GetType()->IsPointerTy()) {
        //     // 全局变量或指针，不释放寄存器
        //     continue;
        // }
        // 增加已使用计数
        use_value->addUsedCount();
        // 如果已经被所有user使用过了，则释放寄存器
        if (use_value->allUseDone()) {
            release(use_value);
            if (use_value->Is<AllocaInst>()) {
                // allocaInst，释放寄存器后重新占用offset
                int offset = use_value->As<AllocaInst>()->GetMipsOffsetToSp();
                occupy(new OffsetReg(offset), use_value);
            }
        }
    }
}

// 释放一个value占用的寄存器
void MipsManager::release(ValuePtr valuePtr) {
    if (occupation.find(valuePtr)!= occupation.end()) {
        auto regPtr = occupation.find(valuePtr)->second;
        if (regPtr->GetType() == TmpRegTy) {
            // 如果是临时寄存器，则需要回收到寄存器池中
            tmpRegPool.insert(std::pair<int, TmpRegPtr>(regPtr->GetIndex(), (TmpRegPtr)regPtr));
            // std::cout << "release $t" << regPtr->GetIndex() << std::endl;
        }
        occupation.erase(valuePtr);
    }
}

// 让一个value占用一个寄存器
void MipsManager::occupy(MipsRegPtr mipsRegPtr, ValuePtr valuePtr) {
    if (mipsRegPtr->GetType() == TmpRegTy) {
        // 如果是临时寄存器，被占用后从寄存器池中删除
        tmpRegPool.erase(mipsRegPtr->GetIndex());
    }
    // 记录llvm值和寄存器的对应关系
    occupation.insert(std::pair<ValuePtr, MipsRegPtr>(valuePtr, mipsRegPtr));
    // std::cout <<"occupy $t" << mipsRegPtr->GetIndex() << "寄存器指针地址: " << mipsRegPtr << " llvm值地址: " << valuePtr << std::endl;
    // for (auto& pair : occupation) {
    //     if (pair.second->GetIndex() == mipsRegPtr->GetIndex()) {
    //         std::cout << "occupy $t" << pair.second->GetIndex() << "成功 " << std::endl;
    //     }
    // }
}

// 释放一个value占用的寄存器，然后把value存到栈上
void MipsManager::push(ValuePtr valuePtr) {
    // 把value存到栈上
    if (!valuePtr->Is<GlobalValue>() && !valuePtr->Is<AllocaInst>()) {
        // 全局变量和局部变量地址不存入栈上，直接释放
        // 再次使用全局变量时，getReg会重新la到寄存器
        // 释放局部变量后重新记录一个offset到occupation中，再次使用局部变量时使用这个offset
        addCode(new ICode(MipsCodeType::SW, getReg(valuePtr, FOR_VAlUE), sp, currentOffset));
    }

    // 释放被value占用的寄存器
    release(valuePtr);

    if (valuePtr->Is<GlobalValue>()) {
        // 全局变量占用的寄存器释放后不用push到栈上
        return;
    }

    if (valuePtr->Is<AllocaInst>()) {
        // 局部变量占用的寄存器释放后，重新占用offset
        int offset = valuePtr->As<AllocaInst>()->GetMipsOffsetToSp();
        occupy(new OffsetReg(offset), valuePtr);
        return;
    }

    // 记录新的llvm值与寄存器的对应关系
    occupation.insert(std::pair<ValuePtr, MipsRegPtr>(valuePtr, new OffsetReg(currentOffset)));

    // 栈指针指向下一个字
    currentOffset -= 4;
}

// 把occupation中所有不在栈上的寄存器存到栈上（函数调用前用此方法来保存寄存器）
void MipsManager::pushAll() {
    std::set<ValuePtr> pushSet;
    for (auto occ : occupation) {
        if (occ.second->GetType() != OffsetTy) {
            pushSet.insert(occ.first);
        }
    }
    for (auto p : pushSet) {
        push(p);
    }
}

// 将栈上的一个值重新load回寄存器
void MipsManager::load(ValuePtr valuePtr) {
    // 寻找一个空闲的寄存器
    auto mipsRegPtr = getFree(valuePtr->GetType()->TypeId());
    // 获取load对象的offset
    int offset = occupation.find(valuePtr)->second->GetIndex();
    // 产生load指令
    addCode(new ICode(MipsCodeType::LW, mipsRegPtr, sp, offset));
    release(valuePtr);
    // 记录llvm值和寄存器的对应关系
    occupy(mipsRegPtr, valuePtr);
}