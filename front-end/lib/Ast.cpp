#include "Ast.hpp"
#include "Token.hpp"

void CompUnit::print(std::ostream &out) {
    // CompUnit → {Decl} {FuncDef} MainFuncDef
    // 输出decls
    for (const auto &decl : decls) {
        if (decl) {
            print_decl(*decl, out);
        }
    }
    // 输出funcDefs
    for (const auto &funcDef : funcDefs) {
        if (funcDef) {
            funcDef->print(out);
        }
    }
    // 输出mainFuncDef
    if (mainFuncDef) {
        mainFuncDef->print(out);
    }
    out << "<CompUnit>" << std::endl;
}

void ConstDecl::print(std::ostream &out) {
    // ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'
    out << tokenTypeToString(Token::TokenType::CONSTTK) << " " << "const" << std::endl;
    out << tokenTypeToString(Token::TokenType::INTTK) << " " << "int" << std::endl;
    for (int i = 0; i < const_defs.size(); i++) {
        if (const_defs[i]) {
            const_defs[i]->print(out);
            if (i < const_defs.size() - 1) {
                out << tokenTypeToString(Token::TokenType::COMMA) << " " << "," << std::endl;
            }
        }
    }
    out << tokenTypeToString(Token::TokenType::SEMICN) << " " << ";" << std::endl;
    out << "<ConstDecl>" << std::endl;
}

void ConstDef::print(std::ostream &out) {
    // ConstDef → Ident [ '[' ConstExp ']' ] '=' ConstInitVal
    if (ident) {
        ident->print(out);
    }
    if (const_exp) {
        out << tokenTypeToString(Token::TokenType::LBRACK) << " " << "[" << std::endl;
        const_exp->print(out);
        out << tokenTypeToString(Token::TokenType::RBRACK) << " " << "]" << std::endl;
    }
    out << tokenTypeToString(Token::TokenType::ASSIGN) << " " << "=" << std::endl;
    if (const_init_val) {
        const_init_val->print(out);
    }
    out << "<ConstDef>" << std::endl;
}

void ConstInitVal::print(std::ostream &out) {
    // ConstInitVal → ConstExp | '{' [ ConstExp { ',' ConstExp } ] '}'
    if (is_array) {
        out << tokenTypeToString(Token::TokenType::LBRACE) << " " << "{" << std::endl;
        for (int i = 0; i < const_exps.size(); i++) {
            if (const_exps[i]) {
                const_exps[i]->print(out);
                if (i < const_exps.size() - 1) {
                    out << tokenTypeToString(Token::TokenType::COMMA) << " " << "," << std::endl;
                }
            }
        }
        out << tokenTypeToString(Token::TokenType::RBRACE) << " " << "}" << std::endl;
    } else {
        if (const_exps.size() > 0) {
            const_exps[0]->print(out);
        }
    }
    out << "<ConstInitVal>" << std::endl;
}

void VarDecl::print(std::ostream &out) {
    // VarDecl → [ 'static' ] BType VarDef { ',' VarDef } ';'
    if (is_static) {
        out << tokenTypeToString(Token::TokenType::STATICTK) << " " << "static" << std::endl;
    }
    out << tokenTypeToString(Token::TokenType::INTTK) << " " << "int" << std::endl;
    for (int i = 0; i < var_defs.size(); i++) {
        if (var_defs[i]) {
            var_defs[i]->print(out);
            if (i < var_defs.size() - 1) {
                out << tokenTypeToString(Token::TokenType::COMMA) << " " << "," << std::endl;
            }
        }
    }
    out << tokenTypeToString(Token::TokenType::SEMICN) << " " << ";" << std::endl;
    out << "<VarDecl>" << std::endl;
}

void VarDef::print(std::ostream &out) {
    // VarDef → Ident [ '[' ConstExp ']' ] | Ident [ '[' ConstExp ']' ] '=' InitVal
    if (ident) {
        ident->print(out);
    }
    if (const_exp) {
        out << tokenTypeToString(Token::TokenType::LBRACK) << " " << "[" << std::endl;
        const_exp->print(out);
        out << tokenTypeToString(Token::TokenType::RBRACK) << " " << "]" << std::endl;
    }
    if (init_val) {
        out << tokenTypeToString(Token::TokenType::ASSIGN) << " " << "=" << std::endl;
        init_val->print(out);
    }
    out << "<VarDef>" << std::endl;
}

