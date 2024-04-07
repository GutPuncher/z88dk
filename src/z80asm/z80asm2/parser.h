//-----------------------------------------------------------------------------
// z80asm - parser
// Copyright (C) Paulo Custodio, 2011-2024
// License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
//-----------------------------------------------------------------------------

#pragma once

#include "files.h"
#include "scan.h"
#include <string>
using namespace std;

class Object;

// parse source
class Parser {
public:
    Parser(Object& object);

    bool parse(const string& filename);

private:
    enum State { ST_MAIN };

    State state_{ ST_MAIN };        // state of parser
    Object* object_;                // object file being built
    int start_error_count_{ 0 };    // errors found before parse
    SourceReader source_reader_;    // source file reader
    Lexer lexer_;                   // scanner

    void error(ErrCode err_code);   // syntax error and flush lexer
    bool parse();                   // parse full input
    void parse_line();              // switch to each state-parser
    void parse_main();              // main state
    void parse_label();             // label, if any
    void parse_instr();             // opcode or directive
    bool match_eos();               // match and consume ':', '\\', END
    void parse_eos();               // match_eos() or error
#include "cpu/parse_code.h"
};
