#ifndef MIPS_INST_HPP
#define MIPS_INST_HPP

#include "MipsForward.hpp"
#include <string>
#include <vector>
#include <iostream>

class MipsData;
class MipsCode;

#pragma region MipsData

/*
    mips全局变量的基类
    
*/
class MipsData {
    public:
        std::string GetName() { return name; };
        int GetValue{};
        virtual void PrintData(std::ostream &out) {};

        // 转换类型，当且仅当知道确切的类型时才使用
        template <typename _Ty> _Ty *As() { return static_cast<_Ty *>(this); }

    protected:
        explicit MipsData(std::string &name) : name(name){};
        void printName(std::ostream &out) { out << name; };
        virtual void printValue(std::ostream &out) {};

        std::string name;
};

/*
    .word类。继承自MipsData类，用于表示一个字的数据类型。
*/
class WordData : public MipsData {
    public:
        WordData(std::string name, int value) : MipsData(name) {
            w_value = value;
        }
        void PrintData(std::ostream &out) override;

    private:
        int w_value;
        void printValue(std::ostream &out) override;
};


/*
    多个.word，表示数组，继承自MipsData类。
*/
class ArrayData : public MipsData {
    public:
        ArrayData(std::string name, std::vector<int> values) : MipsData(name) {
            w_values = values;
        }
        void PrintData(std::ostream &out) override;

    private:
        std::vector<int> w_values;
        void printValue(std::ostream &out) override;
};

/*
    .asciiz类。继承自MipsData类，用于表示以空字符结尾的字符串。
*/
class AsciizData : public MipsData {
    public:
        AsciizData(std::string name, std::string &value) : MipsData(name) {
            str = value;
        }
        void PrintData(std::ostream &out) override;

    private:
        std::string str;
        void printValue(std::ostream &out) override;
};

#pragma endregion

#pragma region MipsCode

enum MipsCodeType {
    // error
    Error,
    // ICode 
    LA,
    Bnez,
    LW,
    SW,
    Addiu,
    Subiu,
    // ICode or RCode 
    Div,
    Mul,
    Rem,
    // RCode
    Addu,
    Subu,
    Slt,
    Sle,
    Sgt,
    Sge,
    Seq,
    Sne,
    Sll,
    Sra,
    Jr,
    Nop,
    Syscall,
    // JCode
    J,
    Jal,
    // Label
    Label,
    Comment // 注释
};

/*
    mips指令的基类。
*/
class MipsCode {
    public:
        virtual void PrintCode(std::ostream &out) {};
        virtual ~MipsCode() = default;
        
        // 转换类型，当且仅当知道确切的类型时才使用
        template <typename _Ty> _Ty *As() { return static_cast<_Ty *>(this); }

    protected:
        MipsCodeType op;
        MipsCode(MipsCodeType op)
            : op(op){};
};

class CommentCode : public MipsCode {
    public:
        ~CommentCode() override = default;
        void PrintCode(std::ostream &out) override;
        explicit CommentCode(std::string comment)
            : MipsCode(MipsCodeType::Comment), comment(comment) {
        };
    private:
        std::string comment;
};

/*
    Rcode类。继承自MipsCode类，用于表示R型指令。
*/
class RCode : public MipsCode {
    public:
        ~RCode() override = default;
        //  Div, Mul, Rem,
        //  Addu, Subu,
        //  Slt, Sle, Sgt, Sge, Seq, Sne,
        RCode(MipsCodeType op, MipsRegPtr rd, MipsRegPtr rs, MipsRegPtr rt)
            : MipsCode(op) , rd(rd), rs(rs), rt(rt) {}

        // sll $t, $t, 2
        RCode(MipsCodeType op, MipsRegPtr rd, MipsRegPtr rt, int sa)
            : MipsCode(op) , rd(rd), rt(rt), sa(sa) {}

        // jr
        RCode(MipsCodeType op, MipsRegPtr rs)
            : MipsCode(op) , rs(rs) {}

        // syscall, nop
        explicit RCode(MipsCodeType op)
            : MipsCode(op) {};

        void PrintCode(std::ostream &out) override;
    
    private:
        MipsRegPtr rd = nullptr;
        MipsRegPtr rs = nullptr;
        MipsRegPtr rt = nullptr;
        int sa; // 用于移位指令的立即数
};

/*
    立即数指令
*/
class ICode : public MipsCode {
    public:
    ~ICode() override = default;
        // LW, SW
        // Addiu, Subiu, Div, Mul, Rem
        ICode(MipsCodeType op, MipsRegPtr rt, MipsRegPtr rs, int inter)
            : MipsCode(op), rt(rt), rs(rs), inter(inter) {}

        // Bnez, Lw, Sw, La
        ICode(MipsCodeType op, MipsRegPtr rs, std::string label)
            : MipsCode(op), rs(rs), label(label) {
        };

        void PrintCode(std::ostream &out) override;
    
    private:
        MipsRegPtr rs = nullptr;
        MipsRegPtr rt = nullptr;
        int inter;
        std::string label = "";
};

/*
    跳转指令
*/
class JCode : public MipsCode {
    public:
        ~JCode() override = default;
        // J, Jal
        JCode(MipsCodeType op, std::string label)
            : MipsCode(op), label(label) {
        };
        void PrintCode(std::ostream &out) override;

    private:
        std::string label;
};

/*
    标签
*/
class MipsLabel : public MipsCode {
    public:
        ~MipsLabel() override = default;
        explicit MipsLabel(std::string label)
            : MipsCode(MipsCodeType::Label), label(label) {
        };

        std::string GetName() { return label; }
        void PrintCode(std::ostream &out) override;
    
    private:
        std::string label;
};

#pragma endregion
#endif