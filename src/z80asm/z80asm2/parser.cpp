//-----------------------------------------------------------------------------
// z80asm - parser
// Copyright (C) Paulo Custodio, 2011-2024
// License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
//-----------------------------------------------------------------------------

#include "common.h"
#include "assembler.h"
#include "parser.h"
#include "xassert.h"
using namespace std;

#include "cpu/parse_code.h"

Parser::Parser(Assembler& assembler)
    : assembler_(&assembler) {
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
        assembler_->add_asmpc_instr();
        if (!parse_directive() && !parse_opcode())
            error(ErrSyntax);
    }
}

void Parser::parse_label() {
    // label:
    if (lexer_.peek(0).code() == TK_IDENT &&
        lexer_.peek(1).code() == TK_COLON &&
        lexer_.peek(2).keyword() != KW_EQU) {
        assembler_->add_label(lexer_.peek(0).svalue());
        lexer_.next(2);
    }
    // .label
    else if (lexer_.peek(0).code() == TK_DOT &&
        lexer_.peek(1).code() == TK_IDENT &&
        lexer_.peek(2).keyword() != KW_EQU) {
        assembler_->add_label(lexer_.peek(1).svalue());
        lexer_.next(2);
    }
}

bool Parser::parse_directive() {
    return false;
}

bool Parser::parse_opcode() {
    int state = 0;
    bool accept = false;
    while (true) {
        const Token& token = lexer_.peek();

        int accept = accept_stt[state];
        if (accept != 0) {
            parse_action(accept);
            return true;
        }

        int end_state = token_stt[state][TK_END];
        if (end_state != 0) {
            if (!match_eos())
                return false;           // syntax error
            state = end_state;
            continue;
        }

        int keyword = token.keyword();
        int next_state = keyword_stt[state][keyword];
        if (keyword != KW_NONE && next_state != 0) {
            lexer_.next();
            state = next_state;
            continue;
        }

        int code = token.code();
        next_state = token_stt[state][code];
        if (next_state != 0) {
            lexer_.next();
            state = next_state;
            continue;
        }

        next_state = expr_stt[state];
        if (next_state != 0) {
            if (!parse_expr())
                return false;           // syntax error
            state = next_state;
            continue;
        }

        next_state = const_expr_stt[state];
        if (next_state != 0) {
            if (!parse_expr())
                return false;           // syntax error
            state = next_state;
            continue;
        }

        if (code == TK_END)
            break;
    }
    return false;                       // syntax error
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

