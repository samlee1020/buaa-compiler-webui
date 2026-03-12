#include "MipsInst.hpp"
#include "MipsManager.hpp"
#include "MipsReg.hpp"
#include <iostream>

void MipsManager::PrintMips(std::ostream &_out) {
    _out << ".data" << std::endl;
    for (auto data : datas) {
        data->PrintData(_out);
    }

    _out << ".text" << std::endl << "j main" << std::endl << "nop" << std::endl;
    for (auto code : codes) {
        code->PrintCode(_out);
    }
}

void ZeroReg::PrintReg(std::ostream &out) { out << "$zero"; }

void ArgumentReg::PrintReg(std::ostream &out) { out << "$a" << GetIndex(); }

void ValueReg::PrintReg(std::ostream &out) { out << "$v" << GetIndex(); }

void TmpReg::PrintReg(std::ostream &out) { 
    if (GetIndex() < 10) {
        out << "$t" << GetIndex(); 
    } else {
        out << "$s" << GetIndex() - 10;
    }
}

void RetAddrReg::PrintReg(std::ostream &out) { out << "$ra"; }

void StkPtrReg::PrintReg(std::ostream &out) { out << "$sp"; }

void FrmPtrReg::PrintReg(std::ostream &out) { out << "$fp"; }

void OffsetReg::PrintReg(std::ostream &out) { out << GetIndex(); }

void WordData::printValue(std::ostream &out) { out << w_value; }

void WordData::PrintData(std::ostream &out) {
    printName(out);
    out << ": .word ";
    printValue(out);
    out << std::endl;
}

void ArrayData::printValue(std::ostream &out) { 
    for (int i = 0; i < w_values.size(); i++) {
        out << w_values[i];
        if (i != w_values.size() - 1) {
            out << ", ";
        }
    }
}

void ArrayData::PrintData(std::ostream &out) {
    printName(out);
    out << ": .word ";
    printValue(out);
    out << std::endl;
}

void AsciizData::printValue(std::ostream &out) { 
    out << "\"";
    for (char c : str) {
        if (c == '\n') {
            out << "\\n";
        } else {
            out << c;
        }
    }
    out << "\"";
}

void AsciizData::PrintData(std::ostream &out) {
    printName(out);
    out << ": .asciiz ";
    printValue(out);
    out << std::endl;
}

void RCode::PrintCode(std::ostream &out) {
    out << "    ";
    std::string Ope;
    if (op >= Div && op <= Sne) {
        Ope = op == Div  ? "div"
            : op == Mul  ? "mul"
            : op == Rem  ? "rem"
            : op == Addu ? "addu"
            : op == Subu ? "subu"
            : op == Slt  ? "slt"
            : op == Sle  ? "sle"
            : op == Sgt  ? "sgt"
            : op == Sge  ? "sge"
            : op == Seq  ? "seq"
            : op == Sne  ? "sne"
            : "";
        out << Ope << " ";
        rd->PrintReg(out);
        out << ", ";
        rs->PrintReg(out);
        out << ", ";
        rt->PrintReg(out);
        out << std::endl;
        return;
    } else if (op == Sll || op == Sra) {
        std::string Ope = op == Sll ? "sll" : "sra";
        out << Ope << " ";
        rd->PrintReg(out);
        out << ", ";
        rt->PrintReg(out);
        out << ", " << sa << std::endl;
        return;
    } else if (op == Jr) {
        out << "jr ";
        rs->PrintReg(out);
        out << std::endl;
        return;
    } else if (op == Syscall || op == Nop) {
        Ope = op == Syscall ? "syscall" : op == Nop ? "nop" : "";
        out << Ope << std::endl;
        return;
    } 
}

void ICode::PrintCode(std::ostream &out) {
    std::string Ope;
    if (op >= LW && op <= Rem && rt != nullptr) {
        out << "    ";
        Ope = op == LW    ? "lw"
            : op == SW    ? "sw"
            : op == Addiu ? "addiu"
            : op == Subiu ? "subiu"
            : op == Div   ? "div"
            : op == Mul   ? "mul"
            : op == Rem   ? "rem"
            : "";
        out << Ope << " ";
        if (op >= LW && op <= SW) {
            rt->PrintReg(out);
            out << ", " << inter << "(";
            rs->PrintReg(out);
            out << ")" << std::endl;
        } else {
            rt->PrintReg(out);
            out << ", ";
            rs->PrintReg(out);
            out << ", " << inter << std::endl;
        }
        return;
    } else if (op >= LA && op <= SW && rt == nullptr && label != "") {
        out << "    ";
        Ope = op == LA   ? "la"
            : op == Bnez ? "bnez"
            : op == LW   ? "lw"
            : op == SW   ? "sw"
            : "";
        out << Ope << " ";
        rs->PrintReg(out);
        out << ", " << label << std::endl;
    }
}

void JCode::PrintCode(std::ostream &out) {
    out << "    ";
    std::string Ope = op == J ? "j" : op == Jal ? "jal" : "";
    out << Ope << " " << label << std::endl;
}

void MipsLabel::PrintCode(std::ostream &out) {
    out << std::endl << label << ":" << std::endl;
}

void CommentCode::PrintCode(std::ostream &out) {
    out << "# " << comment << std::endl;
}