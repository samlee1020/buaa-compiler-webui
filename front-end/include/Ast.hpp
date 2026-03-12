#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <vector>
#include <memory>
#include <variant>

// 语法树基节点
struct Node {
    int lineno;

    Node() = default;
    Node(int lineno) : lineno(lineno) {}
    virtual void print(std::ostream &out) = 0;
};

// 前向声明所有struct
struct CompUnit;
struct ConstDecl;
struct ConstDef;
struct ConstInitVal;
struct VarDecl;
struct VarDef;
struct InitVal;
struct FuncDef;
struct MainFuncDef;
struct FuncType;
struct FuncFParams;
struct FuncFParam;
struct Block;
struct LValStmt;
struct ExpStmt;
struct BlockStmt;
struct IfStmt;
struct ForCondStmt;
struct BreakStmt;
struct ContinueStmt;
struct ReturnStmt;
struct PrintfStmt;
struct ForStmt;
struct Exp;
struct Cond;
struct LVal;
struct PrimaryExp;
struct Number;
struct PrimaryUnaryExp;
struct Ident;
struct BType;
struct ConstExp;
struct UnaryOpExp;
struct FuncCallExp;
struct AddExp;
struct LOrExp;
struct FuncRParams;
struct UnaryOp;


// 两种声明
/*
Decl → ConstDecl 
| VarDecl
*/
using Decl = std::variant<ConstDecl, VarDecl>;

// 多种语句
/*
Stmt → LVal '=' Exp ';' 
| [Exp] ';' 
| Block
| 'if' '(' Cond ')' Stmt [ 'else' Stmt ] 
| 'for' '(' [ForStmt] ';' [Cond] ';' [ForStmt] ')' Stmt 
| 'break' ';'
| 'continue' ';'
| 'return' [Exp] ';' 
| 'printf''('StringConst {','Exp}')'';'
*/
using Stmt = std::variant<LValStmt, ExpStmt, BlockStmt, IfStmt, ForCondStmt, BreakStmt, ContinueStmt, ReturnStmt, PrintfStmt>;

// 语句块内容
/*
BlockItem → Decl 
| Stmt
*/
using BlockItem = std::variant<Decl, Stmt>;

// 初等表达式内容
/*
PrimaryExp → '(' Exp ')' 
| LVal 
| Number
*/
using PrimaryExpContent = std::variant<Exp, LVal, Number>;

// 一元表达式
/*
UnaryExp → PrimaryExp 
| Ident '(' [FuncRParams] ')' 
| UnaryOp UnaryExp
*/
using UnaryExp = std::variant<PrimaryUnaryExp, FuncCallExp, UnaryOpExp>;

// 声明节点
struct CompUnit : public Node {
    // CompUnit → {Decl} {FuncDef} MainFuncDef
    std::vector<std::unique_ptr<Decl>> decls;
    std::vector<std::unique_ptr<FuncDef>> funcDefs;
    std::unique_ptr<MainFuncDef> mainFuncDef;

    void print(std::ostream &out) override;
};

struct ConstDecl : public Node {
    // ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'
    std::vector<std::unique_ptr<ConstDef>> const_defs;

    void print(std::ostream &out) override;
};

struct ConstDef : public Node {
    // ConstDef → Ident [ '[' ConstExp ']' ] '=' ConstInitVal
    std::unique_ptr<Ident> ident;
    std::unique_ptr<ConstExp> const_exp;
    std::unique_ptr<ConstInitVal> const_init_val;

    void print(std::ostream &out) override;
};

struct ConstInitVal : public Node {
    // ConstInitVal → ConstExp | '{' [ ConstExp { ',' ConstExp } ] '}'
    std::vector<std::unique_ptr<ConstExp>> const_exps;
    bool is_array;

    void print(std::ostream &out) override;
};

struct VarDecl : public Node {
    // VarDecl → [ 'static' ] BType VarDef { ',' VarDef } ';'
    bool is_static;
    std::vector<std::unique_ptr<VarDef>> var_defs;

    void print(std::ostream &out) override;
};

