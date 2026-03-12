#include "Token.hpp"

// 根据Token类型获取字符串
std::string tokenTypeToString(Token::TokenType type) {
    switch (type) {
        #define X(a, b) case Token::a: return std::string(b);
        TOKEN_TYPE
        #undef X
        default: return "UNKNOWN";
    }
}
