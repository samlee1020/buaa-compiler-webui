#include "Lexer.hpp"
#include "Token.hpp"
#include "Parser.hpp"
#include "Ast.hpp"
#include "Visitor.hpp"
#include "Module.hpp"
#include "translator.hpp"
#include "GlobalVar.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

// 辅助函数：对 ostringstream 的内容按行稳定排序（按第一个整数）
void stableSortAndWrite(std::ostringstream& source, std::ostream& dest) {
    // 获取所有行
    std::vector<std::string> lines;
    std::istringstream iss(source.str());
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty()) {  // 跳过空行
            lines.push_back(line);
        }
    }

    // 按第一个整数稳定排序（使用 std::stable_sort）
    std::stable_sort(lines.begin(), lines.end(), [](const std::string& a, const std::string& b) {
        int numA = 0, numB = 0;
        std::istringstream issA(a), issB(b);
        issA >> numA;  // 提取第一个整数
        issB >> numB;
        return numA < numB;  // 升序排序
    });

    // 写入目标流
    for (const auto& sortedLine : lines) {
        dest << sortedLine << '\n';
    }
}

int main(int argc, char* argv[]) {
    OPTIMIZE = false; // 是否开启优化
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--optimize" || std::string(argv[i]) == "-o") {
            OPTIMIZE = true;
        }
    }

    std::ifstream fileStream;
    std::ofstream symbolStream;
    std::ofstream errorStream;
    std::ofstream llvmStream;
    std::ofstream mipsStream;

    // 尝试在当前目录打开临时文件
    fileStream.open("./temp_testfile.txt");
    if (!fileStream.is_open()) {
        fileStream.open("../temp_testfile.txt");
        symbolStream.open("../symbol.txt");
        errorStream.open("../error.txt");
        llvmStream.open("../llvm_ir.txt");
        mipsStream.open("../mips.txt");
    } else {
        symbolStream.open("./symbol.txt");
        errorStream.open("./error.txt");
        llvmStream.open("./llvm_ir.txt");
        mipsStream.open("./mips.txt");
    }

    std::ostringstream temp_error_out, temp_symbol_out;

    Lexer lexer(fileStream);

    Parser parser(lexer, temp_error_out);

    std::unique_ptr<CompUnit> compUnit = parser.parse();

    // compUnit->print(std::cout);

    auto module = Module::New("test");

    Visitor visitor = Visitor(temp_error_out, temp_symbol_out, module);

    visitor.visit(*compUnit);

    // 对temp_error_out和temp_symbol_out每一行按照第一个整数进行排序
    stableSortAndWrite(temp_error_out, errorStream);
    stableSortAndWrite(temp_symbol_out, symbolStream);

    if (!VISITOR_ERROR) { // 若无错误，则输出 LLVM IR 和 MIPS 汇编
        // 输出 LLVM IR 
        AsmPrinter printer;
        printer.Print(module, llvmStream);

        // 输出 MIPS 汇编
        Translator translator;
        translator.translate(module);
        translator.print(mipsStream);
    } else {
        llvmStream << "错误！访问器Visitor访问过程中发生语义错误，故不进行代码生成。\n";
        mipsStream << "错误！访问器Visitor访问过程中发生语义错误，故不进行代码生成。\n";
    }
    
    fileStream.close();
    symbolStream.close();
    errorStream.close();
    llvmStream.close();
    mipsStream.close();

    return 0;
}