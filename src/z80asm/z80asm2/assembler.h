//-----------------------------------------------------------------------------
// z80asm - assembler
// Copyright (C) Paulo Custodio, 2011-2024
// License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
//-----------------------------------------------------------------------------

#pragma once

#include <string>
using namespace std;

class Assembler {
public:
    void add_asmpc_instr() {}
    void add_instr(unsigned /*opcode*/) {}
    void add_label(const string& /*name*/) {}
};
