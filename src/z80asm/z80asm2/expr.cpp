//-----------------------------------------------------------------------------
// z80asm - expressions
// Copyright (C) Paulo Custodio, 2011-2024
// License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
//-----------------------------------------------------------------------------

#include "common.h"
#include "expr.h"
#include "utils.h"
#include "xassert.h"
using namespace std;

//-----------------------------------------------------------------------------

ExprResult::ExprResult(int value, ErrCode err_code, const string& err_arg)
    : value_(value), err_code_(err_code), err_arg_(err_arg) {
}

int ExprResult::value() const {
    return value_;
}

bool ExprResult::ok() const {
    return err_code_ == ErrOk;
}

ErrCode ExprResult::err_code() const {
    return err_code_;
}

void ExprResult::error() {
    if (!ok())
        g_errors.error(err_code_, err_arg_);
}

//-----------------------------------------------------------------------------

Expr::Expr(const string& expr_text) {
    Lexer lexer(expr_text);
    if (!parse_expr(&lexer))
        g_errors.error(ErrSyntaxExpr, expr_text);
}

const string& Expr::text() const {
    return text_;
}

bool Expr::parse_expr(Lexer* lexer) {
    parsing_if_ = false;
    return parse_expr1(lexer);
}

bool Expr::parse_if_expr(Lexer* lexer) {
    parsing_if_ = true;
    return parse_expr1(lexer);
}

ExprResult Expr::eval() const {
    vector<int> stack;
    int a, b, c;
    ExprResult result;

    for (auto& token : rpn_tokens_) {
        switch (token.code()) {
        case TK_QUEST:
            xassert(stack.size() >= 3);
            c = stack.back(); stack.pop_back();
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a)
                stack.push_back(b);
            else
                stack.push_back(c);
            break;

        case TK_LOGOR:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a || b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_LOGXOR:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (!!a != !!b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_LOGAND:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a && b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_BINOR:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a | b);
            break;

        case TK_BINXOR:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a ^ b);
            break;

        case TK_BINAND:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a & b);
            break;

        case TK_LT:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a < b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_LE:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a <= b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_GT:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a > b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_GE:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a >= b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_EQ:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a == b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_NE:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (a != b)
                stack.push_back(1);
            else
                stack.push_back(0);
            break;

        case TK_LSHIFT:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a << b);
            break;

        case TK_RSHIFT:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a >> b);
            break;

        case TK_PLUS:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a + b);
            break;

        case TK_MINUS:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a - b);
            break;

        case TK_MULT:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(a * b);
            break;

        case TK_DIV:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (b == 0)
                return ExprResult(0, ErrDivisionByZero);
            else
                stack.push_back(a / b);
            break;

        case TK_MOD:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            if (b == 0)
                return ExprResult(0, ErrDivisionByZero);
            else
                stack.push_back(a % b);
            break;

        case TK_POWER:
            xassert(stack.size() >= 2);
            b = stack.back(); stack.pop_back();
            a = stack.back(); stack.pop_back();
            stack.push_back(ipow(a, b));
            break;

        case TK_UNARY_MINUS:
            xassert(stack.size() >= 1);
            a = stack.back(); stack.pop_back();
            stack.push_back(-a);
            break;

        case TK_UNARY_PLUS:
            xassert(stack.size() >= 1);
            break;

        case TK_LOGNOT:
            xassert(stack.size() >= 1);
            a = stack.back(); stack.pop_back();
            stack.push_back(!a);
            break;

        case TK_BINNOT:
            xassert(stack.size() >= 1);
            a = stack.back(); stack.pop_back();
            stack.push_back(~a);
            break;

        case TK_LPAREN:
            xassert(stack.size() >= 1);
            break;

        case TK_IDENT:
            xassert(token.symbol);
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
            xassert(0);
        }
    }

    xassert(stack.size() == 1);
    int x = stack.back();
    return ExprResult(x, ErrOk);
}

bool Expr::parse_expr1(Lexer* lexer) {
    lexer_ = lexer;
    bool result = parse_expr();
    lexer_ = nullptr;
    return result;
}

bool Expr::parse_expr() {
    return false;
}
