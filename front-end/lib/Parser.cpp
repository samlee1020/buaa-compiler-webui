#include "Parser.hpp"
#include "Token.hpp"

void Parser::_next_token() {
    _last_token = _cur_token;
    if (!_peek_tokens_list.empty()) {
        _cur_token = _peek_tokens_list.front();
        _peek_tokens_list.pop_front();
    } else {
        _lexer.nextToken(_cur_token);
    }
    if (_cur_token.is_error) {
        _error_out << _cur_token.lineno << " " << "a:非法符号\'&\'或\'|\'" << std::endl;
    }
}

Token Parser::_peek_token(int offset) {
    Token token;
    if (offset > _peek_tokens_list.size()) {
        // 偏移量大于_peek_tokens_list的大小，说明需要从lexer中获取token
        int num_to_peek = offset - _peek_tokens_list.size();
        for (int i = 0; i < num_to_peek; i++) {
            Token peek_token;
            _lexer.nextToken(peek_token);
            _peek_tokens_list.push_back(peek_token);
        }
    }
    token = _peek_tokens_list[offset - 1];
    return token;
}

void Parser::_match(Token::TokenType type) {
    if (_cur_token.type == type) {
        _next_token();
    } else {
        if (type == Token::TokenType::SEMICN) {
            _error_out << _last_token.lineno << " " << "i:缺少分号" << std::endl;
        } else if (type == Token::TokenType::RPARENT) {
            _error_out << _last_token.lineno << " " << "j:缺少右括号" << std::endl;
        } else if (type == Token::TokenType::RBRACK) {
            _error_out << _last_token.lineno << " " << "k:缺少右中括号" << std::endl;
        }
    }
}

std::unique_ptr<CompUnit> Parser::parse() {
    return _parse_comp_unit();
}

std::unique_ptr<CompUnit> Parser::_parse_comp_unit() {
    // CompUnit → {Decl} {FuncDef} MainFuncDef
    // 解析decl
    // std::cout << "parse_comp_unit1" << std::endl;
    std::unique_ptr<CompUnit> compUnit = std::make_unique<CompUnit>();
    compUnit->lineno = _cur_token.lineno;
    while (_is_decl_of_comp_unit()) {
        std::unique_ptr<Decl> decl = _parse_decl();
        compUnit->decls.push_back(std::move(decl));
    }
    // 解析funcDef
    // std::cout << "parse_comp_unit2" << std::endl;
    while(_is_func_def_of_comp_unit()) {
        std::unique_ptr<FuncDef> funcDef = _parse_func_def();
        compUnit->funcDefs.push_back(std::move(funcDef));
    }
    // 解析mainFuncDef
    // std::cout << "parse_comp_unit3" << std::endl;
    std::unique_ptr<MainFuncDef> mainFuncDef = _parse_main_func_def();
    compUnit->mainFuncDef = std::move(mainFuncDef);
    return compUnit;
}
bool Parser::_is_decl_of_comp_unit() {
    if (_peek_token(2).type == Token::TokenType::LPARENT) {
        // _cur_token后面第2个token是左括号，说明是函数声明或main函数
        return false;
    }
    return true;
}
bool Parser::_is_func_def_of_comp_unit() {
    if (_peek_token(1).type == Token::TokenType::MAINTK) {    
        // _cur_token后面第1个token是main，说明是main函数
        return false;
    }
    return true;
}

std::unique_ptr<Decl> Parser::_parse_decl() {
    // Decl → ConstDecl | VarDecl
    std::unique_ptr<Decl> decl;
    if (_cur_token.type == Token::TokenType::CONSTTK) {
        // 是ConstDecl
        std::unique_ptr<ConstDecl> constDecl = _parse_const_decl();
        decl = std::make_unique<Decl>(std::move(*constDecl));
    } else {
        // 是VarDecl
        std::unique_ptr<VarDecl> varDecl = _parse_var_decl();
        decl = std::make_unique<Decl>(std::move(*varDecl));
    }
    return decl;
}

std::unique_ptr<ConstDecl> Parser::_parse_const_decl() {
    // ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'
    std::unique_ptr<ConstDecl> constDecl = std::make_unique<ConstDecl>();
    constDecl->lineno = _cur_token.lineno;
    _next_token(); // 跳过const
    _next_token(); // 跳过BType，即int
    constDecl->const_defs.push_back(_parse_const_def());
    while (_cur_token.type == Token::TokenType::COMMA) {
        _next_token(); // 跳过逗号
        constDecl->const_defs.push_back(_parse_const_def());
    }
    _match(Token::TokenType::SEMICN); // 跳过分号
    return constDecl;
}

