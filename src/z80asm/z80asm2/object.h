//-----------------------------------------------------------------------------
// z80asm - object file model
// Copyright (C) Paulo Custodio, 2011-2024
// License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
//-----------------------------------------------------------------------------

#pragma once

#include "expr.h"
#include "location.h"
#include "symtab.h"
#include <cstdint>
#include <list>
#include <string>
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------

typedef uint8_t Byte;
typedef vector<Byte> Bytes;

//-----------------------------------------------------------------------------

class Instr : public HasLocation {
public:
    Instr();
    virtual ~Instr();
    Instr(const Instr& other) = delete;
    Instr& operator=(const Instr& other) = delete;

    void clear();
    void set_bytes(int opcode);             // decompose 0xfd213412 in 4 bytes

private:
    int addr_{ 0 };                         // address computed during linking
    Bytes bytes_;                           // bytes of the instruction with placeholders for patches
    list<Patch*> patches_;                  // patches for each variable expression
    Symbol* label_{ nullptr };              // weak pointer to label if any
};

//-----------------------------------------------------------------------------

class Section {
public:
    Section(const string& name);
    virtual ~Section();
    Section(const Section& other) = delete;
    Section& operator=(const Section& other) = delete;

    void clear();
    auto begin() { return instrs_.begin(); }
    auto end() { return instrs_.end(); }

private:
    string name_;                           // name of section, may be ""
    list<Instr*> instrs_;                   // list of assembled instructions
};

//-----------------------------------------------------------------------------

class Module {
public:
    Module(const string& name);
    virtual ~Module();
    Module(const Module& other) = delete;
    Module& operator=(const Module& other) = delete;

    void clear();
    auto begin() { return sections_.begin(); }
    auto end() { return sections_.end(); }
    void select_section(const string& name);

private:
    string name_;                           // name based on filename, or given by directive
    list<Section*> sections_;               // list of sections in this module
    unordered_map<string, Section*> section_by_name_;   // index sections by name
    Section* cur_section_{ nullptr };       // current section
    Symtab local_symbols_;                  // local symbols
};

//-----------------------------------------------------------------------------

class Object {
public:
    Object(const string& name);
    virtual ~Object();
    Object(const Object& other) = delete;
    Object& operator=(const Object& other) = delete;

    void clear();
    auto begin() { return modules_.begin(); }
    auto end() { return modules_.end(); }
    void select_module(const string& name);

private:
    string name_;                           // name based on filename
    list<Module*> modules_;                 // list of modules in this object
    unordered_map<string, Module*> module_by_name_; // index modules by name
    Module* cur_module_{ nullptr };         // current module
};
