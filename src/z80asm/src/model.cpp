//-----------------------------------------------------------------------------
// z80asm
// Copyright (C) Paulo Custodio, 2011-2024
// License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
//-----------------------------------------------------------------------------

#include "args.h"
#include "errors.h"
#include "file_reader.h"
#include "if.h"
#include "model.h"
#include "utils.h"
#include <cassert>
#include <exception>
#include <iostream>
using namespace std;

Assembler g_asm;

//-----------------------------------------------------------------------------

ExprResult Expr::get_value() const {
	vector<int> stack;
    int a, b, c;
    ExprResult result;

	for (auto& token : rpn_tokens) {
		switch (token.code) {
        case TK_QUEST:
            assert(stack.size() >= 3);
            c = stack.back(); stack.pop_back();
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a)
                stack.push_back(b);
            else
                stack.push_back(c);
            break;

        case TK_LOGOR:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a || b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_LOGXOR:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (!!a != !!b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_LOGAND:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a && b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_BINOR:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a | b);
            break;

        case TK_BINXOR:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a ^ b);
            break;

        case TK_BINAND:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a & b);
            break;

        case TK_LT:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a < b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_LE:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a <= b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_GT:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a > b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_GE:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a >= b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_EQ:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a == b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_NE:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a != b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_LSHIFT:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a << b);
            break;

        case TK_RSHIFT:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a >> b);
            break;

        case TK_PLUS:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a + b);
            break;

        case TK_MINUS:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a - b);
            break;

        case TK_MULT:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a * b);
            break;

        case TK_DIV:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (b == 0)
                return ExprResult(0, ErrDivisionByZero);
            else
                stack.push_back(a / b);
            break;

        case TK_MOD:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (b == 0)
                return ExprResult(0, ErrDivisionByZero);
            else
                stack.push_back(a % b);
            break;

        case TK_POWER:
            assert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(ipow(a, b));
            break;

        case TK_UNARY_MINUS:
            assert(stack.size() >= 1);
            a = stack.back(); stack.pop_back();
            stack.push_back(-a);
            break;

        case TK_UNARY_PLUS:
            assert(stack.size() >= 1);
            break;

        case TK_LOGNOT:
            assert(stack.size() >= 1);
            a = stack.back(); stack.pop_back();
            stack.push_back(!a);
            break;

        case TK_BINNOT:
            assert(stack.size() >= 1);
            a = stack.back(); stack.pop_back();
            stack.push_back(~a);
            break;

        case TK_LPAREN:
            assert(stack.size() >= 1);
            break;

        case TK_IDENT:
            assert(token.symbol);
            result = token.symbol->get_value();
            if (result.err_code != ErrOk)
                return result;
            else
                stack.push_back(result.value);
            break;

        case TK_INTEGER:
            stack.push_back(token.ivalue);
            break;

        default:
            assert(0);
		}
	}

    assert(stack.size() == 1);
    int x = stack.back();
    return ExprResult(x, ErrOk);
}

void Expr::parse_ternary_condition() {
    parse_logical_or();
    if (token_code() == TK_QUEST) {
        consume_token();
        parse_ternary_condition();
        if (token_code() == TK_COLON) {
            consume_token();
            parse_ternary_condition();
            rpn_tokens.push_back(Token(TK_QUEST));
        }
        else {
            throw ExprException(ErrColonExpected, lexer->peek_text());
        }
    }
}

void Expr::parse_logical_or() {
    parse_logical_and();
    while (true) {
        switch (token_code()) {
        case TK_LOGOR:
            consume_token();
            parse_logical_and();
            rpn_tokens.push_back(Token(TK_LOGOR));
            break;
        case TK_LOGXOR:
            consume_token();
            parse_logical_and();
            rpn_tokens.push_back(Token(TK_LOGXOR));
            break;
        default:
            return;
        }
    }
}

//-----------------------------------------------------------------------------

Instr::Instr() {
    location = g_errors.location;
}

Instr::~Instr() {
    clear();
}

void Instr::clear() {
    for (auto& patch : patches)
        delete patch;
}

