#include "Visitor.hpp"
#include "Symbol.hpp"
#include <memory>
#include <variant>
#include "Constant.hpp"
#include "Instruction.hpp"
#include "InstructionType.hpp"
#include "Function.hpp"
#include <iostream>
#include "GlobalVar.hpp"

Visitor::Visitor(std::ostream& error_out, std::ostream& symbol_out, ModuleSmartPtr module) 
    : _cur_scope(std::make_shared<SymbolTable>(1)), 
    _error_out(error_out), _symbol_out(symbol_out), _cur_table_id(1), _ir_module(module),
    _is_void_func(false), _loop_num(0) {
    // 给模块加入库函数
    /*
        declare i32 @getint()          ; 读取一个整数
        declare void @putint(i32)      ; 输出一个整数
        declare void @putch(i32)       ; 输出一个字符
    */
    auto getint = Function::New(_ir_module->Context()->GetInt32Ty(), "getint");
    auto putint = Function::New(_ir_module->Context()->GetVoidTy(), "putint", { Argument::New(_ir_module->Context()->GetInt32Ty(), "int_to_print") });
    auto putch = Function::New(_ir_module->Context()->GetVoidTy(), "putch", { Argument::New(_ir_module->Context()->GetInt32Ty(), "char_to_print") });
    _ir_module->AddFunction(getint);
    _ir_module->AddFunction(putint);
    _ir_module->AddFunction(putch);

    // 给符号表加入getint函数（该函数会在源代码出现，故需要加到符号表）
    auto getint_symbol = std::make_shared<FuncSymbol>("getint", 0, 1, 1, std::vector<bool>{}, getint);
    _cur_scope->add_symbol(getint_symbol);

    // putint和putch函数只在llvm ir中生成，不需要加入符号表
    _putch = putch;
    _putint = putint;
}

void Visitor::_visit_comp_unit(const CompUnit& comp_unit) {
    // CompUnit → {Decl} {FuncDef} MainFuncDef
    for (const auto& decl : comp_unit.decls) {
        if (decl) {
            _visit_decl(*decl);
        }
    }
    for (const auto& funcDef : comp_unit.funcDefs) {
        if (funcDef) {
            _visit_func_def(*funcDef);
        }
    }
    if (comp_unit.mainFuncDef) {
        _visit_main_func_def(*comp_unit.mainFuncDef);
    }
}

void Visitor::_visit_decl(const Decl& decl) {
    if (std::holds_alternative<ConstDecl>(decl)) {
        _visit_const_decl(std::get<ConstDecl>(decl));
    } else if (std::holds_alternative<VarDecl>(decl)) {
        _visit_var_decl(std::get<VarDecl>(decl));
    }
}

void Visitor::_visit_const_decl(const ConstDecl& const_decl) {
    // ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'
    for (const auto& const_def : const_decl.const_defs) {
        if (const_def) {
            _visit_const_def(*const_def);
        }
    }
}

void Visitor::_visit_const_def(const ConstDef& const_def) {
    // ConstDef → Ident [ '[' ConstExp ']' ] '=' ConstInitVal
    ValuePtr const_exp_val;
    std::vector<ValuePtr> init_values;
    if (const_def.ident) {
        _visit_ident(*const_def.ident);
    }
    if (const_def.const_exp) {
        const_exp_val = _visit_const_exp(*const_def.const_exp);
    }
    if (const_def.const_init_val) {
        init_values = _visit_const_init_val(*const_def.const_init_val);
    }

    ValuePtr symbol_value;

    // 处理符号表
    bool is_array = false;
    bool is_const = true;
    bool is_static = false;
    int array_size = 0;
    if (const_def.const_exp) {
        is_array = true;
    }
    if (const_def.const_init_val) {
        array_size = const_def.const_init_val->const_exps.size();
    }
    auto symbol = std::make_shared<VarSymbol>(const_def.ident->name, const_def.ident->lineno, _cur_table_id, is_array, is_const, is_static, array_size, symbol_value);
    if (!_cur_scope->add_symbol(symbol)) {
        // 重复定义
        _error_out << const_def.ident->lineno << " " << "b:重复定义" << std::endl; 
        VISITOR_ERROR = true;
        return;
    }

    // 输出符号
    std::string symbol_type_str = (is_array) ? _symbol_type_to_string(SymbolType::ConstIntArray) : 
                                                _symbol_type_to_string(SymbolType::ConstInt);
    _symbol_out << _cur_scope->get_table_id() << " " << symbol->name << " " << symbol_type_str << std::endl;

    // 创建LLVM变量
    if (VISITOR_ERROR) {
        return;
    }
    if (_cur_table_id == 1) {
        // 全局常量，要创建成GlobalVariable
        if (init_values.size() == 1) {
            // 整数
            auto initialInt = init_values[0]->As<ConstantInt>();
            auto i32_type = _ir_module->Context()->GetInt32Ty();
            auto i32_pointer_type = _ir_module->Context()->GetPointerType(i32_type);
            auto global_var = GlobalVariable::New(i32_pointer_type, const_def.ident->name, true, initialInt);
            _ir_module->AddGlobalVariable(global_var);
            symbol->set_value(global_var);
        } else {
            // 数组
            std::vector<ConstantIntPtr> initialInts;
            int array_size = const_exp_val->As<ConstantInt>()->GetIntValue(); // 数组大小
            for (int i = 0; i < array_size; i++) {
                if (i < init_values.size()) {
                    auto initialInt = init_values[i]->As<ConstantInt>();
                    initialInts.push_back(initialInt);
                } else {
                    // 剩下部分初值默认为0
                    initialInts.push_back(ConstantInt::New(_ir_module->Context()->GetInt32Ty(), 0));
                }
            }
            auto array_type = _ir_module->Context()->GetArrayType(_ir_module->Context()->GetInt32Ty(), array_size);
            auto array_pointer_type = _ir_module->Context()->GetPointerType(array_type);
            auto const_array = ConstantArray::New(array_type, initialInts);
            auto global_var = GlobalVariable::New(array_pointer_type, const_def.ident->name, true, const_array);
            _ir_module->AddGlobalVariable(global_var);
            symbol->set_value(global_var);
        }   
    } else {
        // 局部常量，类似于static变量，转化为全局变量
        // 静态变量，要创建成GlobalVariable，静态变量的初值表达式一定是常量
        if (!const_def.const_exp) {
            // 整数
            ConstantIntPtr initialInt;
            if (const_def.const_init_val) {
                // 有初值表达式
                initialInt = init_values[0]->As<ConstantInt>();
            } else {
                // 没有初值表达式，默认初始化为0
                initialInt = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), 0);
            }
            //  给局部常量在全局变量命名：函数名.符号表id.变量名
            std::string static_var_name = _cur_func->GetName() + "." + std::to_string(_cur_table_id) + "." + const_def.ident->name;
            auto i32_type = _ir_module->Context()->GetInt32Ty();
            auto i32_pointer_type = _ir_module->Context()->GetPointerType(i32_type);
            auto global_var = GlobalVariable::New(i32_pointer_type, static_var_name, false, initialInt);
            _ir_module->AddGlobalVariable(global_var);
            symbol->set_value(global_var);
        } else {
            // 数组
            std::vector<ConstantIntPtr> initialInts;
            int array_size = const_exp_val->As<ConstantInt>()->GetIntValue(); // 数组大小
            if (const_def.const_init_val) {
                // 有初值表达式
                for (int i = 0; i < array_size; i++) {
                    ConstantIntPtr initialInt;
                    if (i < init_values.size()) {
                        initialInt = init_values[i]->As<ConstantInt>();
                    } else {
                        // 剩下部分初值默认为0
                        initialInt = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), 0);
                    }
                    initialInts.push_back(initialInt);
                }
            } // 没有初值表达式，则initialInts为空数组，生成ir时会自动初始化为0
            
            //  给静态局部变量在全局变量命名：函数名.符号表id.变量名
            std::string static_var_name = _cur_func->GetName() + "." + std::to_string(_cur_table_id) + "." + const_def.ident->name;
            auto array_type = _ir_module->Context()->GetArrayType(_ir_module->Context()->GetInt32Ty(), array_size);
            auto array_pointer_type = _ir_module->Context()->GetPointerType(array_type);
            auto const_array = ConstantArray::New(_ir_module->Context()->GetArrayType(_ir_module->Context()->GetInt32Ty(), array_size), initialInts);
            auto global_var = GlobalVariable::New(array_pointer_type, static_var_name, false, const_array);
            _ir_module->AddGlobalVariable(global_var);
            symbol->set_value(global_var);
        } 
    }
}

std::vector<ValuePtr> Visitor::_visit_const_init_val(const ConstInitVal& const_init_val) {
    // ConstInitVal → ConstExp | '{' [ ConstExp { ',' ConstExp } ] '}'
    std::vector<ValuePtr> const_exp_vals;
    for (const auto& const_exp : const_init_val.const_exps) {
        if (const_exp) {
            const_exp_vals.push_back(_visit_const_exp(*const_exp));
        }
    }
    return const_exp_vals;
}

