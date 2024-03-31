//-----------------------------------------------------------------------------
// z80asm - parser
// Copyright (C) Paulo Custodio, 2011-2024
// License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
//-----------------------------------------------------------------------------

#include "common.h"
#include "object.h"
#include "parser.h"
#include "xassert.h"
using namespace std;

Parser::Parser(Object& object)
    : object_(&object) {
}

bool Parser::parse(const string& filename) {
    if (!source_reader_.open(filename))
        return false;
    start_error_count_ = g_errors.count();

    return start_error_count_ == g_errors.count();
}

void Parser::error(ErrCode err_code) {
    g_errors.error(err_code, lexer_.peek_text());
    lexer_.flush();
}

bool Parser::parse() {
    string line;
    while (source_reader_.getline(line)) {
        if (!lexer_.set_text(line))     // error scanning
            continue;
        if (lexer_.at_end())            // empty line
            continue;           
        parse_line();
    }
    return start_error_count_ == g_errors.count();
}

void Parser::parse_line() {
    switch (state_) {
    case ST_MAIN: parse_main(); break;
    default: xassert(0);
    }
}

void Parser::parse_main() {
    parse_label();
    while (true) {
        while (!lexer_.at_end() && match_eos()) {
        }
        if (lexer_.at_end())
            break;
        object_->add_asmpc_instr();
        parse_instr();
    }
}

void Parser::parse_label() {
    // label:
    if (lexer_.peek(0).code() == TK_IDENT &&
        lexer_.peek(1).code() == TK_COLON &&
        lexer_.peek(2).keyword() != KW_EQU) {
        object_->add_label(lexer_.peek(0).svalue());
        lexer_.next(2);
    }
    // .label
    else if (lexer_.peek(0).code() == TK_DOT &&
        lexer_.peek(1).code() == TK_IDENT &&
        lexer_.peek(2).keyword() != KW_EQU) {
        object_->add_label(lexer_.peek(1).svalue());
        lexer_.next(2);
    }
}

void Parser::parse_instr() {
    if (lexer_.peek(0).keyword() == KW_NOP) {
        object_->add_instr(0x00);
        lexer_.next(1);
        parse_eos();
    }
    else
        error(ErrSyntax);
}

bool Parser::match_eos() {
    switch (lexer_.peek().code()) {
    case TK_COLON:
    case TK_BACKSLASH:
        lexer_.next();
        return true;
    case TK_END:
        return true;
    default:
        return false;
    }
}

void Parser::parse_eos() {
    if (!match_eos())
        error(ErrEosExpected);
}