void InitVal::print(std::ostream &out) {
    // InitVal → Exp | '{' [ Exp { ',' Exp } ] '}'
    if (is_array) {
        out << tokenTypeToString(Token::TokenType::LBRACE) << " " << "{" << std::endl;
        for (int i = 0; i < exps.size(); i++) {
            if (exps[i]) {
                exps[i]->print(out);
                if (i < exps.size() - 1) {
                    out << tokenTypeToString(Token::TokenType::COMMA) << " " << "," << std::endl;
                }
            }
        }
        out << tokenTypeToString(Token::TokenType::RBRACE) << " " << "}" << std::endl;
    } else {
        if (exps.size() > 0) {
            exps[0]->print(out);
        }
    }
    out << "<InitVal>" << std::endl;
}

void FuncDef::print(std::ostream &out) {
    // FuncDef → FuncType Ident '(' [FuncFParams] ')' Block
    if (func_type) {
        func_type->print(out);
    }
    if (ident) {
        ident->print(out);
    }
    out << tokenTypeToString(Token::TokenType::LPARENT) << " " << "(" << std::endl;
    if (func_fparams) {
        func_fparams->print(out);
    }
    out << tokenTypeToString(Token::TokenType::RPARENT) << " " << ")" << std::endl;
    if (block) {
        block->print(out);
    }
    out << "<FuncDef>" << std::endl;
}

void MainFuncDef::print(std::ostream &out) {
    // MainFuncDef → 'int' 'main' '(' ')' Block
    out << tokenTypeToString(Token::TokenType::INTTK) << " " << "int" << std::endl;
    out << tokenTypeToString(Token::TokenType::MAINTK) << " " << "main" << std::endl;
    out << tokenTypeToString(Token::TokenType::LPARENT) << " " << "(" << std::endl;
    out << tokenTypeToString(Token::TokenType::RPARENT) << " " << ")" << std::endl;
    if (block) {
        block->print(out);
    }
    out << "<MainFuncDef>" << std::endl;
}

void FuncType::print(std::ostream &out) {
    // FuncType → 'int' | 'void'
    if (is_int) {
        out << tokenTypeToString(Token::TokenType::INTTK) << " " << "int" << std::endl;
    } else {
        out << tokenTypeToString(Token::TokenType::VOIDTK) << " " << "void" << std::endl;
    }
    out << "<FuncType>" << std::endl;
}

void FuncFParams::print(std::ostream &out) {
    // FuncFParams → FuncFParam { ',' FuncFParam }
    for (int i = 0; i < func_fparams.size(); i++) {
        if (func_fparams[i]) {
            func_fparams[i]->print(out);
            if (i < func_fparams.size() - 1) {
                out << tokenTypeToString(Token::TokenType::COMMA) << " " << "," << std::endl;
            }
        }
    }
    out << "<FuncFParams>" << std::endl;
}

void FuncFParam::print(std::ostream &out) {
    // FuncFParam → BType Ident ['[' ']']
    out << tokenTypeToString(Token::TokenType::INTTK) << " " << "int" << std::endl;
    if (ident) {
        ident->print(out);
    }
    if (is_array) {
        out << tokenTypeToString(Token::TokenType::LBRACK) << " " << "[" << std::endl;
        out << tokenTypeToString(Token::TokenType::RBRACK) << " " << "]" << std::endl;
    }
    out << "<FuncFParam>" << std::endl;
}

void Block::print(std::ostream &out) {
    // Block → '{' { BlockItem } '}'
    out << tokenTypeToString(Token::TokenType::LBRACE) << " " << "{" << std::endl;
    for (const auto &block_item : block_items) {
        if (block_item) {
            print_block_item(*block_item, out);
        }
    }
    out << tokenTypeToString(Token::TokenType::RBRACE) << " " << "}" << std::endl;
    out << "<Block>" << std::endl;
}