std::unique_ptr<ConstDef> Parser::_parse_const_def() {
    // ConstDef → Ident [ '[' ConstExp ']' ] '=' ConstInitVal
    std::unique_ptr<ConstDef> constDef = std::make_unique<ConstDef>();
    constDef->lineno = _cur_token.lineno;
    constDef->ident = _parse_ident();
    if (_cur_token.type == Token::TokenType::LBRACK) {
        // 数组
        _next_token(); // 跳过左中括号
        constDef->const_exp = _parse_const_exp();
        _match(Token::TokenType::RBRACK); // 跳过右中括号
    }
    _next_token(); // 跳过等号
    constDef->const_init_val = _parse_const_init_val();
    return constDef;
}

std::unique_ptr<ConstInitVal> Parser::_parse_const_init_val() {
    // ConstInitVal → ConstExp | '{' [ ConstExp { ',' ConstExp } ] '}'
    std::unique_ptr<ConstInitVal> constInitVal = std::make_unique<ConstInitVal>();
    constInitVal->lineno = _cur_token.lineno;
    if (_cur_token.type == Token::TokenType::LBRACE) {
        // 数组
        constInitVal->is_array = true;
        _next_token(); // 跳过左大括号
        if (_cur_token.type == Token::TokenType::RBRACE) {
            // 空数组
            _next_token(); // 跳过右大括号
        } else {
            constInitVal->const_exps.push_back(_parse_const_exp());
            while (_cur_token.type == Token::TokenType::COMMA) {
                _next_token(); // 跳过逗号
                constInitVal->const_exps.push_back(_parse_const_exp());
            }
            _next_token(); // 跳过右大括号
        }
    } else {
        // 单个值
        constInitVal->is_array = false;
        constInitVal->const_exps.push_back(_parse_const_exp());
    }
    return constInitVal;
}

std::unique_ptr<VarDecl> Parser::_parse_var_decl() {
    // VarDecl → [ 'static' ] BType VarDef { ',' VarDef } ';'
    std::unique_ptr<VarDecl> varDecl = std::make_unique<VarDecl>();
    varDecl->lineno = _cur_token.lineno;
    if (_cur_token.type == Token::TokenType::STATICTK) {
        // 说明是静态变量
        varDecl->is_static = true;
        _next_token(); // 跳过static
    }
    _next_token(); // 跳过BType，即int
    varDecl->var_defs.push_back(_parse_var_def());
    while (_cur_token.type == Token::TokenType::COMMA) {
        _next_token(); // 跳过逗号
        varDecl->var_defs.push_back(_parse_var_def());
    }
    _match(Token::TokenType::SEMICN); // 跳过分号
    return varDecl;
}

std::unique_ptr<VarDef> Parser::_parse_var_def() {
    // VarDef → Ident [ '[' ConstExp ']' ] | Ident [ '[' ConstExp ']' ] '=' InitVal
    std::unique_ptr<VarDef> varDef = std::make_unique<VarDef>();
    varDef->lineno = _cur_token.lineno;
    varDef->ident = _parse_ident();
    if (_cur_token.type == Token::TokenType::LBRACK) {
        // 数组
        _next_token(); // 跳过左中括号
        varDef->const_exp = _parse_const_exp();
        _match(Token::TokenType::RBRACK); // 跳过右中括号
    }
    if (_cur_token.type == Token::TokenType::ASSIGN) {
        // 有初始化值
        _next_token(); // 跳过等号
        varDef->init_val = _parse_init_val();
    }
    return varDef;
}

std::unique_ptr<InitVal> Parser::_parse_init_val() {
    // InitVal → Exp | '{' [ Exp { ',' Exp } ] '}'
    std::unique_ptr<InitVal> initVal = std::make_unique<InitVal>();
    initVal->lineno = _cur_token.lineno;
    if (_cur_token.type == Token::TokenType::LBRACE) {
        // 数组
        initVal->is_array = true;
        _next_token(); // 跳过左大括号
        if (_cur_token.type == Token::TokenType::RBRACE) {
            // 空数组
            _next_token(); // 跳过右大括号
        } else {
            initVal->exps.push_back(_parse_exp());
            while (_cur_token.type == Token::TokenType::COMMA) {
                _next_token(); // 跳过逗号
                initVal->exps.push_back(_parse_exp());
            }
            _next_token(); // 跳过右大括号
        }
    } else {
        // 单个值
        initVal->is_array = false;
        initVal->exps.push_back(_parse_exp());
    }
    return initVal;
}