struct VarDef : public Node { 
    // VarDef → Ident [ '[' ConstExp ']' ] | Ident [ '[' ConstExp ']' ] '=' InitVal
    std::unique_ptr<Ident> ident;
    std::unique_ptr<ConstExp> const_exp;
    std::unique_ptr<InitVal> init_val;

    void print(std::ostream &out) override;
};

struct InitVal : public Node {
    // InitVal → Exp | '{' [ Exp { ',' Exp } ] '}'
    std::vector<std::unique_ptr<Exp>> exps;
    bool is_array;

    void print(std::ostream &out) override;
};

struct FuncDef : public Node {
    // FuncDef → FuncType Ident '(' [FuncFParams] ')' Block
    std::unique_ptr<FuncType> func_type;
    std::unique_ptr<Ident> ident;
    std::unique_ptr<FuncFParams> func_fparams;
    std::unique_ptr<Block> block;

    void print(std::ostream &out) override;
};

struct MainFuncDef : public Node {
    // MainFuncDef → 'int' 'main' '(' ')' Block
    std::unique_ptr<Block> block;

    void print(std::ostream &out) override;
};

struct FuncType : public Node {
    // FuncType → 'int' | 'void'
    bool is_int;

    void print(std::ostream &out) override;
};

struct FuncFParams : public Node {
    // FuncFParams → FuncFParam { ',' FuncFParam }
    std::vector<std::unique_ptr<FuncFParam>> func_fparams;

    void print(std::ostream &out) override;
};

struct FuncFParam : public Node {
    // FuncFParam → BType Ident ['[' ']']
    std::unique_ptr<Ident> ident;
    bool is_array;

    void print(std::ostream &out) override;
};

struct Block : public Node {
    // Block → '{' { BlockItem } '}'
    std::vector<std::unique_ptr<BlockItem>> block_items;
    int lineno_of_end; // 右大括号的行号，方便语义分析时错误处理

    void print(std::ostream &out) override;
};

struct LValStmt : public Node {
    // LValStmt → LVal '=' Exp ';'
    std::unique_ptr<LVal> lval;
    std::unique_ptr<Exp> exp;

    void print(std::ostream &out) override;
};

struct ExpStmt : public Node {
    // ExpStmt → [Exp] ';'
    std::unique_ptr<Exp> exp;

    void print(std::ostream &out) override;
};

struct BlockStmt : public Node {
    // BlockStmt → Block
    std::unique_ptr<Block> block;

    void print(std::ostream &out) override;
};

struct IfStmt : public Node {
    // IfStmt → 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
    std::unique_ptr<Cond> cond;
    std::unique_ptr<Stmt> stmt;
    std::unique_ptr<Stmt> else_stmt;

    void print(std::ostream &out) override;
};

struct ForCondStmt : public Node {
    // ForCondStmt → 'for' '(' [ForStmt] ';' [Cond] ';' [ForStmt] ')' Stmt
    std::unique_ptr<ForStmt> for_stmt1;
    std::unique_ptr<Cond> cond;
    std::unique_ptr<ForStmt> for_stmt2;
    std::unique_ptr<Stmt> stmt;

    void print(std::ostream &out) override;
};

struct BreakStmt : public Node {
    // BreakStmt → 'break' ';'
    void print(std::ostream &out) override;
};

struct ContinueStmt : public Node {
    // ContinueStmt → 'continue' ';'
    void print(std::ostream &out) override;
};

struct ReturnStmt : public Node {
    // ReturnStmt → 'return' [Exp] ';'
    std::unique_ptr<Exp> exp;

    void print(std::ostream &out) override;
};

struct PrintfStmt : public Node {
    // PrintfStmt → 'printf''('StringConst {','Exp}')'';'
    std::string string_const;
    std::vector<std::unique_ptr<Exp>> exps;

    void print(std::ostream &out) override;
};

struct ForStmt : public Node {
    // ForStmt → LVal '=' Exp { ',' LVal '=' Exp }
    std::vector<std::unique_ptr<LVal>> lvals;
    std::vector<std::unique_ptr<Exp>> exps;

    void print(std::ostream &out) override;
};

