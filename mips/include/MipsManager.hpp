#ifndef MIPS_MANAGER_HPP
#define MIPS_MANAGER_HPP

#include "MipsForward.hpp"
#include "IrForward.hpp"
#include "MipsReg.hpp"
#include "MipsInst.hpp"
#include "Type.hpp"
#include <unordered_map>
#include <set>
#include <vector>
#include <iostream>

class MipsManager {
    friend Translator;

    private:
        std::vector<MipsDataPtr> datas; // 数据段
        int asciizCount = 0;
        std::string functionName;
        int labelCount = 0;

        std::vector<MipsCodePtr> codes;

        std::unordered_map<int, TmpRegPtr> tmpRegPool;
        ZeroRegPtr zero;
        StkPtrRegPtr sp;
        RetAddrRegPtr ra;
        ArgumentRegPtr a0;
        ValueRegPtr v0;

        TmpRegPtr s7; // s7 作为短路运算的临时寄存器

        int currentOffset = 0;

        std::unordered_map<ValuePtr, MipsRegPtr> occupation;
        std::unordered_map<BasicBlockPtr, std::string> blockNames;

        int tmpCount = 0;
        void addCode(MipsCodePtr codePtr) { codes.emplace_back(codePtr); };
        void addData(MipsDataPtr dataPtr) { datas.emplace_back(dataPtr); };
        void addAsciiz(std::string);
        std::string newLabelName();
        std::string getLabelName(BasicBlockPtr basicBlockPtr);
        void resetFrame(std::string name);
        void allocMem(AllocaInstPtr allocaInstPtr, int size);
        MipsRegPtr allocReg(ValuePtr valuePtr);
        enum GetRegPurpose {
            FOR_VAlUE,
            FOR_OFFSET_TO_SP,
            FOR_ADDRESS
        };
        MipsRegPtr getReg(ValuePtr valuePtr, GetRegPurpose purpose);
        MipsRegPtr loadConst(ValuePtr valuePtr, MipsRegType type);
        void tryRelease(UserPtr userPtr);
        void pushAll();

        TmpRegPtr getFreeTmp();
        MipsRegPtr getFree(Type::TypeID type);
        void release(ValuePtr valuePtr);
        void occupy(MipsRegPtr mipsRegPtr, ValuePtr valuePtr);
        void push(ValuePtr valuePtr);
        void load(ValuePtr valuePtr);

    public:
        MipsManager();
        ~MipsManager();
        void PrintMips(std::ostream &_out);
};

#endif