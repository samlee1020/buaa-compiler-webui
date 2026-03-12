#ifndef MIPS_REG_HPP
#define MIPS_REG_HPP

#include "MipsForward.hpp"
#include <iostream>

/*
    mips寄存器类型。
    此处只考虑了$t，没有考虑$s。
*/
enum MipsRegType {
    ZeroRegTy,    //$zero
    ArgRegTy,     //$a0-$a3，函数调用的前四个参数
    ValueRegTy,   //$v0-$v1，函数调用的返回值
    TmpRegTy,     //$t0-$t7，临时寄存器
    RetAddrRegTy, //$ra，函数返回地址
    StkPtrRegTy,  //$sp，栈指针
    FrmPtrRegTy,  //$fp，帧指针
    OffsetTy      // offset，用于记录栈上没有分配reg的值的位置
};

/*
    寄存器基类
*/
class MipsReg {
    friend MipsManager;

    public:
        // 获取寄存器索引，比如$t3的索引为3
        int GetIndex() const { return index; };

        // 获取寄存器类型
        MipsRegType GetType() { return type; };
        
        virtual void PrintReg(std::ostream &out) {};

    protected:
        MipsReg(int index, MipsRegType type) : index(index), type(type){};

    private:
        int index;
        MipsRegType type;
};

/*
    $zero寄存器
*/
class ZeroReg : public MipsReg {
    friend MipsManager;
    ZeroReg() : MipsReg(-1, ZeroRegTy){};
    void PrintReg(std::ostream &out) override;
};

/*
    $a0-$a3寄存器
*/
class ArgumentReg : public MipsReg {
    friend MipsManager;

    public:
        explicit ArgumentReg(int index) : MipsReg(index, ArgRegTy) {}
        void PrintReg(std::ostream &out) override;
};

/*
    $v0-$v1寄存器
*/
class ValueReg : public MipsReg {
    friend MipsManager;
    explicit ValueReg(int index) : MipsReg(index, ValueRegTy) {};
    void PrintReg(std::ostream &out) override;
};

/*
    $t0-$t7寄存器
*/
class TmpReg : public MipsReg {
    friend MipsManager;
    explicit TmpReg(int index) : MipsReg(index, TmpRegTy) {};
    void PrintReg(std::ostream &out) override;
};

/*
    $ra寄存器
*/
class RetAddrReg : public MipsReg {
    friend MipsManager;
    RetAddrReg() : MipsReg(-1, RetAddrRegTy){};
    void PrintReg(std::ostream &out) override;
};

/*
    $sp寄存器
*/
class StkPtrReg : public MipsReg {
    friend MipsManager;
    StkPtrReg() : MipsReg(-1, StkPtrRegTy){};
    void PrintReg(std::ostream &out) override;
};

/*
    $fp寄存器
*/
class FrmPtrReg : public MipsReg {
    friend MipsManager;
    FrmPtrReg() : MipsReg(-1, FrmPtrRegTy){};
    void PrintReg(std::ostream &out) override;
};

/*
    offset寄存器。
        1. 分配到栈上的变量
        2. 溢出到栈上的变量
        
    该类寄存器的index为分配到的栈上的位置offset
*/
class OffsetReg : public MipsReg {
    friend MipsManager;
    explicit OffsetReg(int offset) : MipsReg(offset, MipsRegType::OffsetTy){};
    void PrintReg(std::ostream &out) override;
};

#endif