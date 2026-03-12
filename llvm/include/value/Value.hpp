#ifndef LLVM_VALUE_VALUE_HPP
#define LLVM_VALUE_VALUE_HPP

#include "Asm.hpp"
#include "IrForward.hpp"
#include <string>
#include "Type.hpp"
#include <iostream>

/*
    ValueType枚举类，用于表示不同类型的value
*/
enum class ValueType {
    // Value
    ArgumentTy,
    BasicBlockTy,

    // Value -> Constant
    ConstantTy,
    ConstantIntTy,
    ConstantArrayTy,
    ConstantBoolTy,

    // Value -> Constant -> GlobalValue
    FunctionTy,
    GlobalVariableTy,

    // Value -> User -> Instruction
    BinaryOperatorTy,
    CompareInstTy,
    BranchInstTy,
    JumpInstTy,
    ReturnInstTy,
    StoreInstTy,
    CallInstTy,
    InputInstTy,
    OutputInstTy,
    AllocaInstTy,
    LoadInstTy,
    GepInstTy,
    UnaryOperatorTy,
    ZextInstTy,
};

/*
    Value类是llvm中所有值的基类
    “万物皆value”
*/
class Value {
    // 用于访问User的protected成员
    friend class User;

    public:
        virtual ~Value() = default;

        // 用于运行时类别识别 RTTI (RunTime Type Identification)。不同的子类继承自Value，需要实现classof方法，用于判断是否属于某个子类
        static bool classof(ValueType type) { return true; }

        // 判断某个value是否是某个具体的类别
        template <typename _Ty> bool Is() const { return _Ty::classof(_valueType); }

        // 转换类型，当且仅当知道确切的类型时才使用
        template <typename _Ty> _Ty *As() { return static_cast<_Ty *>(this); }

        // 输出value对应的完整的llvm ir代码
        virtual void PrintAsm(AsmWriterSmartPtr out);

        // 只输出value的名字，比如%1
        virtual void PrintName(AsmWriterSmartPtr out);

        // 输出该value的use，通常是type和name
        virtual void PrintUse(AsmWriterSmartPtr out);

        // 获取value的类型
        ValueType GetValueType() const { return _valueType; }

        // 获取value的返回值类型
        TypePtr GetType() const { return _type; }

        // 获取上下文
        LlvmContextPtr Context() const { return GetType()->Context(); }

        // 获取value的名字
        const std::string &GetName() const { return _name; }

        // 设置value的名字
        void SetName(const std::string &name) { _name = name; }

        // 遍历use的接口
        using use_iterator = UseList::iterator;
        use_iterator UserBegin() { return _userList.begin(); }
        use_iterator UserEnd() { return _userList.end(); }
        UseListPtr GetUserList() { return &_userList; }

        // 已使用计数
        void addUsedCount() { _usedCount++; }
        bool allUseDone() { return _usedCount == _userList.size(); }

    protected:
        // 添加/删除use的接口
        void AddUser(UserPtr user);
        UserPtr RemoveUser(UserPtr user);

        Value(ValueType valueType, TypePtr type) : _type(type), _valueType(valueType) { _usedCount = 0; }
        Value(ValueType ValueType, TypePtr type, const std::string &name) : _type(type), _name(name), _valueType(ValueType) { _usedCount = 0; }

        TypePtr _type;
        std::string _name;
        UseList _userList;

    private:
        ValueType _valueType;
        int _usedCount; // 已使用计数
};

#endif