void Visitor::_visit_var_decl(const VarDecl& var_decl) {
    // VarDecl → [ 'static' ] BType VarDef { ',' VarDef } ';'
    for (const auto& var_def : var_decl.var_defs) {
        if (var_def) {
            _visit_var_def(*var_def, var_decl.is_static);
        }
    }
}

void Visitor::_visit_var_def(const VarDef& var_def, bool is_static) {
    // VarDef → Ident [ '[' ConstExp ']' ] | Ident [ '[' ConstExp ']' ] '=' InitVal
    ValuePtr const_exp_val;
    std::vector<ValuePtr> init_values;
    if (var_def.ident) {
        _visit_ident(*var_def.ident);
    }
    if (var_def.const_exp) {
        const_exp_val = _visit_const_exp(*var_def.const_exp);
    }
    if (var_def.init_val) {
        init_values = _visit_init_val(*var_def.init_val);
    }

    ValuePtr symbol_value;

    // 处理符号表
    bool is_array = false;
    bool is_const = false;
    int array_size = 0;
    if (var_def.const_exp) {
        is_array = true;
    }
    if (var_def.init_val) {
        array_size = var_def.init_val->exps.size();
    }
    auto symbol = std::make_shared<VarSymbol>(var_def.ident->name, var_def.ident->lineno, _cur_table_id, is_array, is_const, is_static, array_size, symbol_value);
    if (!_cur_scope->add_symbol(symbol)) {
        // 重复定义
        _error_out << var_def.ident->lineno << " " << "b:重复定义" << std::endl; 
        VISITOR_ERROR = true;
        return;
    }

    // 输出符号
    std::string symbol_type_str;
    if (is_array) {
        if (is_static) {
            symbol_type_str = _symbol_type_to_string(SymbolType::StaticIntArray);
        } else {
            symbol_type_str = _symbol_type_to_string(SymbolType::IntArray);
        }
    } else {
        if (is_static) {
            symbol_type_str = _symbol_type_to_string(SymbolType::StaticInt);
        } else {
            symbol_type_str = _symbol_type_to_string(SymbolType::Int);
        }
    }
    _symbol_out << _cur_scope->get_table_id() << " " << symbol->name << " " << symbol_type_str << std::endl;

    // 创建LLVM变量
    if (VISITOR_ERROR) {
        return;
    }
    if (_cur_table_id == 1) {
        // 全局变量，要创建成GlobalVariable
        if (!var_def.const_exp) {
            // 整数
            ConstantIntPtr initialInt;
            if (var_def.init_val) {
                // 有初值表达式
                initialInt = init_values[0]->As<ConstantInt>();
            } else {
                // 没有初值表达式，默认初始化为0
                initialInt = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), 0);
            }
            auto i32_type = _ir_module->Context()->GetInt32Ty();
            auto i32_pointer_type = _ir_module->Context()->GetPointerType(i32_type);
            auto global_var = GlobalVariable::New(i32_pointer_type, var_def.ident->name, false, initialInt);
            _ir_module->AddGlobalVariable(global_var);
            symbol->set_value(global_var);
        } else {
            // 数组
            std::vector<ConstantIntPtr> initialInts;
            int array_size = const_exp_val->As<ConstantInt>()->GetIntValue(); // 数组大小
            if (var_def.init_val) {
                // 有初值表达式
                for (int i = 0; i < array_size; i++) {
                    ConstantIntPtr initialInt;
                    if (i < init_values.size()) {
                        initialInt = init_values[i]->As<ConstantInt>();
                    } else {
                        // 剩下部分初值默认为0
                        initialInt = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), 0);
                    }
                    initialInts.push_back(initialInt);
                }
            } // 没有初值表达式，则initialInts为空数组，生成ir时会自动初始化为0
            auto array_type = _ir_module->Context()->GetArrayType(_ir_module->Context()->GetInt32Ty(), array_size);
            auto array_pointer_type = _ir_module->Context()->GetPointerType(array_type);
            auto const_array = ConstantArray::New(array_type, initialInts);
            auto global_var = GlobalVariable::New(array_pointer_type, var_def.ident->name, false, const_array);
            _ir_module->AddGlobalVariable(global_var);
            symbol->set_value(global_var);
        } 
    } else {
        // 局部变量
        if (is_static) {
            // 静态变量，要创建成GlobalVariable，静态变量的初值表达式一定是常量
            if (!var_def.const_exp) {
                // 整数
                ConstantIntPtr initialInt;
                if (var_def.init_val) {
                    // 有初值表达式
                    initialInt = init_values[0]->As<ConstantInt>();
                } else {
                    // 没有初值表达式，默认初始化为0
                    initialInt = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), 0);
                }
                //  给静态局部变量在全局变量命名：函数名.符号表id.变量名
                std::string static_var_name = _cur_func->GetName() + "." + std::to_string(_cur_table_id) + "." + var_def.ident->name;
                auto i32_type = _ir_module->Context()->GetInt32Ty();
                auto i32_pointer_type = _ir_module->Context()->GetPointerType(i32_type);
                auto global_var = GlobalVariable::New(i32_pointer_type, static_var_name, false, initialInt);
                _ir_module->AddGlobalVariable(global_var);
                symbol->set_value(global_var);
            } else {
                // 数组
                std::vector<ConstantIntPtr> initialInts;
                int array_size = const_exp_val->As<ConstantInt>()->GetIntValue(); // 数组大小
                if (var_def.init_val) {
                    // 有初值表达式
                    for (int i = 0; i < array_size; i++) {
                        ConstantIntPtr initialInt;
                        if (i < init_values.size()) {
                            initialInt = init_values[i]->As<ConstantInt>();
                        } else {
                            // 剩下部分初值默认为0
                            initialInt = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), 0);
                        }
                        initialInts.push_back(initialInt);
                    }
                } // 没有初值表达式，则initialInts为空数组，生成ir时会自动初始化为0
                
                //  给静态局部变量在全局变量命名：函数名.符号表id.变量名
                std::string static_var_name = _cur_func->GetName() + "." + std::to_string(_cur_table_id) + "." + var_def.ident->name;
                auto array_type = _ir_module->Context()->GetArrayType(_ir_module->Context()->GetInt32Ty(), array_size);
                auto array_pointer_type = _ir_module->Context()->GetPointerType(array_type);
                auto const_array = ConstantArray::New(_ir_module->Context()->GetArrayType(_ir_module->Context()->GetInt32Ty(), array_size), initialInts);
                auto global_var = GlobalVariable::New(array_pointer_type, static_var_name, false, const_array);
                _ir_module->AddGlobalVariable(global_var);
                symbol->set_value(global_var);
            } 
        } else {
            // 动态变量，要创建成AllocaInst
            if (!var_def.const_exp) {
                // 整数
                auto alloca_inst = AllocaInst::New(_ir_module->Context()->GetInt32Ty());
                (*_cur_func->BasicBlockBegin())->InsertInstruction(alloca_inst); // AllocaInst需要分配在函数的第一个基本块
                if (var_def.init_val) {
                    auto store_inst = StoreInst::New(init_values[0], alloca_inst);
                    _cur_block->InsertInstruction(store_inst);
                }             
                symbol->set_value(alloca_inst);
            } else {
                // 数组
                int array_size = const_exp_val->As<ConstantInt>()->GetIntValue();
                auto alloca_inst = AllocaInst::New(_ir_module->Context()->GetArrayType(_ir_module->Context()->GetInt32Ty(), array_size));
                (*_cur_func->BasicBlockBegin())->InsertInstruction(alloca_inst); // AllocaInst需要分配在函数的第一个基本块
                if (var_def.init_val) {
                    for (int i = 0; i < init_values.size(); i++) {
                        auto index_val = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), i);
                        auto gep_inst = GepInst::New(alloca_inst, index_val);
                        _cur_block->InsertInstruction(gep_inst);
                        auto store_inst = StoreInst::New(init_values[i], gep_inst);
                        _cur_block->InsertInstruction(store_inst);
                    }
                }  
                symbol->set_value(alloca_inst);
            }
        }
    }
}

std::vector<ValuePtr> Visitor::_visit_init_val(const InitVal& init_val) {
    // InitVal → Exp | '{' [ Exp { ',' Exp } ] '}'
    std::vector<ValuePtr> exp_vals;
    for (const auto& exp : init_val.exps) {
        if (exp) {
            exp_vals.push_back(_visit_exp(*exp));
        }
    }
    return exp_vals;
}