std::unique_ptr<FuncDef> Parser::_parse_func_def() {
    // FuncDef → FuncType Ident '(' [FuncFParams] ')' Block
    std::unique_ptr<FuncDef> funcDef = std::make_unique<FuncDef>();
    funcDef->lineno = _cur_token.lineno;
    funcDef->func_type = _parse_func_type();
    funcDef->ident = _parse_ident();
    _next_token(); // 跳过左括号
    if (_cur_token.type != Token::TokenType::INTTK) {
        // 没有参数
        _match(Token::TokenType::RPARENT); // 跳过右括号
    } else {
        // 有参数
        funcDef->func_fparams = _parse_func_fparams();
        _match(Token::TokenType::RPARENT); // 跳过右括号
    }
    funcDef->block = _parse_block();
    return funcDef;
}

std::unique_ptr<MainFuncDef> Parser::_parse_main_func_def() {
    // MainFuncDef → 'int' 'main' '(' ')' Block
    std::unique_ptr<MainFuncDef> mainFuncDef = std::make_unique<MainFuncDef>();
    mainFuncDef->lineno = _cur_token.lineno;
    _next_token(); // 跳过int
    _next_token(); // 跳过main
    _next_token(); // 跳过左括号
    _match(Token::TokenType::RPARENT); // 跳过右括号
    mainFuncDef->block = _parse_block();
    return mainFuncDef;
}

std::unique_ptr<FuncType> Parser::_parse_func_type() {
    // FuncType → 'int' | 'void'
    std::unique_ptr<FuncType> funcType = std::make_unique<FuncType>();
    funcType->lineno = _cur_token.lineno;
    if (_cur_token.type == Token::TokenType::INTTK) {
        // 是int
        _next_token(); // 跳过int
        funcType->is_int = true;
    } else {
        // 是void
        _next_token(); // 跳过void
        funcType->is_int = false;
    }
    return funcType;
}

std::unique_ptr<FuncFParams> Parser::_parse_func_fparams() {
     // FuncFParams → FuncFParam { ',' FuncFParam }
    std::unique_ptr<FuncFParams> funcFParams = std::make_unique<FuncFParams>();
    funcFParams->lineno = _cur_token.lineno;
    funcFParams->func_fparams.push_back(_parse_func_fparam());
    while (_cur_token.type == Token::TokenType::COMMA) {
        _next_token(); // 跳过逗号
        funcFParams->func_fparams.push_back(_parse_func_fparam());
    }
    return funcFParams;
}

std::unique_ptr<FuncFParam> Parser::_parse_func_fparam() {
    // FuncFParam → BType Ident ['[' ']']
    std::unique_ptr<FuncFParam> funcFParam = std::make_unique<FuncFParam>();
    funcFParam->lineno = _cur_token.lineno;
    _next_token(); // 跳过BType，即int
    funcFParam->ident = _parse_ident();
    if (_cur_token.type == Token::TokenType::LBRACK) {
        // 数组
        _next_token(); // 跳过左中括号
        _match(Token::TokenType::RBRACK); // 跳过右中括号
        funcFParam->is_array = true;
    } else {
        funcFParam->is_array = false;
    }
    return funcFParam;
}

std::unique_ptr<Block> Parser::_parse_block() {
    // Block → '{' { BlockItem } '}'
    std::unique_ptr<Block> block = std::make_unique<Block>();
    block->lineno = _cur_token.lineno;
    _next_token(); // 跳过左大括号
    while (_cur_token.type != Token::TokenType::RBRACE) {
        block->block_items.push_back(_parse_block_item());
    }
    block->lineno_of_end = _cur_token.lineno; // 记录结束行号
    _next_token(); // 跳过右大括号
    return block;
}

std::unique_ptr<BlockItem> Parser::_parse_block_item() {
    // BlockItem → Decl | Stmt
    std::unique_ptr<BlockItem> blockItem;
    if (_is_decl_of_block_item()) {
        // 是Decl
        std::unique_ptr<Decl> decl = _parse_decl();
        blockItem = std::make_unique<BlockItem>(std::move(*decl));
    } else {
        // 是Stmt
        std::unique_ptr<Stmt> stmt = _parse_stmt();
        blockItem = std::make_unique<BlockItem>(std::move(*stmt));
    }
    return blockItem;
}
bool Parser::_is_decl_of_block_item() {
    // decl会以 "const" 、 "static" 、 "int" 开头
    if (_cur_token.type == Token::TokenType::CONSTTK ||
        _cur_token.type == Token::TokenType::STATICTK ||
        _cur_token.type == Token::TokenType::INTTK) {
        return true;
    }
    return false;
}

