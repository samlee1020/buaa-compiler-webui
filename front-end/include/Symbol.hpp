#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "Value.hpp"
#include "IrForward.hpp"


struct Symbol {
    bool is_func; // 0 for var, 1 for func
    std::string name;
    int lineno;
    int table_id;

    // LLVM IR
    ValuePtr value;
    
    virtual ~Symbol() = default;

    void set_value(ValuePtr value) { this->value = value; }

    Symbol(bool is_func, std::string name, int lineno, int table_id, ValuePtr value) 
        : is_func(is_func), name(name), lineno(lineno), table_id(table_id), value(value) {} 
};

struct VarSymbol : public Symbol {    
    VarSymbol(std::string name, int lineno, int table_id, bool is_array, bool is_const, bool is_static, int array_size, ValuePtr value) 
        : Symbol(false, name, lineno, table_id, value), is_array(is_array), is_const(is_const), is_static(is_static), array_size(array_size) {}
    
    bool is_array;
    bool is_const;
    bool is_static;
    int array_size;
};

struct FuncSymbol : public Symbol {
    FuncSymbol(std::string name, int lineno, int table_id, int func_type, std::vector<bool> params_is_array, ValuePtr value) 
        : Symbol(true, name, lineno, table_id, value), func_type(func_type), params_is_array(params_is_array) {}
    
    int func_type; // 0 for void, 1 for int
    std::vector<bool> params_is_array;

};

class SymbolTable : public std::enable_shared_from_this<SymbolTable> {
    public:
        SymbolTable(int table_id) : _father(nullptr), _table_id(table_id) {}

        bool exist_in_scope(const std::string& name) { return _symbols.find(name)!= _symbols.end(); }

        bool add_symbol(std::shared_ptr<Symbol> symbol) {
            if (exist_in_scope(symbol->name)) {
                return false;
            }
            _symbols[symbol->name] = symbol;
            return true;
        }

        std::shared_ptr<Symbol> get_symbol(const std::string& name) {
            if (exist_in_scope(name)) {
                return _symbols[name];
            }
            if (_father) {
                return _father->get_symbol(name);
            }
            return nullptr;
        }

        std::shared_ptr<SymbolTable> push_scope(int table_id) {
            auto new_table = std::make_shared<SymbolTable>(table_id);
            new_table->_father = shared_from_this();
            return new_table;
        }

        std::shared_ptr<SymbolTable> pop_scope() {return _father;}

        int get_table_id() const { return _table_id; }

    private:
        std::shared_ptr<SymbolTable> _father;
        std::unordered_map<std::string, std::shared_ptr<Symbol>> _symbols;
        int _table_id;
};

#endif