void Visitor::_visit_func_def(const FuncDef& func_def) {
    // FuncDef → FuncType Ident '(' [FuncFParams] ')' Block
    if (func_def.func_type) {
        _visit_func_type(*func_def.func_type);
    }
    if (func_def.ident) {
        _visit_ident(*func_def.ident);
    }

    // 处理符号表
    std::vector<bool> params_is_array;
    if (func_def.func_fparams) {
        for (const auto& func_fparam : func_def.func_fparams->func_fparams) {
            if (func_fparam) {
                if (func_fparam->is_array) {
                    params_is_array.push_back(true);
                } else {
                    params_is_array.push_back(false);
                }
            }
        }
    }
    auto symbol = std::make_shared<FuncSymbol>(func_def.ident->name, func_def.ident->lineno, _cur_table_id, func_def.func_type->is_int, params_is_array, nullptr);
    if (!_cur_scope->add_symbol(symbol)) {
        // 重复定义
        _error_out << func_def.ident->lineno << " " << "b" << std::endl; 
        VISITOR_ERROR = true;
        return;
    }

    // 输出符号
    std::string symbol_type_str = (func_def.func_type->is_int) ? _symbol_type_to_string(SymbolType::IntFunc) : 
                                                                _symbol_type_to_string(SymbolType::VoidFunc);
    _symbol_out << _cur_scope->get_table_id() << " " << symbol->name << " " << symbol_type_str << std::endl;

    // 进入新的作用域
    _cur_table_id++;
    _cur_scope = _cur_scope->push_scope(_cur_table_id);


    // 处理参数获取形参的symbol
    std::vector<std::shared_ptr<VarSymbol>> symbols;
    if (func_def.func_fparams) symbols = _visit_func_fparams(*func_def.func_fparams);

    // 处理参数，并创建Function
    BasicBlockPtr second_block;
    if (!VISITOR_ERROR) {
        auto func_return_type = func_def.func_type->is_int ? _ir_module->Context()->GetInt32Ty() :
                                                            _ir_module->Context()->GetVoidTy();
        if (func_def.func_fparams) {
            auto context = _ir_module->Context();
            std::vector<ArgumentPtr> args;

            // 为每个symbol创建Argument
            for (auto& symbol : symbols) {
                if (symbol->is_array) {
                    auto arg = Argument::New(context->GetPointerType(context->GetInt32Ty()), symbol->name);
                    args.push_back(arg);
                    symbol->value = arg;
                } else {
                    auto arg = Argument::New(context->GetInt32Ty(), symbol->name);
                    args.push_back(arg);
                    symbol->value = arg;
                }
                
            }

            // 创建函数
            _cur_func = Function::New(func_return_type, func_def.ident->name, args);
            _cur_block = _cur_func->NewBasicBlock(); // 第一个基本块用于分配参数内存和局部变量内存

            // 给每一个参数分配内存
            for (auto& symbol : symbols) {
                if (symbol->is_array) {
                    // 数组退化为指针
                    auto alloca_inst = AllocaInst::New(_ir_module->Context()->GetPointerType(_ir_module->Context()->GetInt32Ty()));
                    _cur_block->InsertInstruction(alloca_inst);
                    auto store_inst = StoreInst::New(symbol->value, alloca_inst);
                    _cur_block->InsertInstruction(store_inst);
                    symbol->value = alloca_inst;
                } else {
                    auto alloca_inst = AllocaInst::New(_ir_module->Context()->GetInt32Ty());
                    _cur_block->InsertInstruction(alloca_inst);
                    auto store_inst = StoreInst::New(symbol->value, alloca_inst);
                    _cur_block->InsertInstruction(store_inst);
                    symbol->value = alloca_inst;
                }
            }

        } else {
            // 创建没有参数的函数
            _cur_func = Function::New(func_return_type, func_def.ident->name);
            _cur_block = _cur_func->NewBasicBlock();
        }

        second_block = _cur_func->NewBasicBlock(); // 第二个基本块用于处理函数体
        _cur_block = second_block;
        symbol->value = _cur_func;
        // 把函数加入模块
        _ir_module->AddFunction(_cur_func);
    }

    if (func_def.block) {
        // 进入函数体
        _is_void_func = !func_def.func_type->is_int;
        _visit_block(*func_def.block, func_def.func_type->is_int);

        // 给void函数末尾加上可能缺失的return语句 （int函数语义要求一定有return，void可能没有）
        if (!VISITOR_ERROR && _is_void_func) {
            auto ret_inst = ReturnInst::New(_ir_module->Context());
            _cur_block->InsertInstruction(ret_inst);
        }

        // 离开函数体
        _is_void_func = false;
    }

    // 退出当前作用域
    _cur_scope = _cur_scope->pop_scope();

    // 给第一个基本块添加进入第二个基本块的指令
    if (!VISITOR_ERROR) {
        auto jump_inst = JumpInst::New(second_block);
        if (OPTIMIZE) {
            jump_inst->setNotToMips();
        }
        auto first_block = *_cur_func->BasicBlockBegin();
        first_block->InsertInstruction(jump_inst);
    }
}

void Visitor::_visit_main_func_def(const MainFuncDef& main_func_def) {
    // MainFuncDef → 'int' 'main' '(' ')' Block
    
    BasicBlockPtr second_block;
    if (!VISITOR_ERROR) {
        // 创建函数
        _cur_func = Function::New(_ir_module->Context()->GetInt32Ty(), "main");
        _ir_module->AddMainFunction(_cur_func);

        // 创建函数第一个基本块
        _cur_block = _cur_func->NewBasicBlock();

        // 创建第二个基本块
        second_block = _cur_func->NewBasicBlock();
        _cur_block = second_block;
    }

    // 进入新的作用域
    _cur_table_id++;
    _cur_scope = _cur_scope->push_scope(_cur_table_id);

    if (main_func_def.block) {
        // 进入函数体
        _visit_block(*main_func_def.block, true);
        // 离开函数体
    }

    // 退出当前作用域
    _cur_scope = _cur_scope->pop_scope();

    // 给第一个基本块添加进入第二个基本块的指令
    if (!VISITOR_ERROR) {
        auto jump_inst = JumpInst::New(second_block);
        if (OPTIMIZE) {
            jump_inst->setNotToMips();
        }
        auto first_block = *_cur_func->BasicBlockBegin();
        first_block->InsertInstruction(jump_inst);
    }
}

void Visitor::_visit_func_type(const FuncType& func_type) {
    // FuncType → 'int' | 'void'
    // 无需处理
}

std::vector<std::shared_ptr<VarSymbol>> Visitor::_visit_func_fparams(const FuncFParams& func_fparams) {
    // FuncFParams → FuncFParam { ',' FuncFParam }
    std::vector<std::shared_ptr<VarSymbol>> symbols;
    for (const auto& func_fparam : func_fparams.func_fparams) {
        if (func_fparam) {
            auto symbol = _visit_func_fparam(*func_fparam);
            symbols.push_back(symbol);
        }
    }
    return symbols;
}

std::shared_ptr<VarSymbol> Visitor::_visit_func_fparam(const FuncFParam& func_fparam) {
    // FuncFParam → BType Ident ['[' ']']
    if (func_fparam.ident) {
        _visit_ident(*func_fparam.ident);
    }

    // 处理符号表
    bool is_array = func_fparam.is_array;
    bool is_const = false;
    bool is_static = false;
    int array_size = -1;
    auto symbol = std::make_shared<VarSymbol>(func_fparam.ident->name, func_fparam.ident->lineno, _cur_table_id, is_array, is_const, is_static, array_size, nullptr);
    if (!_cur_scope->add_symbol(symbol)) {
        // 重复定义
        _error_out << func_fparam.ident->lineno << " " << "b:重复定义" << std::endl; 
        VISITOR_ERROR = true;
    }

    // 输出符号
    std::string symbol_type_str = (is_array) ? _symbol_type_to_string(SymbolType::IntArray) : 
                                                _symbol_type_to_string(SymbolType::Int);
    _symbol_out << _cur_scope->get_table_id() << " " << symbol->name << " " << symbol_type_str << std::endl;

    return symbol;
}