std::unique_ptr<Stmt> Parser::_parse_stmt() {
    // Stmt → LValStmt | ExpStmt | BlockStmt | IfStmt | ForCondStmt | BreakStmt | ContinueStmt | ReturnStmt | PrintfStmt
    std::unique_ptr<Stmt> stmt;
    if (_cur_token.type == Token::TokenType::LBRACE) { // 遇到"{"
        std::unique_ptr<BlockStmt> blockStmt = _parse_block_stmt();
        stmt = std::make_unique<Stmt>(std::move(*blockStmt));
    } else if (_cur_token.type == Token::TokenType::IFTK) { // 遇到"if"
        std::unique_ptr<IfStmt> ifStmt = _parse_if_stmt();
        stmt = std::make_unique<Stmt>(std::move(*ifStmt));
    } else if (_cur_token.type == Token::TokenType::FORTK) { // 遇到"for"
        std::unique_ptr<ForCondStmt> forCondStmt = _parse_for_cond_stmt();
        stmt = std::make_unique<Stmt>(std::move(*forCondStmt));
    } else if (_cur_token.type == Token::TokenType::BREAKTK) { // 遇到"break"
        std::unique_ptr<BreakStmt> breakStmt = _parse_break_stmt();
        stmt = std::make_unique<Stmt>(std::move(*breakStmt));
    } else if (_cur_token.type == Token::TokenType::CONTINUETK) { // 遇到"continue"
        std::unique_ptr<ContinueStmt> continueStmt = _parse_continue_stmt();
        stmt = std::make_unique<Stmt>(std::move(*continueStmt));
    } else if (_cur_token.type == Token::TokenType::RETURNTK) { // 遇到"return"
        std::unique_ptr<ReturnStmt> returnStmt = _parse_return_stmt();
        stmt = std::make_unique<Stmt>(std::move(*returnStmt));
    } else if (_cur_token.type == Token::TokenType::PRINTFTK) { // 遇到"printf"
        std::unique_ptr<PrintfStmt> printfStmt = _parse_printf_stmt();
        stmt = std::make_unique<Stmt>(std::move(*printfStmt));
    }
    // 区分LValStmt和ExpStmt，前者在有赋值号，后者没有
    else if (_is_lval_stmt_of_stmt()) {
        std::unique_ptr<LValStmt> lValStmt = _parse_lval_stmt();
        stmt = std::make_unique<Stmt>(std::move(*lValStmt));
    } else {
        std::unique_ptr<ExpStmt> expStmt = _parse_exp_stmt();
        stmt = std::make_unique<Stmt>(std::move(*expStmt));
    }
    return stmt;
}
bool Parser::_is_lval_stmt_of_stmt() {
    if (_cur_token.type != Token::TokenType::IDENFR) {
        // 如果不是标识符开始，则一定不是LValStmt
        return false;
    } 
    if (_peek_token(1).type == Token::TokenType::LPARENT) {
        // 如果是函数调用，则一定不是LValStmt
        return false;
    }

    int i = 1;
    if (_peek_token(i).type == Token::TokenType::LBRACK) {
        // 遇到左中括号，说明是数组，需要跳过中括号部分
        i++;
        int lbrack_num = 1;
        while (lbrack_num > 0) {
            if (_peek_token(i).type == Token::TokenType::ASSIGN) {
                // 匹配完右中括号就遇到赋值号，说明是LValStmt，且LVal缺少了右中括号
                return true;
            }
            else if (_peek_token(i).type == Token::TokenType::SEMICN) {
                // 没匹配完右中括号就遇到分号，说明不是LValStmt
                return false;
            }
            else if (_peek_token(i).type == Token::TokenType::LBRACK) {
                // 嵌套中括号
                lbrack_num++;
            } else if (_peek_token(i).type == Token::TokenType::RBRACK) {
                // 匹配右中括号
                lbrack_num--;
            }   
            i++;
        }
        // 跳过右中括号
        if (_peek_token(i).type == Token::TokenType::RBRACK) {
            i++;
        }
    }
    // 看看有没有赋值号
    if (_peek_token(i).type == Token::TokenType::ASSIGN) {
        return true;
    } else {
        return false;
    }
}

