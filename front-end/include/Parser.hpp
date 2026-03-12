#ifndef PARSER_HPP
#define PARSER_HPP

#include "Lexer.hpp"
#include "Ast.hpp"

#include <memory>
#include <deque>

class Parser {
    public:
        Parser(Lexer &lexer, std::ostream &error_out) : _lexer(lexer) , _error_out(error_out) {
            _next_token();
        };

        std::unique_ptr<CompUnit> parse(); // 解析lexer的tokens，返回ast

    private:
        Lexer &_lexer;
        Token _cur_token, _last_token;
        std::deque<Token> _peek_tokens_list;
        std::ostream &_error_out;

        void _next_token(); // 读取下一个token为_cur_token
        Token _peek_token(int offset); // 预读_cur_token的后的第offset个token

        void _match(Token::TokenType expected_type);

        std::unique_ptr<CompUnit> _parse_comp_unit(); // 解析comp_unit
        bool _is_decl_of_comp_unit(); // 判断是否是decl
        bool _is_func_def_of_comp_unit(); // 判断是否是func_def

        // Decl是variant
        std::unique_ptr<Decl> _parse_decl(); // 解析decl
        std::unique_ptr<ConstDecl> _parse_const_decl(); // 解析const_decl
        std::unique_ptr<VarDecl> _parse_var_decl(); // 解析var_decl
        // Decl结束

        std::unique_ptr<ConstDef> _parse_const_def(); // 解析const_def

        std::unique_ptr<ConstInitVal> _parse_const_init_val(); // 解析const_init_val

        std::unique_ptr<VarDef> _parse_var_def(); // 解析var_def

        std::unique_ptr<InitVal> _parse_init_val(); // 解析init_val

        std::unique_ptr<FuncDef> _parse_func_def(); // 解析func_def

        std::unique_ptr<MainFuncDef> _parse_main_func_def(); // 解析main_func_def

        std::unique_ptr<FuncType> _parse_func_type(); // 解析func_type

        std::unique_ptr<FuncFParams> _parse_func_fparams(); // 解析func_fparams

        std::unique_ptr<FuncFParam> _parse_func_fparam(); // 解析func_fparam

        std::unique_ptr<Block> _parse_block(); // 解析block
        
        // BlockItem是variant
        std::unique_ptr<BlockItem> _parse_block_item(); // 解析block_item
        bool _is_decl_of_block_item(); // 判断是否是decl
        // BlockItem结束

        // Stmt是variant
        std::unique_ptr<Stmt> _parse_stmt(); // 解析stmt
        bool _is_lval_stmt_of_stmt(); // 判断是否是lval_stmt。这里是判断下一个分号前有没有出现赋值符号
        std::unique_ptr<LValStmt> _parse_lval_stmt(); // 解析lval_stmt
        std::unique_ptr<ExpStmt> _parse_exp_stmt(); // 解析exp_stmt
        std::unique_ptr<BlockStmt> _parse_block_stmt(); // 解析block_stmt
        std::unique_ptr<IfStmt> _parse_if_stmt(); // 解析if_stmt
        std::unique_ptr<ForCondStmt> _parse_for_cond_stmt(); // 解析for_cond_stmt
        std::unique_ptr<BreakStmt> _parse_break_stmt(); // 解析break_stmt
        std::unique_ptr<ContinueStmt> _parse_continue_stmt(); // 解析continue_stmt
        std::unique_ptr<ReturnStmt> _parse_return_stmt(); // 解析return_stmt
        std::unique_ptr<PrintfStmt> _parse_printf_stmt(); // 解析printf_stmt
        // Stmt结束

        std::unique_ptr<ForStmt> _parse_for_stmt(); // 解析for_stmt

        std::unique_ptr<Exp> _parse_exp(); // 解析exp

        std::unique_ptr<Cond> _parse_cond(); // 解析cond

        std::unique_ptr<LVal> _parse_lval(); // 解析lval

        std::unique_ptr<PrimaryExp> _parse_primary_exp(); // 解析primary_exp

        std::unique_ptr<Number> _parse_number(); // 解析number

        // UnaryExp是variant
        std::unique_ptr<UnaryExp> _parse_unary_exp(); // 解析unary_exp
        std::unique_ptr<PrimaryUnaryExp> _parse_primary_unary_exp(); // 解析primary_unary_exp
        std::unique_ptr<FuncCallExp> _parse_func_call_exp(); // 解析func_call_exp
        bool _is_func_rparams_of_func_call_exp(); // 判断是否有func_rparams
        std::unique_ptr<UnaryOpExp> _parse_unary_op_exp(); // 解析unary_op_exp
        // UnaryExp结束

        std::unique_ptr<UnaryOp> _parse_unary_op(); // 解析unary_op

        std::unique_ptr<FuncRParams> _parse_func_rparams(); // 解析func_rparams

        std::unique_ptr<MulExp> _parse_mul_exp(); // 解析fmul_exp

        std::unique_ptr<AddExp> _parse_add_exp(); // 解析add_exp

        std::unique_ptr<RelExp> _parse_rel_exp(); // 解析rel_exp

        std::unique_ptr<EqExp> _parse_eq_exp(); // 解析eq_exp

        std::unique_ptr<LAndExp> _parse_land_exp(); // 解析land_exp

        std::unique_ptr<LOrExp> _parse_lor_exp(); // 解析lor_exp

        std::unique_ptr<ConstExp> _parse_const_exp(); // 解析const_exp

        std::unique_ptr<Ident> _parse_ident(); // 解析ident
};

#endif