#include "Lexer.hpp"
#include <string>
#include <cctype>
#include <iostream>

void Lexer::nextToken(Token &token) {
    token.is_error = false;
    // 跳过所有空白符和注释
    while (_isWhitespace() || _isComment()) {
        if (_isWhitespace()) {
            // 跳过空白符
            _skipWhitespace();
        } else {
            // 跳过注释
            if (_curChar == '/' && _peekChar() == '/') {
                // 单行注释
                while (_curChar!= '\n' && _curChar != EOF) {
                    _nextChar();
                }
            } else if (_curChar == '/' && _peekChar() == '*') {
                // 多行注释
                _nextChar();
                _nextChar();
                while (true) {
                    if (_curChar == EOF) {
                        break;
                    }
                    if (_curChar == '*' && _peekChar() == '/') {
                        _nextChar();
                        _nextChar();
                        break;
                    } else if (_curChar == '\n') {
                        _lineno++;
                        _nextChar();
                    } else {
                        _nextChar();
                    }
                }
            }
        }
    }

    // 处理标识符或关键字
    if (isalpha(_curChar) || _curChar == '_') {
        std::string word;
        do {
            word += _curChar;
            _nextChar();
        } while (isalnum(_curChar) || _curChar == '_' || isdigit(_curChar));
        auto it = keyword_map.find(word);
        if (it == keyword_map.end()) {
            // 标识符
            token.type = Token::TokenType::IDENFR;
        } else {
            // 关键字
            token.type = it->second;
        }
        token.content = word;
        token.lineno = _lineno;
    } 
    // 处理数字常量
    else if (isdigit(_curChar)) {
        std::string num;
        do {
            num += _curChar;
            _nextChar();
        } while (isdigit(_curChar));
        token.type = Token::TokenType::INTCON;
        token.content = num;
        token.lineno = _lineno;
    }
    // 处理字符串常量
    else if (_curChar == '\"') {
        std::string str;
        do {
            str += _curChar;
            _nextChar();
        } while (_curChar != '\"');
        str += _curChar;
        _nextChar();
        token.type = Token::TokenType::STRCON;
        token.content = str;
        token.lineno = _lineno;
        // std::cout << "string constant: " << token.content << std::endl;
    }
    // 处理流末尾
    else if (_curChar == EOF) {
        token.type = Token::TokenType::TK_EOF;
        token.content = "";
        token.lineno = _lineno;
    }
    // 处理符号
    else {
        switch (_curChar) {
            case '!': {
                char next = _peekChar();
                if (next == '=') {
                    token.type = Token::TokenType::NEQ;
                    token.content = "!=";
                    token.lineno = _lineno;
                    _nextChar();
                    _nextChar();
                } else {
                    token.type = Token::TokenType::NOT;
                    token.content = _curChar;
                    token.lineno = _lineno;
                    _nextChar();
                }
                break;
            }
            case '&': {
                char next = _peekChar();
                if (next == '&') {
                    token.type = Token::TokenType::AND;
                    token.content = "&&";
                    token.lineno = _lineno;
                    _nextChar();
                    _nextChar();
                } else {
                    token.type = Token::TokenType::AND;
                    token.content = _curChar;
                    token.lineno = _lineno;
                    token.is_error = true;
                    _nextChar();
                }
                break;
            }
            case '|': {
                char next = _peekChar();
                if (next == '|') {
                    token.type = Token::TokenType::OR;
                    token.content = "||";
                    token.lineno = _lineno;
                    _nextChar();
                    _nextChar();
                } else {
                    token.type = Token::TokenType::OR;
                    token.content = _curChar;
                    token.lineno = _lineno;
                    token.is_error = true;
                    _nextChar();
                }
                break;
            }
            case '+': {
                token.type = Token::TokenType::PLUS;
                token.content = _curChar;
                token.lineno = _lineno;
                _nextChar();
                break;
            }
            case '-': {
                token.type = Token::TokenType::MINU;
                token.content = _curChar;
                token.lineno = _lineno;
                _nextChar();
                break;
            }
            case '*': {
                token.type = Token::TokenType::MULT;
                token.content = _curChar;
                token.lineno = _lineno;
                _nextChar();
                break;
            }
            case '/': {
                token.type = Token::TokenType::DIV;
                token.content = _curChar;
                token.lineno = _lineno;
                _nextChar();
                break;
            }
            case '%': {
                token.type = Token::TokenType::MOD;
                token.content = _curChar;
                token.lineno = _lineno;
                _nextChar();
                break;
            }
            case '<': {
                char next = _peekChar();
                if (next == '=') {
                    token.type = Token::TokenType::LEQ;
                    token.content = "<=";
                    token.lineno = _lineno;
                    _nextChar();
                    _nextChar();
                } else {
                    token.type = Token::TokenType::LSS;
                    token.content = _curChar;
                    token.lineno = _lineno;
                    _nextChar();
                }
                break;
            }
            case '>': {
                char next = _peekChar();
                if (next == '=') {
                    token.type = Token::TokenType::GEQ;
                    token.content = ">=";
                    token.lineno = _lineno;
                    _nextChar();
                    _nextChar();
                } else {
                    token.type = Token::TokenType::GRE;
                    token.content = _curChar;
                    token.lineno = _lineno;
                    _nextChar();
                }
                break;
            }
            case '=': {
                char next = _peekChar();
                if (next == '=') {
                    token.type = Token::TokenType::EQL;
                    token.content = "==";
                    token.lineno = _lineno;
                    _nextChar();
                    _nextChar();
                } else {
                    token.type = Token::TokenType::ASSIGN;
                    token.content = _curChar;
                    token.lineno = _lineno;
                    _nextChar();
                }
                break;
            }
            case ';': {
                token.type = Token::TokenType::SEMICN;
                token.content = _curChar;
                token.lineno = _lineno;
                _nextChar();
                break;
            }
            case ',': {
                token.type = Token::TokenType::COMMA;
                token.content = _curChar;
                token.lineno = _lineno;
                _nextChar();
                break;
            }
            case '(': {
                token.type = Token::TokenType::LPARENT;
                token.content = _curChar;
                token.lineno = _lineno;
                _nextChar();
                break;
            }
            case ')': {
                token.type = Token::TokenType::RPARENT;
                token.content = _curChar;
                token.lineno = _lineno;
                _nextChar();
                break;
            }
            case '{': {
                token.type = Token::TokenType::LBRACE;
                token.content = _curChar;
                token.lineno = _lineno;
                _nextChar();
                break;
            }
            case '}': {
                token.type = Token::TokenType::RBRACE;
                token.content = _curChar;   
                token.lineno = _lineno;
                _nextChar();
                break;
            }
            case '[': {
                token.type = Token::TokenType::LBRACK;
                token.content = _curChar;
                token.lineno = _lineno;
                _nextChar();
                break;
            }
            case ']': {
                token.type = Token::TokenType::RBRACK;
                token.content = _curChar;
                token.lineno = _lineno;
                _nextChar();
                break;
            }
        }
    }
    // std::cout << "token: " << token.type << " " << token.content << " " << token.lineno << std::endl;
}

void Lexer::_skipWhitespace() {
    while (isspace(_curChar)) {
        if (_curChar == '\n') {
            _lineno++;
        }
        _nextChar();
    }
}

void Lexer::_nextChar() {
    _curChar = _input.get();
}

char Lexer::_peekChar() {
    char c = _input.peek();
    return c;
}

bool Lexer::_isWhitespace() {
    return isspace(_curChar);
}

bool Lexer::_isComment() {
    char next = _peekChar();
    if (_curChar == '/' && (next == '/' || next == '*')) {
        return true;
    } else {
        return false;
    }
}