std::unique_ptr<LValStmt> Parser::_parse_lval_stmt() {
    // LValStmt → LVal '=' Exp ';'
    std::unique_ptr<LValStmt> lValStmt = std::make_unique<LValStmt>();
    lValStmt->lineno = _cur_token.lineno;
    lValStmt->lval = _parse_lval();
    _next_token(); // 跳过等号
    lValStmt->exp = _parse_exp();
    _match(Token::TokenType::SEMICN); // 跳过分号
    return lValStmt;
}

std::unique_ptr<ExpStmt> Parser::_parse_exp_stmt() {
    // ExpStmt → [Exp] ';'
    std::unique_ptr<ExpStmt> expStmt = std::make_unique<ExpStmt>();
    expStmt->lineno = _cur_token.lineno;
    if (_cur_token.type == Token::TokenType::SEMICN) {
        // 空语句
        _next_token(); // 跳过分号
        return expStmt;
    } else {
        expStmt->exp = _parse_exp();
        _match(Token::TokenType::SEMICN); // 跳过分号
        return expStmt;
    }
}

std::unique_ptr<BlockStmt> Parser::_parse_block_stmt() {
    // BlockStmt → Block
    std::unique_ptr<BlockStmt> blockStmt = std::make_unique<BlockStmt>();
    blockStmt->lineno = _cur_token.lineno;
    blockStmt->block = _parse_block();
    return blockStmt;
}

std::unique_ptr<IfStmt> Parser::_parse_if_stmt() {
    // IfStmt → 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
    std::unique_ptr<IfStmt> ifStmt = std::make_unique<IfStmt>();
    ifStmt->lineno = _cur_token.lineno;
    _next_token(); // 跳过if
    _next_token(); // 跳过左括号
    ifStmt->cond = _parse_cond();
    _match(Token::TokenType::RPARENT); // 跳过右括号
    ifStmt->stmt = _parse_stmt();
    if (_cur_token.type == Token::TokenType::ELSETK) {
        // 有else
        _next_token(); // 跳过else
        ifStmt->else_stmt = _parse_stmt();
    }
    return ifStmt;
}

std::unique_ptr<ForCondStmt> Parser::_parse_for_cond_stmt() {
    // ForCondStmt → 'for' '(' [ForStmt] ';' [Cond] ';' [ForStmt] ')' Stmt
    std::unique_ptr<ForCondStmt> forCondStmt = std::make_unique<ForCondStmt>();
    forCondStmt->lineno = _cur_token.lineno;
    _next_token(); // 跳过for
    _next_token(); // 跳过左括号
    if (_cur_token.type != Token::TokenType::SEMICN) {
        // 有初始化语句
        forCondStmt->for_stmt1 = _parse_for_stmt();
    }
    _next_token(); // 跳过分号
    if (_cur_token.type != Token::TokenType::SEMICN) {
        // 有条件语句
        forCondStmt->cond = _parse_cond();
    }
    _next_token(); // 跳过分号
    if (_cur_token.type != Token::TokenType::RPARENT) {
        // 有迭代语句
        forCondStmt->for_stmt2 = _parse_for_stmt();
    }
    _next_token(); // 跳过右括号
    forCondStmt->stmt = _parse_stmt();
    return forCondStmt;
}

std::unique_ptr<BreakStmt> Parser::_parse_break_stmt() {
    // BreakStmt → 'break' ';'
    std::unique_ptr<BreakStmt> breakStmt = std::make_unique<BreakStmt>();
    breakStmt->lineno = _cur_token.lineno;
    _next_token(); // 跳过break
    _match(Token::TokenType::SEMICN); // 跳过分号
    return breakStmt;
}

std::unique_ptr<ContinueStmt> Parser::_parse_continue_stmt() {
    // ContinueStmt → 'continue' ';'
    std::unique_ptr<ContinueStmt> continueStmt = std::make_unique<ContinueStmt>();
    continueStmt->lineno = _cur_token.lineno;
    _next_token(); // 跳过continue
    _match(Token::TokenType::SEMICN); // 跳过分号
    return continueStmt;
}

std::unique_ptr<ReturnStmt> Parser::_parse_return_stmt() {
    // ReturnStmt → 'return' [Exp] ';'
    std::unique_ptr<ReturnStmt> returnStmt = std::make_unique<ReturnStmt>();
    returnStmt->lineno = _cur_token.lineno;
    _next_token(); // 跳过return
    if (_cur_token.type != Token::TokenType::SEMICN) {
        // 有返回值
        returnStmt->exp = _parse_exp();
    }
    _match(Token::TokenType::SEMICN); // 跳过分号
    return returnStmt;
}