void LValStmt::print(std::ostream &out) {
    // LValStmt → LVal '=' Exp ';'
    if (lval) {
        lval->print(out);
    }
    out << tokenTypeToString(Token::TokenType::ASSIGN) << " " << "=" << std::endl;
    if (exp) {
        exp->print(out);
    }
    out << tokenTypeToString(Token::TokenType::SEMICN) << " " << ";" << std::endl;
    out << "<Stmt>" << std::endl;
}

void ExpStmt::print(std::ostream &out) {
    // ExpStmt → [Exp] ';'
    if (exp) {
        exp->print(out);
    }
    out << tokenTypeToString(Token::TokenType::SEMICN) << " " << ";" << std::endl;
    out << "<Stmt>" << std::endl;
}

void BlockStmt::print(std::ostream &out) {
    // BlockStmt → Block
    if (block) {
        block->print(out);
    }
    out << "<Stmt>" << std::endl;
}

void IfStmt::print(std::ostream &out) {
    // IfStmt → 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
    out << tokenTypeToString(Token::TokenType::IFTK) << " " << "if" << std::endl;
    out << tokenTypeToString(Token::TokenType::LPARENT) << " " << "(" << std::endl;
    if (cond) {
        cond->print(out);
    }
    out << tokenTypeToString(Token::TokenType::RPARENT) << " " << ")" << std::endl;
    if (stmt) {
        print_stmt(*stmt, out);
    }
    if (else_stmt) {
        out << tokenTypeToString(Token::TokenType::ELSETK) << " " << "else" << std::endl;
        print_stmt(*else_stmt, out);
    }
    out << "<Stmt>" << std::endl;
}

void ForCondStmt::print(std::ostream &out) {
    // ForCondStmt → 'for' '(' [ForStmt] ';' [Cond] ';' [ForStmt] ')' Stmt
    out << tokenTypeToString(Token::TokenType::FORTK) << " " << "for" << std::endl;
    out << tokenTypeToString(Token::TokenType::LPARENT) << " " << "(" << std::endl;
    if (for_stmt1) {
        for_stmt1->print(out);
    }
    out << tokenTypeToString(Token::TokenType::SEMICN) << " " << ";" << std::endl;
    if (cond) {
        cond->print(out);
    }
    out << tokenTypeToString(Token::TokenType::SEMICN) << " " << ";" << std::endl;
    if (for_stmt2) {
        for_stmt2->print(out);
    }
    out << tokenTypeToString(Token::TokenType::RPARENT) << " " << ")" << std::endl;
    if (stmt) {
        print_stmt(*stmt, out);
    }
    out << "<Stmt>" << std::endl;
}

void BreakStmt::print(std::ostream &out) {
    // BreakStmt → 'break' ';'
    out << tokenTypeToString(Token::TokenType::BREAKTK) << " " << "break" << std::endl;
    out << tokenTypeToString(Token::TokenType::SEMICN) << " " << ";" << std::endl;
    out << "<Stmt>" << std::endl;
}

void ContinueStmt::print(std::ostream &out) {
    // ContinueStmt → 'continue' ';'
    out << tokenTypeToString(Token::TokenType::CONTINUETK) << " " << "continue" << std::endl;
    out << tokenTypeToString(Token::TokenType::SEMICN) << " " << ";" << std::endl;
    out << "<Stmt>" << std::endl;
}

void ReturnStmt::print(std::ostream &out) {
    // ReturnStmt →'return' [Exp] ';'
    out << tokenTypeToString(Token::TokenType::RETURNTK) << " " << "return" << std::endl;
    if (exp) {
        exp->print(out);
    }
    out << tokenTypeToString(Token::TokenType::SEMICN) << " " << ";" << std::endl;
    out << "<Stmt>" << std::endl;
}

