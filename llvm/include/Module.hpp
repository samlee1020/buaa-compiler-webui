#ifndef LLVM_MODULE_HPP
#define LLVM_MODULE_HPP

#include "IrForward.hpp"
#include "LlvmContext.hpp"
#include <string>
#include <vector>

/*
    模块类
    在llvm ir中，一个.c文件对应一个模块
    模块包含：全局变量，自定义函数，main函数。和前端的compunit的内容一一对应
*/
class Module final {
    public:
        // 工厂模式
        static ModuleSmartPtr New(const std::string& name) { return std::shared_ptr<Module>(new Module(name)); }

        const std::string Name() const { return _name; } // 获取模块名
        LlvmContextPtr Context() { return &_context; } // 获取上下文指针

        // 用于遍历模块全局变量的接口
        using global_variable_iterator = std::vector<GlobalVariablePtr>::iterator;
        global_variable_iterator GlobalVariableBegin() { return _globalVariables.begin(); }
        global_variable_iterator GlobalVariableEnd() { return _globalVariables.end(); }
        int GlobalVariableCount() const { return static_cast<int>(_globalVariables.size()); }

        // 用于遍历模块函数的接口
        using function_iterator = std::vector<FunctionPtr>::iterator;
        function_iterator FunctionBegin() { return _functions.begin(); }
        function_iterator FunctionEnd() { return _functions.end(); }
        int FunctionCount() const { return static_cast<int>(_functions.size()); }

        // 获取main函数的接口
        FunctionPtr MainFunction() const { return _mainFunction; }    

        // 添加全局变量
        void AddGlobalVariable(GlobalVariablePtr globalVariable) { _globalVariables.push_back(globalVariable); }

        // 添加函数
        void AddFunction(FunctionPtr function) { _functions.push_back(function); }
        void AddMainFunction(FunctionPtr function) { _mainFunction = function; }

    private:
        Module(const std::string& name) : _name(name), _context() {} // 私有构造函数

        std::string _name; // 模块名
        LlvmContext _context; // 模块上下文。该变量会在Module初始化时被创建，管理所有的value，并在Module销毁时被销毁

        // 以下传统指针的生命周期由LlvmContext管理
        std::vector<GlobalVariablePtr> _globalVariables; // 全局变量列表
        std::vector<FunctionPtr> _functions; // 函数列表
        FunctionPtr _mainFunction; // main函数

};

#endif