void Visitor::_visit_block(const Block& block, bool is_int_func) {
    // Block → '{' { BlockItem } '}'
    if (block.block_items.size() == 0 && is_int_func) {
        // int函数体的最后一个语句必须是return语句，错误g
        _error_out << block.lineno_of_end << " " << "g:有返回值的函数的最后一个语句必须是return语句" << std::endl; 
        VISITOR_ERROR = true;
    }
    for (int i = 0; i < block.block_items.size(); i++) {
        if (block.block_items[i]) {
            auto& block_item = *block.block_items[i];
            if (_is_void_func) {
                // void函数体中不能有return exp语句，错误f
                if (std::holds_alternative<Stmt>(block_item)) {
                    auto& stmt = std::get<Stmt>(block_item);
                    if (std::holds_alternative<ReturnStmt>(stmt)) {
                        auto& return_stmt = std::get<ReturnStmt>(stmt);
                        if (return_stmt.exp) {
                            _error_out << return_stmt.lineno << " " << "f:无返回值函数存在不匹配的return语句" << std::endl; 
                            VISITOR_ERROR = true;
                        }
                    }
                }
            }
            if (is_int_func && i == block.block_items.size() - 1) {
                // int函数体的最后一个语句必须是return语句，错误g
                bool is_return = false;
                if (std::holds_alternative<Stmt>(block_item)) {
                    auto& stmt = std::get<Stmt>(block_item);
                    if (std::holds_alternative<ReturnStmt>(stmt)) {
                        is_return = true;
                    }
                }
                if (!is_return) {
                    _error_out << block.lineno_of_end << " " << "g:有返回值的函数的最后一个语句必须是return语句" << std::endl; 
                    VISITOR_ERROR = true;
                }
            } 
            if (_loop_num == 0) {
                // 非循环体中不能有break或continue语句，错误m
                if (std::holds_alternative<Stmt>(block_item)) {
                    auto& stmt = std::get<Stmt>(block_item);
                    if (std::holds_alternative<BreakStmt>(stmt)) {
                        auto& break_stmt = std::get<BreakStmt>(stmt);
                        _error_out << break_stmt.lineno << " " << "m:非循环体中不能有break语句" << std::endl; 
                        VISITOR_ERROR = true;
                    } else if (std::holds_alternative<ContinueStmt>(stmt)) {
                        auto& continue_stmt = std::get<ContinueStmt>(stmt);
                        _error_out << continue_stmt.lineno << " " << "m:非循环体中不能有continue语句" << std::endl; 
                        VISITOR_ERROR = true;
                    }
                }
            }
            _visit_block_item(block_item);
        }
    }
}

void Visitor::_visit_block_item(const BlockItem& block_item) {
    if (std::holds_alternative<Decl>(block_item)) {
        _visit_decl(std::get<Decl>(block_item));
    } else if (std::holds_alternative<Stmt>(block_item)) {
        _visit_stmt(std::get<Stmt>(block_item));
    }
}

void Visitor::_visit_stmt(const Stmt& stmt) {
    if (std::holds_alternative<LValStmt>(stmt)) {
        _visit_lval_stmt(std::get<LValStmt>(stmt));
    } else if (std::holds_alternative<ExpStmt>(stmt)) {
        _visit_exp_stmt(std::get<ExpStmt>(stmt));
    } else if (std::holds_alternative<BlockStmt>(stmt)) {
        _visit_block_stmt(std::get<BlockStmt>(stmt));
    } else if (std::holds_alternative<IfStmt>(stmt)) {
        _visit_if_stmt(std::get<IfStmt>(stmt));
    } else if (std::holds_alternative<ForCondStmt>(stmt)) {
        _visit_for_cond_stmt(std::get<ForCondStmt>(stmt));
    } else if (std::holds_alternative<BreakStmt>(stmt)) {
        _visit_break_stmt(std::get<BreakStmt>(stmt));
    } else if (std::holds_alternative<ContinueStmt>(stmt)) {
        _visit_continue_stmt(std::get<ContinueStmt>(stmt));
    } else if (std::holds_alternative<ReturnStmt>(stmt)) {
        _visit_return_stmt(std::get<ReturnStmt>(stmt));
    } else if (std::holds_alternative<PrintfStmt>(stmt)) {
        _visit_printf_stmt(std::get<PrintfStmt>(stmt));
    }
}

void Visitor::_visit_lval_stmt(const LValStmt& lval_stmt) {
    // LValStmt → LVal '=' Exp ';'
    if (lval_stmt.lval) {
        // 不能改变常量的值，h错误
        auto symbol = _cur_scope->get_symbol(lval_stmt.lval->ident->name);
        if (symbol && !symbol->is_func) {
            auto var_symbol = std::dynamic_pointer_cast<VarSymbol>(symbol);
            if (var_symbol && var_symbol->is_const) {
                _error_out << lval_stmt.lval->ident->lineno << " " << "h:不能改变常量的值" << std::endl; 
                VISITOR_ERROR = true;
                return;
            }
        }
    }
    ValuePtr addr, rvalue;
    if (lval_stmt.exp) {
        rvalue = _visit_exp(*lval_stmt.exp);
    } 
    if (lval_stmt.lval) {
        addr = _visit_lval_forAssignment(*lval_stmt.lval);
    }
    if (!VISITOR_ERROR && addr && rvalue) {
        auto store_inst = StoreInst::New(rvalue, addr);
        _cur_block->InsertInstruction(store_inst);
    }
}

void Visitor::_visit_exp_stmt(const ExpStmt& exp_stmt) {
    // ExpStmt → [Exp] ';'
    if (exp_stmt.exp) {
        _visit_exp(*exp_stmt.exp);
    }
}

void Visitor::_visit_block_stmt(const BlockStmt& block_stmt) {
    // BlockStmt → Block
    // 进入新的作用域
    _cur_table_id++;
    _cur_scope = _cur_scope->push_scope(_cur_table_id);
    if (block_stmt.block) {
        _visit_block(*block_stmt.block, false);
    }
    // 退出当前作用域
    _cur_scope = _cur_scope->pop_scope();
}

void Visitor::_visit_if_stmt(const IfStmt& if_stmt) {
    // IfStmt → 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
    BasicBlockPtr if_true_block, if_false_block, if_end_block;
    if (!VISITOR_ERROR) {
        if_true_block = _cur_func->NewBasicBlock();
        if_false_block = _cur_func->NewBasicBlock();
        if_end_block = (if_stmt.else_stmt) ? _cur_func->NewBasicBlock() : if_false_block;
    }

    BranchInstPtr branch_inst;
    if (if_stmt.cond) {
        auto cond_value = _visit_cond(*if_stmt.cond);
        if (!VISITOR_ERROR) {
            branch_inst = BranchInst::New(cond_value, nullptr, nullptr);
            _cur_block->InsertInstruction(branch_inst);
        }
    }
    if (if_stmt.stmt) {
        if (!VISITOR_ERROR) {
            _cur_block = if_true_block;
            branch_inst->SetTrueBlock(_cur_block);
        }
        _visit_stmt(*if_stmt.stmt);
        if (!VISITOR_ERROR) {
            auto jump = JumpInst::New(if_end_block);
            _cur_block->InsertInstruction(jump);
        } 
    }
    if (if_stmt.else_stmt) {
        if (!VISITOR_ERROR) {
            _cur_block = if_false_block;
            branch_inst->SetFalseBlock(_cur_block);
        }
        _visit_stmt(*if_stmt.else_stmt);
        if (!VISITOR_ERROR) {
            auto jump = JumpInst::New(if_end_block);
            _cur_block->InsertInstruction(jump);
        }
    } else {
        if (!VISITOR_ERROR) {
            branch_inst->SetFalseBlock(if_end_block);
        }
    }
    if (!VISITOR_ERROR) {
        _cur_block = if_end_block;
    }
}

void Visitor::_visit_for_cond_stmt(const ForCondStmt& for_cond_stmt) {
    // ForCondStmt → 'for' '(' [ForStmt] ';' [Cond] ';' [ForStmt] ')' Stmt

    if (for_cond_stmt.for_stmt1) {
        //  这部分inst会在_cur_block中生成，由_visit_for_stmt处理
        _visit_for_stmt(*for_cond_stmt.for_stmt1);
    }

    BasicBlockPtr for_cond, for_inc, for_body, for_end;
    if (!VISITOR_ERROR) {
        if (for_cond_stmt.cond) {
            for_cond = _cur_func->NewBasicBlock();
        }
        if (for_cond_stmt.for_stmt2) {
            for_inc = _cur_func->NewBasicBlock();
        }
        for_body = _cur_func->NewBasicBlock();
        for_end = _cur_func->NewBasicBlock();
    }
    
    // 解析cond
    if (for_cond_stmt.cond) {
        // 进入for_cond
        if (!VISITOR_ERROR) {
            auto jump = JumpInst::New(for_cond);
            _cur_block->InsertInstruction(jump);
            _cur_block = for_cond;
        }

        auto cond_val =  _visit_cond(*for_cond_stmt.cond);

        if (!VISITOR_ERROR) {
            auto cond_branch = BranchInst::New(cond_val, for_body, for_end);
            _cur_block->InsertInstruction(cond_branch);
        }
    } else {
        // 无条件循环，直接进入for_body
        if (!VISITOR_ERROR) {
            auto jump = JumpInst::New(for_body);
            _cur_block->InsertInstruction(jump);
        }
    }

    // 解析inc
    if (for_cond_stmt.for_stmt2) {
        if (!VISITOR_ERROR) _cur_block = for_inc;

        _visit_for_stmt(*for_cond_stmt.for_stmt2);

        if (!VISITOR_ERROR) {
            auto jump = JumpInst::New((for_cond_stmt.cond)? for_cond : for_body);
            _cur_block->InsertInstruction(jump);
        }
    }

    // 解析body
    if (for_cond_stmt.stmt) {
        _cur_block = for_body;

        // 进入循环体前，记录break和continue的位置
        if (!VISITOR_ERROR) {
            _break_targets.push(for_end);
            _continue_targets.push((for_cond_stmt.for_stmt2)? for_inc : (for_cond_stmt.cond) ? for_cond : for_body);
        }
        
        if (std::holds_alternative<BlockStmt>(*for_cond_stmt.stmt)) {
            auto& block_stmt = std::get<BlockStmt>(*for_cond_stmt.stmt);
            auto& block = *block_stmt.block;
            // 进入新的作用域
            _cur_table_id++;
            _cur_scope = _cur_scope->push_scope(_cur_table_id);

            // 进入循环体
            _loop_num++;
            _visit_block(block, false);
            // 离开循环体
            _loop_num--;

            // 退出当前作用域
            _cur_scope = _cur_scope->pop_scope();
        } else {
            _visit_stmt(*for_cond_stmt.stmt);
        }
        
        if (!VISITOR_ERROR) {
            // 离开循环体时出栈
            _break_targets.pop();
            _continue_targets.pop();

            auto jump = JumpInst::New(
                (for_cond_stmt.for_stmt2)? for_inc : 
                (for_cond_stmt.cond)? for_cond : 
                for_body);
            _cur_block->InsertInstruction(jump);
        }
    }

    if (!VISITOR_ERROR) {
        _cur_block = for_end;
    }
}

