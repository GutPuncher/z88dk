//-----------------------------------------------------------------------------
// z80asm - parser
// Copyright (C) Paulo Custodio, 2011-2024
// License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
//-----------------------------------------------------------------------------

#pragma once

#include "files.h"
#include "scan.h"
#include <vector>
#include <string>
using namespace std;

class Assembler;
class Expr;

// parse source
class Parser {
public:
    Parser(Assembler& assembler);
    virtual ~Parser();
    Parser(const Parser& other) = delete;
    Parser& operator=(const Parser& other) = delete;

    bool parse(const string& filename);

private:
    enum State { ST_MAIN };

    State state_{ ST_MAIN };        // state of parser
    Assembler* assembler_;          // assembler instance
    int start_error_count_{ 0 };    // errors found before parse
    SourceReader source_reader_;    // source file reader
    Lexer lexer_;                   // scanner
    vector<Expr*> exprs_;           // list of expressions parsed

    void error(ErrCode err_code);   // syntax error and flush lexer
    bool parse();                   // parse full input
    void parse_line();              // switch to each state-parser
    void parse_main();              // main state
    void parse_label();             // label, if any
    bool parse_directive();         // directive
    bool parse_opcode();            // opcode
    bool match_eos();               // match and consume ':', '\\', END
    void parse_action(int action);  // execute opcode parser action
    bool parse_expr();              // parse expression in input
    void warn_if_expr_in_parens();  // warning if expression in () and considered immediate
    void add_opcode(int opcode);    // add opcode without arguments
    Instr* add_opcode(int opcode, range_t range); // add opcode with one patch of the given range
    Instr* add_opcode_n(int opcode);    // RANGE_BYTE_UNSIGNED
    Instr* add_opcode_nn(int opcode);   // RANGE_WORD
    Instr* add_opcode_idx(int opcode);  // RANGE_BYTE_SIGNED at third address
    void error_illegal_ident();     // issue error
    void add_call_function(const string& name);     // call to a library function
};
