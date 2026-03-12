#include "SlotTracker.hpp"
#include "IrForward.hpp"
#include "Function.hpp"
#include "Instruction_base.hpp"

/*
    给函数包含的所有value编号，包含以下步骤
        1. 清空_slot，即清空上个函数的编号
        2. 遍历函数的参数并给其编号
        3. 遍历函数的基本块，给块编号。遍历块中的指令，给（非void的）指令编号。
*/
void SlotTracker::Trace(FunctionPtr function) {
    int slot = 0;
    _slot.clear();

    // 加入参数
    for (auto arg = function->ArgBegin(); arg != function->ArgEnd(); ++arg) {
        _slot.emplace(*arg, slot++);
    }

    // 加入基本块和指令
    for (auto blockIter = function->BasicBlockBegin(); blockIter != function->BasicBlockEnd(); ++blockIter) {
        BasicBlockPtr block = *blockIter;
        _slot.emplace(block, slot++);

        for (auto instIter = block->InstructionBegin(); instIter != block->InstructionEnd(); ++instIter) {
            InstructionPtr inst = *instIter;

            if (!inst->GetType()->IsVoidTy()) {
                _slot.emplace(inst, slot++);
            }
        }
    }
}

int SlotTracker::Slot(ValuePtr value) {
    auto iter = _slot.find(value);
    if (iter != _slot.end()) {
        return iter->second;
    } 
    return -1;
}