void Visitor::_visit_break_stmt(const BreakStmt& break_stmt) {
    // BreakStmt → 'break' ';'
    if (!VISITOR_ERROR) {
        auto jump = JumpInst::New(_break_targets.top());
        _cur_block->InsertInstruction(jump);
    }
}

void Visitor::_visit_continue_stmt(const ContinueStmt& continue_stmt) {
    // ContinueStmt → 'continue' ';'
    if (!VISITOR_ERROR) {
        auto jump = JumpInst::New(_continue_targets.top());
        _cur_block->InsertInstruction(jump);
    }
}

void Visitor::_visit_return_stmt(const ReturnStmt& return_stmt) {
    // ReturnStmt → 'return' [Exp] ';'
    if (VISITOR_ERROR) {
        return;
    }
    if (return_stmt.exp) {
        auto val = _visit_exp(*return_stmt.exp);
        if (!VISITOR_ERROR) {
            auto ret_inst = ReturnInst::New(val);
            _cur_block->InsertInstruction(ret_inst);
        }
    } else {
        auto ret_inst = ReturnInst::New(_ir_module->Context());
        _cur_block->InsertInstruction(ret_inst);
    }
}

void Visitor::_visit_printf_stmt(const PrintfStmt& printf_stmt) {
    // PrintfStmt → 'printf''('StringConst {','Exp}')'';'
    /*
        需要转化成调用以下函数的call语句
        declare void @putint(i32)      ; 输出一个整数
        declare void @putch(i32)       ; 输出一个字符
    */
    // 若字符串中格式字符数量与实参数量不匹配，i错误
    auto& str = printf_stmt.string_const;
    int pos = 0, count = 0;
    while ((pos = str.find("%d", pos)) != std::string::npos) {
        count++;
        pos += 2; // 跳过已找到的"%d"
    }
    if (count != printf_stmt.exps.size()) {
        _error_out << printf_stmt.lineno << " " << "l:格式字符数量与实参数量不匹配" << std::endl; 
        VISITOR_ERROR = true;
    }
    
    // 遍历字符串，生成输出语句
    std::vector<ValuePtr> exps;
    for (const auto& exp : printf_stmt.exps) {
        if (exp) {
            exps.push_back(_visit_exp(*exp));
        }
    }

    if (!VISITOR_ERROR) {
        int index = 0;
        int size = str.size() - 1; // 忽略首尾的双引号
        for (int i = 1; i < size; i++) { 
            if (str[i] == '%' && i + 1 < size && str[i+1] == 'd') {
                // 找到格式字符"%d"
                i++;
                auto val = exps[index++];
                auto putint_call = CallInst::New(_putint, {val});
                _cur_block->InsertInstruction(putint_call);
            } else if (str[i] == '\\' && i + 1 < size && str[i+1] == 'n') {
                // 找到转义字符"\n"
                i++;
                auto const_int = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), '\n');
                auto putch_call = CallInst::New(_putch, {const_int});
                _cur_block->InsertInstruction(putch_call);
            } else {
                // 找到普通字符
                auto const_int = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), str[i]);
                auto putch_call = CallInst::New(_putch, {const_int});
                _cur_block->InsertInstruction(putch_call);
            }
        }
    }
}

void Visitor::_visit_for_stmt(const ForStmt& for_stmt) {
    // ForStmt → LVal '=' Exp { ',' LVal '=' Exp }
    for (int i = 0; i < for_stmt.lvals.size(); i++) {
        auto& lval = for_stmt.lvals[i];
        auto& exp = for_stmt.exps[i];
        ValuePtr addr, rvalue;
        if (lval) {
            // 不能改变常量的值，h错误
            auto symbol = _cur_scope->get_symbol(lval->ident->name);
            if (symbol && !symbol->is_func) {
                auto var_symbol = std::dynamic_pointer_cast<VarSymbol>(symbol);
                if (var_symbol && var_symbol->is_const) {
                    _error_out << lval->ident->lineno << " " << "h:不能改变常量的值" << std::endl; 
                    VISITOR_ERROR = true;
                }
            }
        }
        if (exp) {
            rvalue = _visit_exp(*exp);
        }
        if (lval) {
            addr = _visit_lval_forAssignment(*lval);
        }
        if (!VISITOR_ERROR && addr && rvalue) {
            auto store_inst = StoreInst::New(rvalue, addr);
            _cur_block->InsertInstruction(store_inst);
        }
    }
}

ValuePtr Visitor::_visit_exp(const Exp& exp) {
    // Exp → AddExp
    if (exp.add_exp) {
        return _visit_add_exp(*exp.add_exp);
    }
    return nullptr;
}

ValuePtr Visitor::_visit_cond(const Cond& cond) {
    // Cond → LOrExp
    if (cond.lor_exp) {
        return _visit_lor_exp(*cond.lor_exp);
    }
    return nullptr;
}

ValuePtr Visitor::_visit_lval_forAssignment(const LVal& lval) {
    // LVal → Ident [ '[' Exp ']' ]
    // 返回地址
    std::shared_ptr<Symbol> symbol;
    if (lval.ident) {
        // 符号表中不存在，c错误
        symbol = _cur_scope->get_symbol(lval.ident->name);
        if (!symbol || symbol->is_func) {
            _error_out << lval.ident->lineno << " " << "c:名字未定义" << std::endl; 
            VISITOR_ERROR = true;
        }
        _visit_ident(*lval.ident);
    }

    ValuePtr index;
    if (lval.exp) {
        index = _visit_exp(*lval.exp);
    }
    if (!VISITOR_ERROR) {
        if (lval.exp) {
            // 是数组，返回Gep计算后的地址
            auto var_symbol = std::dynamic_pointer_cast<VarSymbol>(symbol);
            auto addr = symbol->value;
            auto load_addr = addr;
            auto addr_type = addr->GetType()->As<PointerType>();
            if (addr_type->ElementType()->IsPointerTy()) {
                // 二维指针，需要先load出一维指针的值
                load_addr = LoadInst::New(addr);
                _cur_block->InsertInstruction(load_addr->As<Instruction>());
            }
            auto gep = GepInst::New(load_addr, index);
            _cur_block->InsertInstruction(gep);
            return gep;
        } else {
            // 不是数组，返回符号表中变量的地址
            auto var_symbol = std::dynamic_pointer_cast<VarSymbol>(symbol);
            return symbol->value;
        }
    } 

    return nullptr;
}

