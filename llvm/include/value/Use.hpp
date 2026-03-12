#ifndef LLVM_USE_HPP
#define LLVM_USE_HPP

#include "IrForward.hpp"
#include "LlvmContext.hpp"
#include "User.hpp"

/*
    Use类表示_user使用了_value
    注意：Use不是一个value，它只是一份信息
*/
class Use {
    public:
        // 阻止拷贝
        Use(const Use &) = delete;
        Use &operator=(const Use &) = delete;

        static UsePtr New(UserPtr user, ValuePtr value) { return user->Context()->SaveUse(new Use(user, value)); }
        ValuePtr GetValue() const { return _value; }
        UserPtr GetUser() const { return _user; }

    private:
        Use(UserPtr user, ValuePtr value) : _user(user), _value(value) {}

        UserPtr _user;
        ValuePtr _value;    
};

#endif