void Instr::set_bytes(int opcode) {
    bytes.clear();
    if ((opcode & 0xff000000) != 0 || bytes.size() > 0)
        bytes.push_back((opcode >> 24) & 0xff);
    if ((opcode & 0x00ff0000) != 0 || bytes.size() > 0)
        bytes.push_back((opcode >> 16) & 0xff);
    if ((opcode & 0x0000ff00) != 0 || bytes.size() > 0)
        bytes.push_back((opcode >> 8) & 0xff);
    bytes.push_back(opcode & 0xff);
}

//-----------------------------------------------------------------------------

Section::Section(const string& name_)
    : name(name_) {
}

Section::~Section() {
    clear();
}

void Section::clear() {
    for (auto& instr : instrs)
        delete instr;
}

//-----------------------------------------------------------------------------

Module::Module(const string& name_)
    : name(name_) {
    // create "" section
    select_section("");
}

Module::~Module() {
    clear();
}

void Module::clear() {
    for (auto& section : sections)
        delete section;
    sections.clear();
    section_by_name.clear();
    cur_section = nullptr;
    local_symbols.clear();
}

void Module::select_section(const string& name) {
    auto it = section_by_name.find(name);
    if (it == section_by_name.end()) {
        cur_section = new Section(name);
        sections.push_back(cur_section);
        section_by_name[name] = cur_section;
    }
    else
        cur_section = it->second;
}

//-----------------------------------------------------------------------------

Object::Object(const string& name_)
    : name(name_) {
    select_module(name);
}

Object::~Object() {
    clear();
}

void Object::clear() {
    for (auto& module : modules)
        delete module;
    modules.clear();
    module_by_name.clear();
    cur_module = nullptr;
}

void Object::select_module(const string& name) {
    auto it = module_by_name.find(name);
    if (it == module_by_name.end()) {
        cur_module = new Module(name);
        modules.push_back(cur_module);
        module_by_name[name] = cur_module;
    }
    else
        cur_module = it->second;
}

//-----------------------------------------------------------------------------

Assembler::Assembler() {
}

Assembler::~Assembler() {
    clear();
}

void Assembler::clear() {
    for (auto& object : objects)
        delete object;
    objects.clear();
    cur_object = nullptr;
    defines.clear();
    global_symbols.clear();
}

void Assembler::add_object(Object* object) {
    objects.push_back(object);
    cur_object = object;
}

string Assembler::autolabel() {
    static int n = 0;
    ostringstream oss;
    oss << "z80asm$" << setw(4) << setfill('0') << n++;
    return oss.str();
}

Symbol* Assembler::add_asmpc_instr() {
    string asmpc_name = autolabel();
    return add_label(asmpc_name);
}

Symbol* Assembler::add_label(const string& name) {
    Instr* instr = add_instr();
    Symbol* label = add_symbol(name);
    if (label) {
        label->type = TYPE_ADDRESS;
        instr->label = label;
        return label;
    }
    else
        return nullptr;
}

Instr* Assembler::add_instr() {
    Module* module = cur_module();
    assert(module);
    Instr* instr = new Instr();
    module->cur_section->instrs.push_back(instr);
    return instr;
}

Instr* Assembler::add_instr(int opcode) {
    Instr* instr = add_instr();
    instr->set_bytes(opcode);
    return instr;
}

Symbol* Assembler::find_symbol(const string& name) {
    Symbol* symbol = find_local_symbol(name);
    if (!symbol)
        symbol = find_global_symbol(name);
    if (!symbol)
        symbol = find_global_define(name);
    return symbol;
}

Symbol* Assembler::add_symbol(const string& name) {
    Module* module = cur_module();
    assert(module);

    // check if already defined
    Symbol* symbol = find_symbol(name);
    if (!symbol) {                              // new symbol
        symbol = new Symbol(name, SCOPE_LOCAL, TYPE_CONSTANT, module->cur_section);
        bool ok = module->local_symbols.insert(symbol);
        assert(ok);
        return symbol;
    }
    else if (symbol->is_global_def) {           // global define, make a copy
        int value = symbol->value;
        symbol = new Symbol(name, SCOPE_LOCAL, TYPE_CONSTANT, module->cur_section);
        symbol->value = value;
        bool ok = module->local_symbols.insert(symbol);
        assert(ok);
        return symbol;
    }
    else if (symbol->type == TYPE_UNDEFINED) {   // already declared
        symbol->type = TYPE_CONSTANT;
        symbol->location = g_errors.location;
        return symbol;
    }
    else {                                      // already defined
        g_errors.error(ErrDuplicateDefinition, name);
        return nullptr;
    }
}