std::unique_ptr<PrintfStmt> Parser::_parse_printf_stmt() {
    // PrintfStmt → 'printf''('StringConst {','Exp}')'';'
    std::unique_ptr<PrintfStmt> printfStmt = std::make_unique<PrintfStmt>();
    printfStmt->lineno = _cur_token.lineno;
    _next_token(); // 跳过printf
    _next_token(); // 跳过左括号
    printfStmt->string_const = _cur_token.content;
    _next_token(); // 跳过StringConst
    while (_cur_token.type == Token::TokenType::COMMA) {
        _next_token(); // 跳过逗号
        printfStmt->exps.push_back(_parse_exp());
    }
    _match(Token::TokenType::RPARENT); // 跳过右括号
    _match(Token::TokenType::SEMICN); // 跳过分号
    return printfStmt;
}

std::unique_ptr<ForStmt> Parser::_parse_for_stmt() {
    // ForStmt → LVal '=' Exp { ',' LVal '=' Exp }
    std::unique_ptr<ForStmt> forStmt = std::make_unique<ForStmt>();
    forStmt->lineno = _cur_token.lineno;
    forStmt->lvals.push_back(_parse_lval());
    _next_token(); // 跳过等号
    forStmt->exps.push_back(_parse_exp());
    while (_cur_token.type == Token::TokenType::COMMA) {
        _next_token(); // 跳过逗号
        forStmt->lvals.push_back(_parse_lval());
        _next_token(); // 跳过等号
        forStmt->exps.push_back(_parse_exp());
    }
    return forStmt;
}

std::unique_ptr<Exp> Parser::_parse_exp() {
    // Exp → AddExp
    std::unique_ptr<Exp> exp = std::make_unique<Exp>();
    exp->lineno = _cur_token.lineno;
    exp->add_exp = _parse_add_exp();
    return exp;
}

std::unique_ptr<Cond> Parser::_parse_cond() {
    // Cond → LOrExp
    std::unique_ptr<Cond> cond = std::make_unique<Cond>();
    cond->lineno = _cur_token.lineno;
    cond->lor_exp = _parse_lor_exp();
    return cond;
}

std::unique_ptr<LVal> Parser::_parse_lval() {
    // LVal → Ident [ '[' Exp ']' ]
    std::unique_ptr<LVal> lVal = std::make_unique<LVal>();
    lVal->lineno = _cur_token.lineno;
    lVal->ident = _parse_ident();
    if (_cur_token.type == Token::TokenType::LBRACK) {
        // 数组
        _next_token(); // 跳过左中括号
        lVal->exp = _parse_exp();
        _match(Token::TokenType::RBRACK); // 跳过右中括号
    }
    return lVal;
}

std::unique_ptr<PrimaryExp> Parser::_parse_primary_exp() {
    // PrimaryExp → '(' Exp ')' | LVal | Number
    std::unique_ptr<PrimaryExp> primaryExp = std::make_unique<PrimaryExp>();
    primaryExp->lineno = _cur_token.lineno;
    if (_cur_token.type == Token::TokenType::LPARENT) {
        // 左括号
        _next_token(); // 跳过左括号
        std::unique_ptr<Exp> exp = _parse_exp();
        _match(Token::TokenType::RPARENT); // 跳过右括号
        primaryExp->content = std::make_unique<PrimaryExpContent>(std::move(*exp));
    } else if (_cur_token.type == Token::TokenType::INTCON) {
        // 数字
        std::unique_ptr<Number> number = _parse_number();
        primaryExp->content = std::make_unique<PrimaryExpContent>(std::move(*number));
    } else {
        // 变量
        std::unique_ptr<LVal> lVal = _parse_lval();
        primaryExp->content = std::make_unique<PrimaryExpContent>(std::move(*lVal));
    }
    return primaryExp;
}

std::unique_ptr<Number> Parser::_parse_number() {
    // Number → INTCON
    std::unique_ptr<Number> number = std::make_unique<Number>();
    number->lineno = _cur_token.lineno;
    number->int_const = _cur_token.content;
    _next_token(); // 跳过数字
    return number;
}

