#ifndef LEXER_HPP
#define LEXER_HPP

#include "Token.hpp"
#include <istream>

class Lexer {
    public:
        Lexer(std::istream &in) : _input(in) , _lineno(1), _curChar(' ') {
            _nextChar();
        };

        void nextToken(Token &token); // 词法分析，生成下一个Token
    
    private:
        std::istream &_input;
        int _lineno;
        char _curChar;

        // 辅助方法
        void _skipWhitespace(); // 跳过空白符，若遇到换行符，则更新行号
        void _nextChar(); // 读取下一个字符
        char _peekChar(); // 预读下一个字符
        bool _isWhitespace(); // 判断当前字符是否是空白符
        bool _isComment();   // 判断当前字符是否是注释
        
};

#endif