void PrintfStmt::print(std::ostream &out) {
    // PrintfStmt → 'printf''('StringConst {','Exp}')'';'
    out << tokenTypeToString(Token::TokenType::PRINTFTK) << " " << "printf" << std::endl;
    out << tokenTypeToString(Token::TokenType::LPARENT) << " " << "(" << std::endl;
    out << tokenTypeToString(Token::TokenType::STRCON) << " " <<string_const << std::endl;
    for (int i = 0; i < exps.size(); i++) {
        if (exps[i]) {
            out << tokenTypeToString(Token::TokenType::COMMA) << " " << "," << std::endl;
            exps[i]->print(out);
        }
    }
    out << tokenTypeToString(Token::TokenType::RPARENT) << " " << ")" << std::endl;
    out << tokenTypeToString(Token::TokenType::SEMICN) << " " << ";" << std::endl;
    out << "<Stmt>" << std::endl;
}

void ForStmt::print(std::ostream &out) {
    // ForStmt → LVal '=' Exp { ',' LVal '=' Exp }
    for (int i = 0; i < lvals.size(); i++) {
        if (lvals[i] && exps[i]) {
            lvals[i]->print(out);
            out << tokenTypeToString(Token::TokenType::ASSIGN) << " " << "=" << std::endl;
            exps[i]->print(out);
            if (i < lvals.size() - 1) {
                out << tokenTypeToString(Token::TokenType::COMMA) << " " << "," << std::endl;
            }
        }
    }
    out << "<ForStmt>" << std::endl;
}

void Exp::print(std::ostream &out) {
    // Exp → AddExp
    if (add_exp) {
        add_exp->print(out);
    }
    out << "<Exp>" << std::endl;
}

void Cond::print(std::ostream &out) {
    // Cond → LOrExp
    if (lor_exp) {
        lor_exp->print(out);
    }
    out << "<Cond>" << std::endl;
}

void LVal::print(std::ostream &out) {
    // LVal → Ident [ '[' Exp ']' ]
    if (ident) {
        ident->print(out);
    }
    if (exp) {
        out << tokenTypeToString(Token::TokenType::LBRACK) << " " << "[" << std::endl;
        exp->print(out);
        out << tokenTypeToString(Token::TokenType::RBRACK) << " " << "]" << std::endl;
    }
    out << "<LVal>" << std::endl;
}

void PrimaryExp::print(std::ostream &out) {
    // PrimaryExp → '(' Exp ')' | LVal | Number
    print_primary_exp_content(*content, out);
    out << "<PrimaryExp>" << std::endl;
}

void Number::print(std::ostream &out) {
    // Number → IntConst
    out << tokenTypeToString(Token::TokenType::INTCON) << " " << int_const << std::endl;
    out << "<Number>" << std::endl;
}

void PrimaryUnaryExp::print(std::ostream &out) {
    // PrimaryUnaryExp → PrimaryExp 
    if (primary_exp) {
        primary_exp->print(out);
    }
    out << "<UnaryExp>" << std::endl;
}

void FuncCallExp::print(std::ostream &out) {
    // FuncCallExp → Ident '(' [FuncRParams] ')'
    if (ident) {
        ident->print(out);
    }
    out << tokenTypeToString(Token::TokenType::LPARENT) << " " << "(" << std::endl;
    if (func_rparams) {
        func_rparams->print(out);
    }
    out << tokenTypeToString(Token::TokenType::RPARENT) << " " << ")" << std::endl;
    out << "<UnaryExp>" << std::endl;
}

void UnaryOpExp::print(std::ostream &out) {
    // UnaryOpExp → UnaryOp UnaryExp
    if (unary_op) {
        unary_op->print(out);
    }
    if (unary_exp) {
        print_unary_exp(*unary_exp, out);
    }
    out << "<UnaryExp>" << std::endl;
}

void UnaryOp::print(std::ostream &out) {
    // UnaryOp → '+' | '−' | '!'
    if (op == "+") {
        out << tokenTypeToString(Token::TokenType::PLUS) << " " << "+" << std::endl;
    } else if (op == "-") {
        out << tokenTypeToString(Token::TokenType::MINU) << " " << "-" << std::endl;
    } else if (op == "!") {
        out << tokenTypeToString(Token::TokenType::NOT) << " " << "!" << std::endl;
    }
    out << "<UnaryOp>" << std::endl;
}