ValuePtr Visitor::_visit_lval_forEvaluation(const LVal& lval) {
    // LVal → Ident [ '[' Exp ']' ]
    // 返回值
    std::shared_ptr<Symbol> symbol; 
    if (lval.ident) {
        // 符号表中不存在，c错误
        symbol = _cur_scope->get_symbol(lval.ident->name);
        if (!symbol || symbol->is_func) {
            _error_out << lval.ident->lineno << " " << "c:名字未定义" << std::endl; 
            VISITOR_ERROR = true;
        }
        _visit_ident(*lval.ident);
    }

    ValuePtr index;
    if (lval.exp) {
        index = _visit_exp(*lval.exp);
    }
    if (!VISITOR_ERROR) {
        if (lval.exp) {
            // 是数组，返回Gep计算后Load的值
            auto var_symbol = std::dynamic_pointer_cast<VarSymbol>(symbol);
            auto addr = symbol->value;

            if (var_symbol->is_const && index->GetValueType() == ValueType::ConstantIntTy) {
                // 全局常量数组，且下标是常量，可以直接返回一个常值
                auto global_array = var_symbol->value->As<GlobalVariable>();
                auto array_val = global_array->GetInitialArray();
                int int_index = index->As<ConstantInt>()->GetIntValue();
                auto int_val = array_val->GetConstantInts()[int_index];
                return ConstantInt::New(_ir_module->Context()->GetInt32Ty(), int_val->GetIntValue());
            }

            // 如果addr_type是一维指针，则直接作为gep参数。如果是二维指针，则需要先load出一维指针的值
            auto load_addr = addr;
            auto addr_type = addr->GetType()->As<PointerType>();
            if (addr_type->ElementType()->IsPointerTy()) {
                load_addr = LoadInst::New(addr);
                _cur_block->InsertInstruction(load_addr->As<Instruction>());
            }
            auto gep = GepInst::New(load_addr, index);
            _cur_block->InsertInstruction(gep);
            auto load = LoadInst::New(gep);
            _cur_block->InsertInstruction(load);
            return load;
        } else {
            auto var_symbol = std::dynamic_pointer_cast<VarSymbol>(symbol);

            if (var_symbol->is_array) {
                // 不带索引下标，但是数组，说明是函数调用时传递数组参数，返回数组地址
                        
                // 若为函数参数数组，则value为i32**，用load获取数组地址
                if (var_symbol->value->GetType()->As<PointerType>()->ElementType()->IsPointerTy()) {
                    auto load = LoadInst::New(var_symbol->value);
                    _cur_block->InsertInstruction(load);
                    return load;
                }
                // 若为局部或全局数组，则value为[n x i32]*，用gep计算出数组地址
                auto base_addr = GepInst::New(var_symbol->value, ConstantInt::New(_ir_module->Context()->GetInt32Ty(), 0));
                _cur_block->InsertInstruction(base_addr);
                return base_addr;
            } 

            // 不是数组，返回符号表中变量的值

            if (var_symbol->is_const) {
                // 全局常量（常量不会被赋值，所以只会是作为右值被使用，可以直接返回一个常值）
                // 全局常量的Value是一个GlobalVariable
                auto global_var = var_symbol->value->As<GlobalVariable>();
                // GlobalVariable的初始值是常值
                return ConstantInt::New(_ir_module->Context()->GetInt32Ty(), global_var->GetInitialInt()->GetIntValue());
            }

            auto load = LoadInst::New(var_symbol->value);
            _cur_block->InsertInstruction(load);
            return load;
        }
    }
    return nullptr;
}

ValuePtr Visitor::_visit_primary_exp(const PrimaryExp& primary_exp) {
    // PrimaryExp → '(' Exp ')' | LVal | Number
    auto& content = *primary_exp.content;
    if (std::holds_alternative<Exp>(content)) {
        return _visit_exp(std::get<Exp>(content));
    } else if (std::holds_alternative<LVal>(content)) {
        return _visit_lval_forEvaluation(std::get<LVal>(content));
    } else if (std::holds_alternative<Number>(content)) {
        return _visit_number(std::get<Number>(content));
    }
    return nullptr;
}

ValuePtr Visitor::_visit_number(const Number& number) {
    // Number → ConstInt
    if (VISITOR_ERROR) {
        return nullptr;
    }
    auto val = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), std::stoi(number.int_const));
    return val;
}

ValuePtr Visitor::_visit_unary_exp(const UnaryExp& unary_exp) {
     // UnaryExp → PrimaryUnaryExp | FuncCallExp | UnaryOpExp
    if (std::holds_alternative<PrimaryUnaryExp>(unary_exp)) {
        return _visit_primary_unary_exp(std::get<PrimaryUnaryExp>(unary_exp));
    } else if (std::holds_alternative<FuncCallExp>(unary_exp)) {
        return _visit_func_call_exp(std::get<FuncCallExp>(unary_exp));
    } else if (std::holds_alternative<UnaryOpExp>(unary_exp)) {
        return _visit_unary_op_exp(std::get<UnaryOpExp>(unary_exp));
    }
    return nullptr;
} 

ValuePtr Visitor::_visit_primary_unary_exp(const PrimaryUnaryExp& primary_unary_exp) {
    // PrimaryUnaryExp → PrimaryExp 
    if (primary_unary_exp.primary_exp) {
        return _visit_primary_exp(*primary_unary_exp.primary_exp);
    }
    return nullptr;
}

ValuePtr Visitor::_visit_func_call_exp(const FuncCallExp& func_call_exp) {
    // FuncCallExp → Ident '(' [FuncRParams] ')'
    std::shared_ptr<Symbol> symbol;
    if (func_call_exp.ident) {
        symbol = _cur_scope->get_symbol(func_call_exp.ident->name);
        if (!symbol || !symbol->is_func) {
            // 符号表中不存在，c错误
            _error_out << func_call_exp.ident->lineno << " " << "c:名字未定义" << std::endl; 
            VISITOR_ERROR = true;
        } else {
            // 实参数量与形参数量不匹配，d错误
            auto func_symbol = std::dynamic_pointer_cast<FuncSymbol>(symbol);
            int size = (func_call_exp.func_rparams) ? func_call_exp.func_rparams->exps.size() : 0;
            if (func_symbol->params_is_array.size() != size) {
                _error_out << func_call_exp.ident->lineno << " " << "d:实参与形参数量不匹配" << std::endl; 
                VISITOR_ERROR = true;
            } else {
                // 实参类型与形参类型不匹配，e错误
                for (int i = 0; i < size; i++) {
                    auto& exp = func_call_exp.func_rparams->exps[i];
                    if (func_symbol->params_is_array[i]) {
                        // 数组类型参数必须是整型数组
                        if (!_is_exp_is_array(*exp)) {
                            _error_out << func_call_exp.ident->lineno << " " << "e:函数参数类型不匹配" << std::endl;
                            VISITOR_ERROR = true;
                        }
                    } else {
                        // 非数组类型参数必须是整型
                        if (_is_exp_is_array(*exp)) {
                            _error_out << func_call_exp.ident->lineno << " " << "e:函数参数类型不匹配" << std::endl;
                            VISITOR_ERROR = true;
                        }
                    }
                }
            }
        }
        _visit_ident(*func_call_exp.ident);
    }
    if (func_call_exp.func_rparams) {
        // 有参数，先处理参数
        auto values = _visit_func_rparams(*func_call_exp.func_rparams);
        if (!VISITOR_ERROR) {
            auto call = CallInst::New(static_cast<FunctionPtr>(symbol->value), values);
            _cur_block->InsertInstruction(call);
            return call;
        }
    } else {
        // 无参数，直接调用
        if (!VISITOR_ERROR) {
            auto call = CallInst::New(static_cast<FunctionPtr>(symbol->value));
            _cur_block->InsertInstruction(call);
            return call;
        }
    }
    return nullptr;
}