std::unique_ptr<UnaryExp> Parser::_parse_unary_exp() {
    //UnaryExp → PrimaryUnaryExp | FuncCallExp | UnaryOpExp
    std::unique_ptr<UnaryExp> unaryExp;
    if (_cur_token.type == Token::TokenType::PLUS ||
        _cur_token.type == Token::TokenType::MINU ||
        _cur_token.type == Token::TokenType::NOT) {
        // 单目运算符，是UnaryOpExp
        std::unique_ptr<UnaryOpExp> unaryOpExp = _parse_unary_op_exp();
        unaryExp = std::make_unique<UnaryExp>(std::move(*unaryOpExp));
    } else if (_cur_token.type == Token::TokenType::IDENFR && _peek_token(1).type == Token::TokenType::LPARENT) {
        // 函数调用，是FuncCallExp
        std::unique_ptr<FuncCallExp> funcCallExp = _parse_func_call_exp();
        unaryExp = std::make_unique<UnaryExp>(std::move(*funcCallExp));
    } else {
        // 初等表达式，是PrimaryUnaryExp
        std::unique_ptr<PrimaryUnaryExp> primaryUnaryExp = _parse_primary_unary_exp();
        unaryExp = std::make_unique<UnaryExp>(std::move(*primaryUnaryExp));
    }
    return unaryExp;
} 

std::unique_ptr<PrimaryUnaryExp> Parser::_parse_primary_unary_exp() {
    // PrimaryUnaryExp → PrimaryExp 
    std::unique_ptr<PrimaryUnaryExp> primaryUnaryExp = std::make_unique<PrimaryUnaryExp>();
    primaryUnaryExp->lineno = _cur_token.lineno;
    primaryUnaryExp->primary_exp = _parse_primary_exp();
    return primaryUnaryExp;
}

std::unique_ptr<FuncCallExp> Parser::_parse_func_call_exp() {
    // FuncCallExp → Ident '(' [FuncRParams] ')'
    std::unique_ptr<FuncCallExp> funcCallExp = std::make_unique<FuncCallExp>();
    funcCallExp->lineno = _cur_token.lineno;
    funcCallExp->ident = _parse_ident();
    _next_token(); // 跳过左括号
    if (_is_func_rparams_of_func_call_exp()) {
        // 有参数
        funcCallExp->func_rparams = _parse_func_rparams();
    }
    _match(Token::TokenType::RPARENT); // 跳过右括号
    return funcCallExp;
}
bool Parser::_is_func_rparams_of_func_call_exp() {
    if (_cur_token.type == Token::TokenType::LPARENT ||
        _cur_token.type == Token::TokenType::IDENFR ||
        _cur_token.type == Token::TokenType::INTCON ||
        _cur_token.type == Token::TokenType::PLUS ||
        _cur_token.type == Token::TokenType::MINU ||
        _cur_token.type == Token::TokenType::NOT ) {
        // func_rparams由exp组成，以上是exp的开始符号
        return true;
    } else {
        return false;
    }
}

std::unique_ptr<UnaryOpExp> Parser::_parse_unary_op_exp() {
    // UnaryOpExp → UnaryOp UnaryExp
    std::unique_ptr<UnaryOpExp> unaryOpExp = std::make_unique<UnaryOpExp>();
    unaryOpExp->lineno = _cur_token.lineno;
    unaryOpExp->unary_op = _parse_unary_op();
    unaryOpExp->unary_exp = _parse_unary_exp();
    return unaryOpExp;
}

std::unique_ptr<UnaryOp> Parser::_parse_unary_op() {
    // UnaryOp → '+' | '-' | '!'
    std::unique_ptr<UnaryOp> unaryOp = std::make_unique<UnaryOp>();
    unaryOp->lineno = _cur_token.lineno;
    unaryOp->op = _cur_token.content;
    _next_token(); // 跳过运算符
    return unaryOp;
}

std::unique_ptr<FuncRParams> Parser::_parse_func_rparams() {
    // FuncRParams → Exp { ',' Exp }
    std::unique_ptr<FuncRParams> funcRParams = std::make_unique<FuncRParams>();
    funcRParams->lineno = _cur_token.lineno;
    funcRParams->exps.push_back(_parse_exp());
    while (_cur_token.type == Token::TokenType::COMMA) {
        _next_token(); // 跳过逗号
        funcRParams->exps.push_back(_parse_exp());
    }
    return funcRParams;
}