struct Exp : public Node {
    // Exp → AddExp
    std::unique_ptr<AddExp> add_exp;

    void print(std::ostream &out) override;
};

struct Cond : public Node {
    // Cond → LOrExp
    std::unique_ptr<LOrExp> lor_exp;

    void print(std::ostream &out) override;
};

struct LVal : public Node {
    // LVal → Ident [ '[' Exp ']' ]
    std::unique_ptr<Ident> ident;
    std::unique_ptr<Exp> exp;

    void print(std::ostream &out) override;
};

struct PrimaryExp : public Node {
    // PrimaryExp → '(' Exp ')' | LVal | Number
    std::unique_ptr<PrimaryExpContent> content;

    void print(std::ostream &out) override;
};

struct Number : public Node {
    // Number → IntConst
    std::string int_const;

    void print(std::ostream &out) override;
};

struct PrimaryUnaryExp : public Node {
    // PrimaryUnaryExp → PrimaryExp 
    std::unique_ptr<PrimaryExp> primary_exp;

    void print(std::ostream &out) override;
};

struct FuncCallExp : public Node {
    // FuncCallExp → Ident '(' [FuncRParams] ')'
    std::unique_ptr<Ident> ident;
    std::unique_ptr<FuncRParams> func_rparams;

    void print(std::ostream &out) override;
};

struct UnaryOpExp : public Node {
    // UnaryOpExp → UnaryOp UnaryExp
    std::unique_ptr<UnaryOp> unary_op;
    std::unique_ptr<UnaryExp> unary_exp;

    void print(std::ostream &out) override;
};

struct UnaryOp : public Node {
    // UnaryOp → '+' | '−' | '!'
    std::string op;

    void print(std::ostream &out) override;
};

struct FuncRParams : public Node {
    // FuncRParams → Exp { ',' Exp }
    std::vector<std::unique_ptr<Exp>> exps;

    void print(std::ostream &out) override;
};

struct MulExp : public Node {
    // MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
    // 左递归转变为循环
    std::vector<std::unique_ptr<UnaryExp>> unary_exps;
    std::vector<std::string> ops;

    void print(std::ostream &out) override;
};

struct AddExp : public Node {
    // AddExp → MulExp | AddExp ('+' | '−') MulExp
    // 左递归转变为循环
    std::vector<std::unique_ptr<MulExp>> mul_exps;
    std::vector<std::string> ops;

    void print(std::ostream &out) override;
};

struct RelExp : public Node {
    // RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
    // 左递归转变为循环
    std::vector<std::unique_ptr<AddExp>> add_exps;
    std::vector<std::string> ops;

    void print(std::ostream &out) override;
};

struct EqExp : public Node {
    // EqExp → RelExp | EqExp ('==' | '!=') RelExp
    // 左递归转变为循环
    std::vector<std::unique_ptr<RelExp>> rel_exps;
    std::vector<std::string> ops;

    void print(std::ostream &out) override;
};

struct LAndExp : public Node {
    // LAndExp → EqExp | LAndExp '&&' EqExp
    // 左递归转变为循环
    std::vector<std::unique_ptr<EqExp>> eq_exps;

    void print(std::ostream &out) override;
};

struct LOrExp : public Node {
    // LOrExp → LAndExp | LOrExp '||' LAndExp
    // 左递归转变为循环
    std::vector<std::unique_ptr<LAndExp>> land_exps;

    void print(std::ostream &out) override;
};

struct ConstExp : public Node {
    // ConstExp → AddExp 注：使用的 Ident 必须是常量
    std::unique_ptr<AddExp> add_exp;

    void print(std::ostream &out) override;
};

struct Ident : public Node {
    std::string name;

    void print(std::ostream &out) override;
}; 

// 辅助输出variant类型--------------------------------------------------------------------------------------------
void print_decl(Decl &decl, std::ostream &out);
void print_block_item(BlockItem &block_item, std::ostream &out);
void print_stmt(Stmt &stmt, std::ostream &out);
void print_primary_exp_content(PrimaryExpContent &primary_exp_content, std::ostream &out);
void print_unary_exp(UnaryExp &unary_exp, std::ostream &out);

#endif