bool Visitor::_is_exp_is_array(const Exp& exp) {
    // 判断一个Exp是否是array
    if (exp.add_exp) {
        auto& add_exp = exp.add_exp;
        if (add_exp->mul_exps.size() == 1) {
            auto& mul_exp = add_exp->mul_exps[0];
            if (mul_exp) {
                if (mul_exp->unary_exps.size() == 1) {
                    auto& unary_exp = mul_exp->unary_exps[0];
                    if (unary_exp) {
                        // using UnaryExp = std::variant<PrimaryUnaryExp, FuncCallExp, UnaryOpExp>;
                        // 数组作为实参，只会是identifier，所以只需要判断是否是PrimaryUnaryExp
                        if (std::holds_alternative<PrimaryUnaryExp>(*unary_exp)) {
                            auto& primary_unary_exp = std::get<PrimaryUnaryExp>(*unary_exp);
                            if (primary_unary_exp.primary_exp) {
                                auto& primary_exp = primary_unary_exp.primary_exp;
                                if (primary_exp->content) {
                                    auto& content = primary_exp->content;
                                    if (std::holds_alternative<LVal>(*content)) {
                                        auto& lval = std::get<LVal>(*content);
                                        if (lval.ident && !lval.exp) {
                                            auto symbol = _cur_scope->get_symbol(lval.ident->name);
                                            if (symbol && !symbol->is_func) {
                                                auto var_symbol = std::dynamic_pointer_cast<VarSymbol>(symbol);
                                                if (var_symbol && var_symbol->is_array) {
                                                    return true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        } 
                    }
                }
            }
        }
    }
    return false;
}

ValuePtr Visitor::_visit_unary_op_exp(const UnaryOpExp& unary_op_exp) {
    // UnaryOpExp → UnaryOp UnaryExp
    UnaryOpType op_type;
    if (unary_op_exp.unary_op) {
        op_type = _visit_unary_op(*unary_op_exp.unary_op);
    } 
    if (unary_op_exp.unary_exp) { 
        auto val = _visit_unary_exp(*unary_op_exp.unary_exp);
        if (!VISITOR_ERROR) {
            if (val->GetValueType() == ValueType::ConstantIntTy) {
                // 常量操作，直接计算
                int int_val = val->As<ConstantInt>()->GetIntValue();
                if (op_type == UnaryOpType::Neg) {
                    return ConstantInt::New(_ir_module->Context()->GetInt32Ty(), -int_val);
                } else if (op_type == UnaryOpType::Pos) {
                    return ConstantInt::New(_ir_module->Context()->GetInt32Ty(), int_val);
                } else if (op_type == UnaryOpType::Not) {
                    return ConstantInt::New(_ir_module->Context()->GetInt1Ty(), !int_val);
                }
            } else {
                // 非常量操作，生成指令
                if (op_type == UnaryOpType::Not) {
                    // 逻辑非，!exp 等价于 (exp == 0)
                    auto is_zero = CompareInstruction::New(CompareOpType::Equal, val, ConstantInt::New(_ir_module->Context()->GetInt32Ty(), 0));
                    _cur_block->InsertInstruction(is_zero);
                    return is_zero;
                }
                auto unary_operator = UnaryOperator::New(op_type, val);
                _cur_block->InsertInstruction(unary_operator);
                return unary_operator;
            } 
        }
    }
    return nullptr;
}

UnaryOpType Visitor::_visit_unary_op(const UnaryOp& unary_op) {
    // UnaryOp → '+' | '-' | '!'
    if (unary_op.op == "+") {
        return UnaryOpType::Pos;
    } else if (unary_op.op == "-") {
        return UnaryOpType::Neg; 
    } else if (unary_op.op == "!") {
        return UnaryOpType::Not;
    }
    return UnaryOpType::Pos;
}

std::vector<ValuePtr> Visitor::_visit_func_rparams(const FuncRParams& func_rparams) {
    // FuncRParams → Exp { ',' Exp }
    std::vector<ValuePtr> values;
    for (const auto& exp : func_rparams.exps) {
        if (exp) {
            auto val = _visit_exp(*exp);
            values.push_back(val);
        }
    }
    return values;
}

ValuePtr Visitor::_visit_mul_exp(const MulExp& mul_exp) {
    // MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
    // 左递归转变为循环

    // 初始化，访问第一个unary_exp
    auto result_val = _visit_unary_exp(*mul_exp.unary_exps[0]);
    if (!VISITOR_ERROR) result_val = _zext_if_i1(result_val);

    // 循环遍历剩余的unary_exp
    for (int i = 1; i < mul_exp.unary_exps.size(); i++) {
        if (!VISITOR_ERROR) result_val = _zext_if_i1(result_val);

        auto& unary_exp = mul_exp.unary_exps[i];
        auto val = _visit_unary_exp(*unary_exp);

        if (!VISITOR_ERROR) {
            val = _zext_if_i1(val);
            if (unary_exp) {
                auto str_op = mul_exp.ops[i-1];

                if (result_val->GetValueType() == ValueType::ConstantIntTy && 
                    val->GetValueType() == ValueType::ConstantIntTy) {
                    // 常量直接计算
                    int l = result_val->As<ConstantInt>()->GetIntValue();
                    int r = val->As<ConstantInt>()->GetIntValue();
                    int int_result = (str_op == "*") ? l * r : 
                                    (str_op == "/") ? l / r : 
                                    l % r;
                    result_val = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), int_result);
                    continue;
                } 

                if (OPTIMIZE) {
                    // 代数优化
                    if (val->Is<ConstantInt>() && 
                        val->As<ConstantInt>()->GetIntValue() == 0 && 
                        str_op == "*" ) {
                        // x * 0，结果为0
                        result_val = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), 0);
                        continue;
                    } 
                    if (result_val->Is<ConstantInt>() && 
                        result_val->As<ConstantInt>()->GetIntValue() == 0) {
                        // 0 * x, 0 / x, 0 % x，结果为0
                        result_val = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), 0);
                        continue;
                    } 
                    if (val->Is<ConstantInt>() && 
                        val->As<ConstantInt>()->GetIntValue() == 1 &&
                        (str_op == "*" || str_op == "/")) {
                        // x * 1, x / 1，结果为x
                        continue; // 不改变result_val
                    }
                    if (result_val->Is<ConstantInt>() && 
                        result_val->As<ConstantInt>()->GetIntValue() == 1 &&
                        str_op == "*") {
                        // 1 * x，结果为x
                        result_val = val;
                        continue;
                    }
                }

                // 非常量，生成指令
                auto op_type = (str_op == "*") ? BinaryOpType::Mul : (str_op == "/") ? BinaryOpType::Div : BinaryOpType::Mod;
                result_val = BinaryOperator::New(op_type, result_val, val);
                _cur_block->InsertInstruction(result_val->As<BinaryOperator>());
            }
        }
    }
    if (VISITOR_ERROR) return nullptr;
    return result_val;
}

ValuePtr Visitor::_visit_add_exp(const AddExp& add_exp) {
    // AddExp → MulExp | AddExp ('+' | '−') MulExp
    // 左递归转变为循环

    // 初始化，访问第一个mul_exp
    auto result_val = _visit_mul_exp(*add_exp.mul_exps[0]);

    // 循环遍历剩余的mul_exp
    for (int i = 1; i < add_exp.mul_exps.size(); i++) {
        if (!VISITOR_ERROR) result_val = _zext_if_i1(result_val);

        auto& mul_exp = add_exp.mul_exps[i];
        auto val = _visit_mul_exp(*mul_exp);

        if (!VISITOR_ERROR) {
            val = _zext_if_i1(val);
            if (mul_exp) {
                auto str_op = add_exp.ops[i-1];

                if (result_val->GetValueType() == ValueType::ConstantIntTy && 
                    val->GetValueType() == ValueType::ConstantIntTy) {
                    // 常量直接计算
                    int l = result_val->As<ConstantInt>()->GetIntValue();
                    int r = val->As<ConstantInt>()->GetIntValue();
                    int int_result = (str_op == "+") ? l + r : l - r;
                    result_val = ConstantInt::New(_ir_module->Context()->GetInt32Ty(), int_result);
                    continue;
                }

                if (OPTIMIZE) {
                    // 代数优化
                    if (val->Is<ConstantInt>() && 
                        val->As<ConstantInt>()->GetIntValue() == 0) {
                        // x + 0, x - 0，结果为x
                        continue; // 不改变result_val
                    }
                    if (result_val->Is<ConstantInt>() && 
                        result_val->As<ConstantInt>()->GetIntValue() == 0 &&
                        str_op == "+") {
                        // 0 + x, 结果为x
                        result_val = val;
                        continue;
                    }
                }

                // 非常量，生成指令
                auto op_type = (str_op == "+") ? BinaryOpType::Add : BinaryOpType::Sub;
                result_val = BinaryOperator::New(op_type, result_val, val);
                _cur_block->InsertInstruction(result_val->As<Instruction>());
            }
        }
    }
    if (VISITOR_ERROR) return nullptr;
    return result_val;
}

ValuePtr Visitor::_visit_rel_exp(const RelExp& rel_exp) {
    // RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
    // 左递归转变为循环
    // 初始化，访问第一个add_exp
    auto result_val = _visit_add_exp(*rel_exp.add_exps[0]);

    for (int i = 1; i < rel_exp.add_exps.size(); i++) {
        if (!VISITOR_ERROR) result_val = _zext_if_i1(result_val);

        auto& add_exp = rel_exp.add_exps[i];
        auto val = _visit_add_exp(*add_exp);

        if (!VISITOR_ERROR) {
            val = _zext_if_i1(val);
            if (add_exp) {
                auto str_op = rel_exp.ops[i-1];
                auto op_type = (str_op == "<") ? CompareOpType::LessThan :
                            (str_op == ">") ? CompareOpType::GreaterThan :
                            (str_op == "<=")? CompareOpType::LessThanOrEqual :
                            CompareOpType::GreaterThanOrEqual;
                result_val = CompareInstruction::New(op_type, result_val, val);
                _cur_block->InsertInstruction(result_val->As<CompareInstruction>());
            }
        }
    }
    if (VISITOR_ERROR) return nullptr;
    return result_val;
}

ValuePtr Visitor::_visit_eq_exp(const EqExp& eq_exp) {
    // EqExp → RelExp | EqExp ('==' | '!=') RelExp
    // 左递归转变为循环
    // 初始化，访问第一个rel_exp
    auto result_val = _visit_rel_exp(*eq_exp.rel_exps[0]);

    for (int i = 1; i < eq_exp.rel_exps.size(); i++) {
        if (!VISITOR_ERROR) result_val = _zext_if_i1(result_val);

        auto& rel_exp = eq_exp.rel_exps[i];
        auto val = _visit_rel_exp(*rel_exp);
        
        if (!VISITOR_ERROR) {
            val = _zext_if_i1(val);
            if (rel_exp) {
                auto str_op = eq_exp.ops[i-1];
                auto op_type = (str_op == "==")? CompareOpType::Equal : CompareOpType::NotEqual;
                result_val = CompareInstruction::New(op_type, result_val, val);
                _cur_block->InsertInstruction(result_val->As<CompareInstruction>());  
            }
        }
    }

    if (!VISITOR_ERROR && eq_exp.rel_exps.size() == 1) {
        auto& rel_exp = eq_exp.rel_exps[0];
        if (rel_exp->add_exps.size() == 1) {
            // 整个比较表达式只有一个add_exp，说明形如 if (a) 或 for (...; a; ...)这种，需要改成a != 0
            // 这样返回的value的返回值一定是i1类型
            auto new_result_val = CompareInstruction::New(CompareOpType::NotEqual, 
                result_val, ConstantInt::New(_ir_module->Context()->GetInt32Ty(), 0));
            _cur_block->InsertInstruction(new_result_val->As<CompareInstruction>());
            return new_result_val;
        }
    }

    if (VISITOR_ERROR) return nullptr;
    return result_val;
}

