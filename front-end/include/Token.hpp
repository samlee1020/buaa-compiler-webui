#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <unordered_map>

// Token列表
#define TOKEN_TYPE \
    X(IDENFR, "IDENFR") \
    X(INTCON, "INTCON") \
    X(STRCON, "STRCON") \
    X(CONSTTK, "CONSTTK") \
    X(INTTK, "INTTK") \
    X(STATICTK, "STATICTK") \
    X(BREAKTK, "BREAKTK") \
    X(CONTINUETK, "CONTINUETK") \
    X(IFTK, "IFTK") \
    X(MAINTK, "MAINTK") \
    X(ELSETK, "ELSETK") \
    X(NOT, "NOT") \
    X(AND, "AND") \
    X(OR, "OR") \
    X(FORTK, "FORTK") \
    X(RETURNTK, "RETURNTK") \
    X(VOIDTK, "VOIDTK") \
    X(PLUS, "PLUS") \
    X(MINU, "MINU") \
    X(PRINTFTK, "PRINTFTK") \
    X(MULT, "MULT") \
    X(DIV, "DIV") \
    X(MOD, "MOD") \
    X(LSS, "LSS") \
    X(LEQ, "LEQ") \
    X(GRE, "GRE") \
    X(GEQ, "GEQ") \
    X(EQL, "EQL") \
    X(NEQ, "NEQ") \
    X(SEMICN, "SEMICN") \
    X(COMMA, "COMMA") \
    X(LPARENT, "LPARENT") \
    X(RPARENT, "RPARENT") \
    X(LBRACK, "LBRACK") \
    X(RBRACK, "RBRACK") \
    X(LBRACE, "LBRACE") \
    X(RBRACE, "RBRACE") \
    X(ASSIGN, "ASSIGN") \
    X(TK_EOF, "TK_EOF") \
    X(ERROR_A, "ERROR_A")

// Token结构体
struct Token {
    enum TokenType {
        #define X(a, b) a,
        TOKEN_TYPE
        #undef X
    } type;
    std::string content;
    int lineno;
    bool is_error = false;

    Token() = default;
    Token(TokenType type, const std::string& content, int lineno)
        : type(type), content(content), lineno(lineno) {}
};

// 关键字哈希表
static const std::unordered_map<std::string, Token::TokenType> keyword_map = {
    {"const", Token::CONSTTK},
    {"int", Token::INTTK},
    {"static", Token::STATICTK},
    {"break", Token::BREAKTK},
    {"continue", Token::CONTINUETK},
    {"if", Token::IFTK},
    {"main", Token::MAINTK},
    {"else", Token::ELSETK},
    {"for", Token::FORTK},
    {"return", Token::RETURNTK},
    {"void", Token::VOIDTK},
    {"printf", Token::PRINTFTK},
};

// 根据Token类型获取字符串
std::string tokenTypeToString(Token::TokenType type);

#endif