void FuncRParams::print(std::ostream &out) {
    // FuncRParams → Exp { ',' Exp }
    for (int i = 0; i < exps.size(); i++) {
        if (exps[i]) {
            exps[i]->print(out);
            if (i < exps.size() - 1) {
                out << tokenTypeToString(Token::TokenType::COMMA) << " " << "," << std::endl;
            }
        }
    }
    out << "<FuncRParams>" << std::endl;
}

void MulExp::print(std::ostream &out) {
    // MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
    // 左递归转变为循环
    for (int i = 0; i < unary_exps.size(); i++) {
        if (unary_exps[i]) {
            print_unary_exp(*unary_exps[i], out);
            out << "<MulExp>" << std::endl;
            if (i < unary_exps.size() - 1) {
                if (ops[i] == "*") {
                    out << tokenTypeToString(Token::TokenType::MULT) << " " << "*" << std::endl;
                } else if (ops[i] == "/") {
                    out << tokenTypeToString(Token::TokenType::DIV) << " " << "/" << std::endl;
                } else if (ops[i] == "%") {
                    out << tokenTypeToString(Token::TokenType::MOD) << " " << "%" << std::endl;
                }
            }
        }
    }
}

void AddExp::print(std::ostream &out) {
    // AddExp → MulExp | AddExp ('+' | '−') MulExp
    // 左递归转变为循环
    for (int i = 0; i < mul_exps.size(); i++) {
        if (mul_exps[i]) {
            mul_exps[i]->print(out);
            out << "<AddExp>" << std::endl;
            if (i < mul_exps.size() - 1) {
                if (ops[i] == "+") {
                    out << tokenTypeToString(Token::TokenType::PLUS) << " " << "+" << std::endl;
                } else if (ops[i] == "-") {
                    out << tokenTypeToString(Token::TokenType::MINU) << " " << "-" << std::endl;
                }
            }
        }
    }
}

void RelExp::print(std::ostream &out) {
    // RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
    // 左递归转变为循环
    for (int i = 0; i < add_exps.size(); i++) {
        if (add_exps[i]) {
            add_exps[i]->print(out);
            out << "<RelExp>" << std::endl;
            if (i < add_exps.size() - 1) {
                if (ops[i] == "<") {
                    out << tokenTypeToString(Token::TokenType::LSS) << " " << "<" << std::endl;
                } else if (ops[i] == ">") {
                    out << tokenTypeToString(Token::TokenType::GRE) << " " << ">" << std::endl;
                } else if (ops[i] == "<=") {
                    out << tokenTypeToString(Token::TokenType::LEQ) << " " << "<=" << std::endl;
                } else if (ops[i] == ">=") {
                    out << tokenTypeToString(Token::TokenType::GEQ) << " " << ">=" << std::endl;
                }
            }
        }
    }
}

void EqExp::print(std::ostream &out) {
    // EqExp → RelExp | EqExp ('==' | '!=') RelExp
    // 左递归转变为循环
    for (int i = 0; i < rel_exps.size(); i++) {
        if (rel_exps[i]) {
            rel_exps[i]->print(out);
            out << "<EqExp>" << std::endl;
            if (i < rel_exps.size() - 1) {
                if (ops[i] == "==") {
                    out << tokenTypeToString(Token::TokenType::EQL) << " " << "==" << std::endl;
                } else if (ops[i] == "!=") {
                    out << tokenTypeToString(Token::TokenType::NEQ) << " " << "!=" << std::endl;
                }
            }
        }
    }
}

void LAndExp::print(std::ostream &out) {
    // LAndExp → EqExp | LAndExp '&&' EqExp
    // 左递归转变为循环
    for (int i = 0; i < eq_exps.size(); i++) {
        if (eq_exps[i]) {
            eq_exps[i]->print(out);
            out << "<LAndExp>" << std::endl;
            if (i < eq_exps.size() - 1) {
                out << tokenTypeToString(Token::TokenType::AND) << " " << "&&" << std::endl;
            }
        }
    }
}