ValuePtr Visitor::_visit_land_exp(const LAndExp& land_exp) {
    // LAndExp → EqExp | LAndExp '&&' EqExp
    // 左递归转变为循环

    // 初始化
    auto result_val = _visit_eq_exp(*land_exp.eq_exps[0]);

    // 循环遍历剩余的eq_exp
    for (int i = 1; i < land_exp.eq_exps.size(); i++) {
        auto& eq_exp = land_exp.eq_exps[i];
        result_val = _mix_land_exp(result_val, *eq_exp); // 该方法需要实现短路求值
    }

    if (VISITOR_ERROR) return nullptr;
    return result_val;
}

ValuePtr Visitor::_visit_lor_exp(const LOrExp& lor_exp) {
    // LOrExp → LAndExp | LOrExp '||' LAndExp
    // 左递归转变为循环
    
    // 初始化
    auto result_val = _visit_land_exp(*lor_exp.land_exps[0]);

    // 循环遍历剩余的land_exp
    for (int i = 1; i < lor_exp.land_exps.size(); i++) {
        auto& land_exp = lor_exp.land_exps[i];
        result_val = _mix_lor_exp(result_val, *land_exp); // 该方法需要实现短路求值
    }

    if (VISITOR_ERROR) return nullptr;
    return result_val;
}

ValuePtr Visitor::_visit_const_exp(const ConstExp& const_exp) {
    // ConstExp → AddExp 注：使用的 Ident 必须是常量
    if (const_exp.add_exp) {
        return _visit_add_exp(*const_exp.add_exp);
    }
    return nullptr;
}

void Visitor::_visit_ident(const Ident& ident) {
    // 无需处理
}


// 以下为辅助函数
std::string Visitor::_symbol_type_to_string(SymbolType symbol_type) {
    switch (symbol_type) {
        case SymbolType::ConstInt:
            return "ConstInt";
        case SymbolType::ConstIntArray:
            return "ConstIntArray";
        case SymbolType::StaticInt:
            return "StaticInt";
        case SymbolType::Int:
            return "Int";
        case SymbolType::IntArray:
            return "IntArray";
        case SymbolType::StaticIntArray:
            return "StaticIntArray";
        case SymbolType::IntFunc:
            return "IntFunc";
        case SymbolType::VoidFunc:
            return "VoidFunc";
        default:
            return "Unknown";
    }
}

ValuePtr Visitor::_mix_land_exp(ValuePtr left, const EqExp& right_exp) {
    if (VISITOR_ERROR) {
        _visit_eq_exp(right_exp);
        return nullptr;
    }
    // 返回值都是i1类型，需要根据短路求值规则进行处理

    // 设置临时变量作为返回值
    auto result_alloca = AllocaInst::New(_ir_module->Context()->GetInt1Ty());
    if (OPTIMIZE) {
        result_alloca->setNotToMips(); // 优化时，不将结果存储到内存，而是存到s7中
    }
    (*_cur_func->BasicBlockBegin())->InsertInstruction(result_alloca); // AllocaInst要插入到第一个基本块

    // 新建三个基本块，一个是左边的true分支，一个是左边的false分支，一个汇合点
    auto if_left_true = _cur_func->NewBasicBlock();
    auto if_left_false = _cur_func->NewBasicBlock();
    auto merge_block = _cur_func->NewBasicBlock();
    
    // 在当前基本块插入跳转指令，根据left的返回值决定跳转到哪个分支
    auto jump = BranchInst::New(left, if_left_true, if_left_false);
    _cur_block->InsertInstruction(jump);

    // 将代码生成到if_left_true分支
    _cur_block = if_left_true;
    auto right = _visit_eq_exp(right_exp); // 获取右边的返回值
    auto store_true = StoreInst::New(right, result_alloca); // left为true，则返回right
    if (OPTIMIZE) {
        store_true->setStoreToS7(); // 优化时，将结果存储到s7中
    }
    _cur_block->InsertInstruction(store_true);
    auto true_jump_to_merge = JumpInst::New(merge_block); // 无条件跳转到汇合点
    _cur_block->InsertInstruction(true_jump_to_merge);


    // 将代码生成到if_left_false分支
    _cur_block = if_left_false;
    auto false_val = ConstantInt::New(_ir_module->Context()->GetInt1Ty(), 0); // left为false，则返回false
    auto store_false = StoreInst::New(false_val, result_alloca);
    if (OPTIMIZE) {
        store_false->setStoreToS7(); // 优化时，将结果存储到s7中
    }
    _cur_block->InsertInstruction(store_false);
    auto false_jump_to_merge = JumpInst::New(merge_block); // 无条件跳转到汇合点
    if (OPTIMIZE) {
        false_jump_to_merge->setNotToMips(); // if_left_false块的下一个块就是汇合点
    }
    _cur_block->InsertInstruction(false_jump_to_merge);

    // 将代码生成到汇合点
    _cur_block = merge_block;
    auto load_result = LoadInst::New(result_alloca); // 加载临时变量
    if (OPTIMIZE) {
        load_result->setLoadFromS7(); // 优化时，从s7中加载结果
    }
    _cur_block->InsertInstruction(load_result);
    return load_result;
}

ValuePtr Visitor::_mix_lor_exp(ValuePtr left, const LAndExp& right_exp) {
    if (VISITOR_ERROR) {
        _visit_land_exp(right_exp);
        return nullptr;
    }
    // 的返回值都是i1类型，需要根据短路求值规则进行处理

    // 设置临时变量作为返回值
    auto result_alloca = AllocaInst::New(_ir_module->Context()->GetInt1Ty());
    if (OPTIMIZE) {
        result_alloca->setNotToMips(); // 优化时，不将结果存储到内存，而是存到s7中
    }
    (*_cur_func->BasicBlockBegin())->InsertInstruction(result_alloca);

    // 新建三个基本块，一个是左边的true分支，一个是左边的false分支，一个汇合点
    auto if_left_true = _cur_func->NewBasicBlock();
    auto if_left_false = _cur_func->NewBasicBlock();
    auto merge_block = _cur_func->NewBasicBlock();
    
    // 在当前基本块插入跳转指令，根据left的返回值决定跳转到哪个分支
    auto jump = BranchInst::New(left, if_left_true, if_left_false);
    _cur_block->InsertInstruction(jump);

    // 将代码生成到if_left_true分支
    _cur_block = if_left_true;
    auto true_val = ConstantInt::New(_ir_module->Context()->GetInt1Ty(), 1); // left为true，则返回true
    auto store_true = StoreInst::New(true_val, result_alloca);
    if (OPTIMIZE) {
        store_true->setStoreToS7(); // 优化时，将结果存储到s7中
    }
    _cur_block->InsertInstruction(store_true);
    auto true_jump_to_merge = JumpInst::New(merge_block); // 无条件跳转到汇合点
    _cur_block->InsertInstruction(true_jump_to_merge);

    // 将代码生成到if_left_false分支
    _cur_block = if_left_false;
    auto right = _visit_land_exp(right_exp); // 获取右边的返回值
    auto store_false = StoreInst::New(right, result_alloca); // left为false，则返回right
    if (OPTIMIZE) {
        store_false->setStoreToS7(); // 优化时，将结果存储到s7中
    }
    _cur_block->InsertInstruction(store_false);
    auto false_jump_to_merge = JumpInst::New(merge_block); // 无条件跳转到汇合点
    if (OPTIMIZE) {
        false_jump_to_merge->setNotToMips(); // if_left_false块的下一个块就是汇合点
    }
    _cur_block->InsertInstruction(false_jump_to_merge);

    // 将代码生成到汇合点
    _cur_block = merge_block;
    auto load_result = LoadInst::New(result_alloca); // 加载临时变量
    if (OPTIMIZE) {
        load_result->setLoadFromS7(); // 优化时，从s7中加载结果
    }
    _cur_block->InsertInstruction(load_result);
    return load_result;
}

ValuePtr Visitor::_zext_if_i1(ValuePtr val) {
    // 如果val是i1，扩展为i32，以进行算术运算和比较运算
    if (val->GetType() == _ir_module->Context()->GetInt1Ty()) {
        auto zext = ZextInst::New(val);
        _cur_block->InsertInstruction(zext);
        return zext;
    }
    return val;
}
