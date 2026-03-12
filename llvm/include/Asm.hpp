#ifndef LLVM_IR_ASM_HPP
#define LLVM_IR_ASM_HPP

#include <memory>
#include "IrForward.hpp"
#include "Asm.hpp"

class AsmPrinter {
    public:
        void Print(ModuleSmartPtr module, std::ostream &out);

    private:
        void _PrintHeader(AsmWriterSmartPtr out); // 输出模块的头部信息
        void _PrintFooter(AsmWriterSmartPtr out); // 输出模块的尾部信息

        void _PrintModule(AsmWriterSmartPtr out, ModuleSmartPtr module); // 输出模块的llvm ir代码
        void _PrintDeclaration(AsmWriterSmartPtr out); // 输出库函数声明
};

class AsmWriter {
    public:
        static AsmWriterSmartPtr New(std::ostream &out) { return std::shared_ptr<AsmWriter>(new AsmWriter(out)); }

        const AsmWriter &Push(char ch) const; // 输出一个字符
        const AsmWriter &Push(const char *format, ...) const; // 输出格式化字符串
        const AsmWriter &Push(const std::string &str) const; // 输出字符串

        const AsmWriter &PushNext(char ch) const; // 输出一个空格后再输出一个字符
        const AsmWriter &PushNext(const char *format, ...) const; // 输出一个空格后再输出格式化字符串
        const AsmWriter &PushNext(const std::string &str) const; // 输出一个空格后再输出字符串

        const AsmWriter &PushSpace() const; // 输出一个空格
        const AsmWriter &PushSpaces(int repeat) const; // 输出指定数量的空格
        const AsmWriter &PushNewLine() const; // 输出一个换行符
        const AsmWriter &PushNewLines(int repeat) const; // 输出指定数量的换行符

        const AsmWriter &PushComment(const char *format, ...) const; // 输出注释
        const AsmWriter &CommentBegin() const; // 开始注释
        const AsmWriter &CommentEnd() const; // 结束注释

    private:
        AsmWriter(std::ostream &out) : _out(out) {}

        std::ostream &_out;
};

#endif