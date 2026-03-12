#ifndef Visitor_hpp
#define Visitor_hpp

#include <memory>
#include "Symbol.hpp"
#include "Ast.hpp"
#include <iostream>
#include "Module.hpp"
#include "IrForward.hpp"
#include "InstructionType.hpp"

#include <stack>

class Visitor {
    public:
        Visitor(std::ostream& error_out, std::ostream& symbol_out, ModuleSmartPtr module);

        void visit(const CompUnit& comp_unit) {_visit_comp_unit(comp_unit);};

    private:
        std::shared_ptr<SymbolTable> _cur_scope;
        std::ostream& _error_out;
        std::ostream& _symbol_out;
        int _cur_table_id;
        bool _is_void_func; // 是否在void函数体内
        int _loop_num; // 循环嵌套层数
        std::stack<BasicBlockPtr> _break_targets; // break语句跳转目标
        std::stack<BasicBlockPtr> _continue_targets; // continue语句跳转目标

        // LLVM IR
        ModuleSmartPtr _ir_module;
        FunctionPtr _cur_func;
        BasicBlockPtr _cur_block;

        // 方便生成printf
        FunctionPtr _putint;
        FunctionPtr _putch;

        void _visit_comp_unit(const CompUnit& comp_unit); // 访问comp_unitv

        // Decl是variant
        void _visit_decl(const Decl& decl); // 访问decl
        void _visit_const_decl(const ConstDecl& const_decl); // 访问const_decl
        void _visit_var_decl(const VarDecl& var_decl); // 访问var_decl
        // Decl结束

        void _visit_const_def(const ConstDef& const_def); // 访问const_def

        std::vector<ValuePtr> _visit_const_init_val(const ConstInitVal& const_init_val); // 访问const_init_val

        void _visit_var_def(const VarDef& var_def, bool is_static); // 访问var_def

        std::vector<ValuePtr> _visit_init_val(const InitVal& init_val); // 访问init_val

        void _visit_func_def(const FuncDef& func_def); // 访问func_def

        void _visit_main_func_def(const MainFuncDef& main_func_def); // 访问main_func_def

        void _visit_func_type(const FuncType& func_type); // 访问func_type

        std::vector<std::shared_ptr<VarSymbol>> _visit_func_fparams(const FuncFParams& func_fparams); // 访问func_fparams

        std::shared_ptr<VarSymbol> _visit_func_fparam(const FuncFParam& func_fparam); // 访问func_fparam

        void _visit_block(const Block& block, bool is_int_func); // 访问block

        // BlockItem是variant
        void _visit_block_item(const BlockItem& block_item); // 访问block_item
        // BlockItem结束

        // Stmt是variant
        void _visit_stmt(const Stmt& stmt); // 访问stmt
        void _visit_lval_stmt(const LValStmt& lval_stmt); // 访问lval_stmt
        void _visit_exp_stmt(const ExpStmt& exp_stmt); // 访问exp_stmt
        void _visit_block_stmt(const BlockStmt& block_stmt); // 访问block_stmt
        void _visit_if_stmt(const IfStmt& if_stmt); // 访问if_stmt
        void _visit_for_cond_stmt(const ForCondStmt& for_cond_stmt); // 访问for_cond_stmt
        void _visit_break_stmt(const BreakStmt& break_stmt); // 访问break_stmt
        void _visit_continue_stmt(const ContinueStmt& continue_stmt); // 访问continue_stmt
        void _visit_return_stmt(const ReturnStmt& return_stmt); // 访问return_stmt
        void _visit_printf_stmt(const PrintfStmt& printf_stmt); // 访问printf_stmt
        // Stmt结束

        void _visit_for_stmt(const ForStmt& for_stmt); // 访问for_stmt

        ValuePtr _visit_exp(const Exp& exp); // 访问exp

        ValuePtr _visit_cond(const Cond& cond); // 访问cond

        ValuePtr _visit_lval_forAssignment(const LVal& lval); // 赋值表达式访问lval，返回地址
        ValuePtr _visit_lval_forEvaluation(const LVal& lval); // 求值表达式访问lval，返回值

        ValuePtr _visit_primary_exp(const PrimaryExp& primary_exp); // 访问primary_exp

        ValuePtr _visit_number(const Number& number); // 访问number

        // UnaryExp是variant
        ValuePtr _visit_unary_exp(const UnaryExp& unary_exp); // 访问unary_exp
        ValuePtr _visit_primary_unary_exp(const PrimaryUnaryExp& primary_unary_exp); // 访问primary_unary_exp
        ValuePtr _visit_func_call_exp(const FuncCallExp& func_call_exp); // 访问func_call_exp
            bool _is_exp_is_array(const Exp& exp); // 判断exp是否为数组, 用于检测函数参数类型匹配
        ValuePtr _visit_unary_op_exp(const UnaryOpExp& unary_op_exp); // 访问unary_op_exp
        // UnaryExp结束

        UnaryOpType _visit_unary_op(const UnaryOp& unary_op); // 访问unary_op

        std::vector<ValuePtr> _visit_func_rparams(const FuncRParams& func_rparams); // 访问func_rparams

        ValuePtr _visit_mul_exp(const MulExp& mul_exp); // 访问mul_exp

        ValuePtr _visit_add_exp(const AddExp& add_exp); // 访问add_exp

        ValuePtr _visit_rel_exp(const RelExp& rel_exp); // 访问rel_exp

        ValuePtr _visit_eq_exp(const EqExp& eq_exp); // 访问eq_exp

        ValuePtr _visit_land_exp(const LAndExp& land_exp); // 访问land_exp

        ValuePtr _visit_lor_exp(const LOrExp& lor_exp); // 访问lor_exp

        ValuePtr _visit_const_exp(const ConstExp& const_exp); // 访问const_exp

        void _visit_ident(const Ident& ident); // 访问ident

        // 辅助函数
        enum SymbolType {
            ConstInt, ConstIntArray, StaticInt, Int, IntArray, StaticIntArray, IntFunc, VoidFunc
        };

        std::string _symbol_type_to_string(SymbolType symbol_type);
        ValuePtr _mix_land_exp(ValuePtr left, const EqExp& right_exp);
        ValuePtr _mix_lor_exp(ValuePtr left, const LAndExp& right_exp);
        ValuePtr _zext_if_i1(ValuePtr value); // 扩展i1类型到i32类型
};

#endif