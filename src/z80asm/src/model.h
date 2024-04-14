//-----------------------------------------------------------------------------
// z80asm
// Copyright (C) Paulo Custodio, 2011-2024
// License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
//-----------------------------------------------------------------------------

#pragma once

#include "errors.h"
#include "parse.h"
#include "scan.h"
#include "z80asm_defs.h"
#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>
using namespace std;

struct Section;
struct Instr;
struct Expr;

//-----------------------------------------------------------------------------

struct Assembler {
    list<Object*> objects;                  // list of objects assembled
    Object* cur_object{ nullptr };          // current object
    Symtab defines;                         // -D and predefined constants
    Symtab global_symbols;                  // global symbols
    Parser parser;                          // parser
    Symbol* asmpc{ nullptr };               // asmpc of current statement
    int start_errors{ 0 };                  // errors at start of assembly

    Assembler();
    virtual ~Assembler();
    Assembler(const Assembler& other) = delete;
    Assembler& operator=(const Assembler& other) = delete;
    void clear();
    auto begin() { return objects.begin(); }
    auto end() { return objects.end(); }
    void add_object(Object* object);

    // labels
    Symbol* add_asmpc_instr();
    Symbol* add_label(const string& name);

    // instructions
    Instr* add_instr();
    Instr* add_instr(int opcode);

    // symbol table
    Symbol* find_symbol(const string& name);            // search local/global symbols and defines
    Symbol* add_symbol(const string& name);             // nullptr if duplicate
    Symbol* use_symbol(const string& name);             // return pointer to new or existing symbol
	Symbol* add_define(const string& name, int value);
	void erase_define(const string& name);
	
    // assemble source file, add to list of object modules
    bool assemble(const string& filename);              // assemble one source file

private:
    Module* cur_module();                               // nullptr if none
    Symbol* find_local_symbol(const string& name);      // nullptr if not found
    Symbol* find_global_symbol(const string& name);     // nullptr if not found
    Symbol* find_global_define(const string& name);     // nullptr if not found
    bool got_errors() const;                            // check if errors occured
    void assemble1(const string& filename);             // worker of assemble()
    void check_relative_jumps();                        // change JR to JP if needed
    void patch_local_exprs();                           // patch values of expressions
    void check_undefined_symbols();                     // check for undefined symbols
    void write_obj_file(const string& filename);        // write object file
};

extern Assembler g_asm;