std::unique_ptr<MulExp> Parser::_parse_mul_exp() {
    // MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
    // 左递归转变为循环
    std::unique_ptr<MulExp> mulExp = std::make_unique<MulExp>();
    mulExp->lineno = _cur_token.lineno;
    mulExp->unary_exps.push_back(_parse_unary_exp());
    while (_cur_token.type == Token::TokenType::MULT ||
        _cur_token.type == Token::TokenType::DIV ||
        _cur_token.type == Token::TokenType::MOD) {
        mulExp->ops.push_back(_cur_token.content);
        _next_token(); // 跳过运算符
        mulExp->unary_exps.push_back(_parse_unary_exp());
    }
    return mulExp;
}

std::unique_ptr<AddExp> Parser::_parse_add_exp() {
    // AddExp → MulExp | AddExp ('+' | '−') MulExp
    // 左递归转变为循环
    std::unique_ptr<AddExp> addExp = std::make_unique<AddExp>();
    addExp->lineno = _cur_token.lineno;
    addExp->mul_exps.push_back(_parse_mul_exp());
    while (_cur_token.type == Token::TokenType::PLUS ||
        _cur_token.type == Token::TokenType::MINU) {
        addExp->ops.push_back(_cur_token.content);
        _next_token(); // 跳过运算符
        addExp->mul_exps.push_back(_parse_mul_exp());
    }
    return addExp;
}

std::unique_ptr<RelExp>  Parser::_parse_rel_exp() {
    // RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
    // 左递归转变为循环
    std::unique_ptr<RelExp> relExp = std::make_unique<RelExp>();
    relExp->lineno = _cur_token.lineno;
    relExp->add_exps.push_back(_parse_add_exp());
    while (_cur_token.type == Token::TokenType::LSS ||
        _cur_token.type == Token::TokenType::GRE ||
        _cur_token.type == Token::TokenType::LEQ ||
        _cur_token.type == Token::TokenType::GEQ) {
        relExp->ops.push_back(_cur_token.content);
        _next_token(); // 跳过运算符
        relExp->add_exps.push_back(_parse_add_exp());
    }
    return relExp;
}

std::unique_ptr<EqExp> Parser::_parse_eq_exp() {
    // EqExp → RelExp | EqExp ('==' | '!=') RelExp
    // 左递归转变为循环
    std::unique_ptr<EqExp> eqExp = std::make_unique<EqExp>();
    eqExp->lineno = _cur_token.lineno;
    eqExp->rel_exps.push_back(_parse_rel_exp());
    while (_cur_token.type == Token::TokenType::EQL ||
        _cur_token.type == Token::TokenType::NEQ) {
        eqExp->ops.push_back(_cur_token.content);
        _next_token(); // 跳过运算符
        eqExp->rel_exps.push_back(_parse_rel_exp());
    }
    return eqExp;
}

std::unique_ptr<LAndExp> Parser::_parse_land_exp() {
    // LAndExp → EqExp | LAndExp '&&' EqExp
    // 左递归转变为循环
    std::unique_ptr<LAndExp> landExp = std::make_unique<LAndExp>();
    landExp->lineno = _cur_token.lineno;
    landExp->eq_exps.push_back(_parse_eq_exp());
    while (_cur_token.type == Token::TokenType::AND) {
        _next_token(); // 跳过运算符
        landExp->eq_exps.push_back(_parse_eq_exp());
    }
    return landExp;
}

std::unique_ptr<LOrExp> Parser::_parse_lor_exp() {
    // LOrExp → LAndExp | LOrExp '||' LAndExp
    // 左递归转变为循环
    std::unique_ptr<LOrExp> lorExp = std::make_unique<LOrExp>();
    lorExp->lineno = _cur_token.lineno;
    lorExp->land_exps.push_back(_parse_land_exp());
    while (_cur_token.type == Token::TokenType::OR) {
        _next_token(); // 跳过运算符
        lorExp->land_exps.push_back(_parse_land_exp());
    }
    return lorExp;
}

std::unique_ptr<ConstExp> Parser::_parse_const_exp() {
    // ConstExp → AddExp 注：使用的 Ident 必须是常量
    std::unique_ptr<ConstExp> constExp = std::make_unique<ConstExp>();
    constExp->lineno = _cur_token.lineno;
    constExp->add_exp = _parse_add_exp();
    return constExp;
}

std::unique_ptr<Ident> Parser::_parse_ident() {
    std::unique_ptr<Ident> ident = std::make_unique<Ident>();
    ident->lineno = _cur_token.lineno;
    ident->name = _cur_token.content;
    _next_token(); // 跳过标识符
    return ident;
}
