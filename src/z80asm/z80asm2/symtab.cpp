//-----------------------------------------------------------------------------
// z80asm - symbol table
// Copyright (C) Paulo Custodio, 2011-2024
// License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
//-----------------------------------------------------------------------------

#include "common.h"
#include "expr.h"
#include "object.h"
#include "symtab.h"
using namespace std;

Symbol::Symbol(const string& name, sym_scope_t scope, sym_type_t type, Section* section, int value)
    : name_(name), scope_(scope), type_(type), section_(section), value_(value) {
}

const string& Symbol::name() const {
    return name_;
}

ExprResult Symbol::eval() {
    if (recurse_count_ != 0)
        return ExprResult(0, ErrExprRecursion);
    else {
        recurse_count_++;
        ExprResult result = eval1();
        recurse_count_--;
        return result;
    }
}

ExprResult Symbol::eval1() {
    if (type_ == TYPE_UNDEFINED)
        return ExprResult(0, ErrUndefinedSymbol, name_);
    else if (expr_)
        return expr_->eval();
    else if (instr_)
        return instr_->addr();
    else
        return value_;
}

//-----------------------------------------------------------------------------

Symtab::Symtab() {
}

Symtab::~Symtab() {
    clear();
}

void Symtab::clear() {
    for (auto& it : symbols_)
        delete it.second;
    symbols_.clear();
}

bool Symtab::insert(Symbol* symbol) {
    auto it = symbols_.find(symbol->name());
    if (it != symbols_.end()) {
        if (symbol->name().substr(0, 11) == "__CDBINFO__")
            return true;	// ignore duplicates of these
        else {
            g_errors.error(ErrDuplicateDefinition, symbol->name());
            return false;
        }
    }
    else {
        symbols_[symbol->name()] = symbol;
        return true;
    }
}

Symbol* Symtab::find(const string& name) {
    auto it = symbols_.find(name);
    if (it != symbols_.end())
        return it->second;
    else
        return nullptr;
}

Symbol* Symtab::erase(const string& name) {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        Symbol* symbol = it->second;
        symbols_.erase(it);
        return symbol;
    }
    else
        return nullptr;
}
