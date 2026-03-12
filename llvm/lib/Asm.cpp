#include "Asm.hpp"

#include "IrForward.hpp"
#include "Module.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include <iostream>

#include <cstdarg>

#pragma region AsmPrinter

void AsmPrinter::Print(ModuleSmartPtr module, std::ostream &out) {
    AsmWriterSmartPtr writer = AsmWriter::New(out);
    _PrintHeader(writer);
    _PrintModule(writer, std::move(module));
    _PrintFooter(writer);
}

void AsmPrinter::_PrintHeader(AsmWriterSmartPtr out) {
    out->PushComment("这是 samlee1020 的编译器生成的 LLVM IR 代码").PushNewLine();
}

void AsmPrinter::_PrintFooter(AsmWriterSmartPtr out) {
    out->PushNewLine().PushComment("LLVM IR代码就此结束");
}

void AsmPrinter::_PrintModule(AsmWriterSmartPtr out, ModuleSmartPtr module) {
    // 注释输出模块名
    out->PushComment("Module ID = '%s'", module->Name().c_str());

    // 输出库函数声明
    _PrintDeclaration(out);

    // 输出全局变量
    for (auto it = module->GlobalVariableBegin(); it != module->GlobalVariableEnd(); ++it) {
        (*it)->PrintAsm(out);
    }
    out->PushNewLine();

    // 输出函数
    for (auto it = module->FunctionBegin(); it != module->FunctionEnd(); ++it) {
        if ((*it)->GetName() == "getint" || (*it)->GetName() == "putint" || (*it)->GetName() == "putch") {
            continue;
        }
        (*it)->PrintAsm(out);
    }
    out->PushNewLine();

    // 输出main函数
    module->MainFunction()->PrintAsm(out);
    out->PushNewLine();
}

void AsmPrinter::_PrintDeclaration(AsmWriterSmartPtr out) {
    /*
        declare i32 @getint()          ; 读取一个整数
        declare void @putint(i32)      ; 输出一个整数
        declare void @putch(i32)       ; 输出一个字符
    */
    out->Push("declare i32 @getint()").PushNewLine();
    out->Push("declare void @putint(i32)").PushNewLine();
    out->Push("declare void @putch(i32)").PushNewLine();
    out->PushNewLine();
}

#pragma endregion

#pragma region AsmWriter

static char buffer[1024];

const AsmWriter &AsmWriter::Push(char ch) const {
    _out << ch;
    return *this;
}

const AsmWriter &AsmWriter::Push(const char *format, ...) const {
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    _out << buffer;
    return *this;
}

const AsmWriter &AsmWriter::Push(const std::string &str) const {
    _out << str;
    return *this;
}

const AsmWriter &AsmWriter::PushNext(char ch) const {
    return PushSpace().Push(ch);
}

const AsmWriter &AsmWriter::PushNext(const char *format, ...) const {
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    return PushSpace().Push(buffer);
}

const AsmWriter &AsmWriter::PushNext(const std::string &str) const {
    return PushSpace().Push(str);
}

const AsmWriter &AsmWriter::PushSpace() const { return Push(' '); }

const AsmWriter &AsmWriter::PushSpaces(int repeat) const {
    for (int i = 0; i < repeat; i++) {
        Push(' ');
    }
    return *this;
}

const AsmWriter &AsmWriter::PushNewLine() const { return Push('\n'); }

const AsmWriter &AsmWriter::PushNewLines(int repeat) const {
    for (int i = 0; i < repeat; i++) {
        Push('\n');
    }
    return *this;
}

const AsmWriter &AsmWriter::PushComment(const char *format, ...) const {
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    return CommentBegin().Push(buffer).CommentEnd();
}

const AsmWriter &AsmWriter::CommentBegin() const {
    return Push(';').PushSpace();
}

const AsmWriter &AsmWriter::CommentEnd() const { 
    return PushNewLine(); 
}


#pragma endregion