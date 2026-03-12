#ifndef MIPS_FORWARD_HPP
#define MIPS_FORWARD_HPP

/*
    mips生成器的前向声明
*/

// 管理器和翻译器
class MipsManager;
class Translator;

// 寄存器
class MipsReg;
using MipsRegPtr = MipsReg *;
class ZeroReg;
using ZeroRegPtr = ZeroReg *;
class ArgumentReg;
using ArgumentRegPtr = ArgumentReg *;
class ValueReg;
using ValueRegPtr = ValueReg *;
class TmpReg;
using TmpRegPtr = TmpReg *;
class RetAddrReg;
using RetAddrRegPtr = RetAddrReg *;
class StkPtrReg;
using StkPtrRegPtr = StkPtrReg *;
class FrmPtrReg;
using FrmPtrRegPtr = FrmPtrReg *;
class OffsetReg;
using OffsetRegPtr = OffsetReg *;

// 全局变量
class MipsData;
using MipsDataPtr = MipsData *;
class WordData;
using WordDataPtr = WordData *;
class FloatData;
using FloatDataPtr = FloatData *;
class AsciizData;
using AsciizDataPtr = AsciizData *;

// 指令
class MipsCode;
using MipsCodePtr = MipsCode *;
class RCode;
class ICode;
class JCode;
class MipsLabel;
using MipsLabelPtr = MipsLabel *;

#endif