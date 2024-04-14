//-----------------------------------------------------------------------------
// z80asm - object file model
// Copyright (C) Paulo Custodio, 2011-2024
// License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
//-----------------------------------------------------------------------------

#include "object.h"
using namespace std;

//-----------------------------------------------------------------------------

Instr::Instr(int offset, int phased_asmpc)
    : offset_(offset), phased_asmpc_(phased_asmpc) {
}

Instr::~Instr() {
    clear();
}

void Instr::clear() {
    offset_ = 0;
    phased_asmpc_ = ORG_NOT_DEFINED;
    for (auto& patch : patches_)
        delete patch;
    patches_.clear();
    label_ = nullptr;
}

void Instr::set_bytes(int opcode) {
    bytes_.clear();
    if ((opcode & 0xff000000) != 0 || bytes_.size() > 0)
        bytes_.push_back((opcode >> 24) & 0xff);
    if ((opcode & 0x00ff0000) != 0 || bytes_.size() > 0)
        bytes_.push_back((opcode >> 16) & 0xff);
    if ((opcode & 0x0000ff00) != 0 || bytes_.size() > 0)
        bytes_.push_back((opcode >> 8) & 0xff);
    bytes_.push_back(opcode & 0xff);
}

int Instr::offset() const {
    return offset_;
}

int Instr::phased_asmpc() const {
    return phased_asmpc_;
}

int Instr::asmpc() const {
    if (phased_asmpc_ != ORG_NOT_DEFINED)
        return phased_asmpc_;
    else
        return offset_;
}

const Bytes& Instr::bytes() const {
    return bytes_;
}

const list<Patch*>& Instr::patches() const {
    return patches_;
}

Symbol* Instr::label() const {
    return label_;
}

void Instr::set_label(Symbol* label) {
    label_ = label;
}

//-----------------------------------------------------------------------------

Section::Section(const string& name)
    : name_(name) {
}

Section::~Section() {
    clear();
}

void Section::clear() {
    for (auto& instr : instrs_)
        delete instr;
}

const string& Section::name() const {
    return name_;
}

int Section::size() const {
    if (instrs_.empty())
        return 0;
    else
        return instrs_.back()->offset() + (int)instrs_.back()->bytes().size();
}

Instr* Section::add_instr() {
    Instr* instr = new Instr(size(), phased_asmpc());
    instrs_.push_back(instr);
    return instr;
}

int Section::phased_asmpc() const {
    if (instrs_.empty())
        return ORG_NOT_DEFINED;
    else if (instrs_.back()->phased_asmpc() == ORG_NOT_DEFINED)
        return ORG_NOT_DEFINED;
    else
        return instrs_.back()->phased_asmpc() + (int)instrs_.back()->bytes().size();
}

//-----------------------------------------------------------------------------

Module::Module(const string& name)
    : name_(name) {
    create_default_section();
}

Module::~Module() {
    clear_all();
}

void Module::clear() {
    clear_all();
    create_default_section();
}

void Module::clear_all() {
    for (auto& section : sections_)
        delete section;
    sections_.clear();
    section_by_name_.clear();
    cur_section_ = nullptr;
    local_symbols_.clear();
}

void Module::create_default_section() {
    select_section("");                     // create "" section
}

void Module::select_section(const string& name) {
    auto it = section_by_name_.find(name);
    if (it == section_by_name_.end()) {
        cur_section_ = new Section(name);
        sections_.push_back(cur_section_);
        section_by_name_[name] = cur_section_;
    }
    else
        cur_section_ = it->second;
}

//-----------------------------------------------------------------------------

Object::Object(const string& name)
    : name_(name) {
    create_default_module();
}

Object::~Object() {
    clear();
}

void Object::clear() {
    clear_all();
    create_default_module();
}

void Object::clear_all() {
    for (auto& module : modules_)
        delete module;
    modules_.clear();
    module_by_name_.clear();
    cur_module_ = nullptr;
    select_module(name_);
}

void Object::create_default_module() {
    select_module(name_);
}

void Object::select_module(const string& name) {
    auto it = module_by_name_.find(name);
    if (it == module_by_name_.end()) {
        cur_module_ = new Module(name);
        modules_.push_back(cur_module_);
        module_by_name_[name] = cur_module_;
    }
    else
        cur_module_ = it->second;
}