Symbol* Assembler::use_symbol(const string& name) {
    Module* module = cur_module();
    assert(module);

    // check if already defined
    Symbol* symbol = find_symbol(name);
    if (!symbol) {                               // new symbol
        symbol = new Symbol(name, SCOPE_LOCAL, TYPE_UNDEFINED, module->cur_section);
        bool ok = module->local_symbols.insert(symbol);
        assert(ok);
    }
    else if (symbol->is_global_def) {           // global define, make a copy
        int value = symbol->value;
        symbol = new Symbol(name, SCOPE_LOCAL, TYPE_CONSTANT, module->cur_section);
        symbol->value = value;
        bool ok = module->local_symbols.insert(symbol);
        assert(ok);
    }
    symbol->is_touched = true;
    return symbol;
}

Symbol* Assembler::add_define(const string& name, int value) {
    Symbol* symbol = defines.find(name);
    if (symbol) {                   // already defined
        ExprResult result = symbol->get_value();
        if (result.err_code != ErrOk) {
            g_errors.error(result.err_code, result.err_arg);
            return nullptr;
        }
        else if (result.value != value) {
            g_errors.error(ErrDuplicateDefinition, name);
            return nullptr;
        }
        else {
            return symbol;          // OK, already defined but same value
        }
    }
    else {
        symbol = new Symbol(name, SCOPE_LOCAL, TYPE_CONSTANT, nullptr);
        symbol->is_global_def = true;
        bool ok = defines.insert(symbol);
        assert(ok);
        return symbol;
    }
}

void Assembler::erase_define(const string& name) {
    Symbol* symbol = defines.erase(name);
    if (symbol)
        delete symbol;
}

Module* Assembler::cur_module() {
    if (!cur_object)
        return nullptr;
    else
        return cur_object->cur_module;
}

Symbol* Assembler::find_local_symbol(const string& name) {
    Module* module = cur_module();
    if (module)
        return module->local_symbols.find(name);
    else
        return nullptr;
}

Symbol* Assembler::find_global_symbol(const string& name) {
    return global_symbols.find(name);
}

Symbol* Assembler::find_global_define(const string& name) {
    return defines.find(name);
}

bool Assembler::got_errors() const {
    return start_errors != g_errors.count;
}

bool Assembler::assemble(const string& filename) {
    if (g_args.verbose)
        cout << "Assembling '" << filename << "'" << endl;

    start_errors = g_errors.count;

    // create object and default module
    Object* object = new Object(fs::path(filename).stem().generic_string());
    add_object(object);

    // clear globals
    global_symbols.clear();
    parser.clear();
    assemble1(filename);
    cur_object = nullptr;

    if (g_args.verbose)
        cout << endl;

    // exit true if no more errors
    return !got_errors();
}

void Assembler::assemble1(const string& filename) {
    // create parent directory of object file
    string o_filename = g_args.o_filename(filename);
    string parent_dir = fs::path(o_filename).parent_path().generic_string();
    if (!parent_dir.empty() && !fs::is_directory(parent_dir)) {
        if (!fs::create_directories(parent_dir)) {
            g_errors.error(ErrDirCreate, parent_dir);
            perror(parent_dir.c_str());
            return;
        }
    }

    if (!g_source_reader.open(filename))
        return;

    parser.parse();                        
    if (got_errors())
        return;

    check_relative_jumps();         
    if (got_errors())
        return;

    patch_local_exprs();
    if (got_errors())
        return;

    check_undefined_symbols();
    if (got_errors())
        return;

    write_obj_file(o_filename);
    if (got_errors())
        return;

    // make legacy data structures
    assemble_file(o_filename.c_str());
}

void Assembler::check_relative_jumps() {
    // TODO
}

void Assembler::patch_local_exprs() {
    // TODO
}

void Assembler::check_undefined_symbols() {
    // TODO
}

void Assembler::write_obj_file(const string& /*filename*/) {
    // TODO
}

void g_asm_assemble_file(const char* filename) {
    g_asm.assemble(filename);
}