void LOrExp::print(std::ostream &out) {
    // LOrExp → LAndExp | LOrExp '||' LAndExp
    // 左递归转变为循环
    for (int i = 0; i < land_exps.size(); i++) {
        if (land_exps[i]) {
            land_exps[i]->print(out);
            out << "<LOrExp>" << std::endl;
            if (i < land_exps.size() - 1) {
                out << tokenTypeToString(Token::TokenType::OR) << " " << "||" << std::endl;
            }   
        }
    }
}

void ConstExp::print(std::ostream &out) {
    // ConstExp → AddExp 注：使用的 Ident 必须是常量
    add_exp->print(out);
    out << "<ConstExp>" << std::endl;
}

void Ident::print(std::ostream &out) {
    out << tokenTypeToString(Token::TokenType::IDENFR) << " " << name << std::endl;
}

// 辅助输出variant类型--------------------------------------------------------------------------------------------
void print_decl(Decl &decl, std::ostream &out) {
    if (std::holds_alternative<ConstDecl>(decl)) {
        std::get<ConstDecl>(decl).print(out);
    } else if (std::holds_alternative<VarDecl>(decl)) {
        std::get<VarDecl>(decl).print(out);
    }
}

void print_block_item(BlockItem &block_item, std::ostream &out) {
    if (std::holds_alternative<Decl>(block_item)) {
        print_decl(std::get<Decl>(block_item), out);
    } else if (std::holds_alternative<Stmt>(block_item)) {
        print_stmt(std::get<Stmt>(block_item), out);
    }
}

void print_stmt(Stmt &stmt, std::ostream &out) {
    if (std::holds_alternative<LValStmt>(stmt)) {
        std::get<LValStmt>(stmt).print(out);
    } else if (std::holds_alternative<ExpStmt>(stmt)) {
        std::get<ExpStmt>(stmt).print(out);
    } else if (std::holds_alternative<BlockStmt>(stmt)) {
        std::get<BlockStmt>(stmt).print(out);
    } else if (std::holds_alternative<IfStmt>(stmt)) {
        std::get<IfStmt>(stmt).print(out);
    } else if (std::holds_alternative<ForCondStmt>(stmt)) {
        std::get<ForCondStmt>(stmt).print(out);
    } else if (std::holds_alternative<BreakStmt>(stmt)) {
        std::get<BreakStmt>(stmt).print(out);
    } else if (std::holds_alternative<ContinueStmt>(stmt)) {
        std::get<ContinueStmt>(stmt).print(out);
    } else if (std::holds_alternative<ReturnStmt>(stmt)) {
        std::get<ReturnStmt>(stmt).print(out);
    } else if (std::holds_alternative<PrintfStmt>(stmt)) {
        std::get<PrintfStmt>(stmt).print(out);
    }
}

void print_primary_exp_content(PrimaryExpContent &primary_exp_content, std::ostream &out) {
    if (std::holds_alternative<Exp>(primary_exp_content)) {
        out << tokenTypeToString(Token::TokenType::LPARENT) << " " << "(" << std::endl;
        std::get<Exp>(primary_exp_content).print(out); 
        out << tokenTypeToString(Token::TokenType::RPARENT) << " " << ")" << std::endl;
    } else if (std::holds_alternative<LVal>(primary_exp_content)) {
        std::get<LVal>(primary_exp_content).print(out);
    } else if (std::holds_alternative<Number>(primary_exp_content)) {
        std::get<Number>(primary_exp_content).print(out);
    }
}

void print_unary_exp(UnaryExp &unary_exp, std::ostream &out) {
    if (std::holds_alternative<PrimaryUnaryExp>(unary_exp)) {
        std::get<PrimaryUnaryExp>(unary_exp).print(out);
    } else if (std::holds_alternative<FuncCallExp>(unary_exp)) {
        std::get<FuncCallExp>(unary_exp).print(out);
    } else if (std::holds_alternative<UnaryOpExp>(unary_exp)) {
        std::get<UnaryOpExp>(unary_exp).print(out);
    }
}