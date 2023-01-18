#include <sstream>
#include <string>
#include <vector>

#include "scheme.h"
#include "parser.h"
#include "tokenizer.h"
#include "error.h"

std::string Interpreter::Run(const std::string &str2) {
    if (IsTrivial(str2)) {
        return Trivial();
    }
    if (str2[0] == ' ') {
        throw SyntaxError("");
    }
    std::string str1 = str2;
    if (str1[str1.length() - 1] == ' ') {
        while (str1[str1.length() - 1] == ' ') {
            str1.pop_back();
        }
    }
    std::string str = str1;
    if (str1[0] == '\'') {
        if (str1[1] == ' ') {
            throw SyntaxError("");
        }
        str = "(quote ";
        for (int64_t i = 1; i < str1.length(); ++i) {
            str += str1[i];
        }
        str += ")";
    }
    std::stringstream s(str);
    Tokenizer tokenizer = Tokenizer(&s);
    std::shared_ptr<Object> input_ast;
    try {
        input_ast = Read(&tokenizer);
    } catch (...) {
        throw SyntaxError("");
    }
    if (!input_ast) {
        throw RuntimeError("");
    }
    if (Is<CloseBracket>(input_ast)) {
        throw SyntaxError("");
    }
    if (Is<Number>(input_ast) || Is<Symbol>(input_ast) || Is<Bool>(input_ast)) {
        return input_ast->Serialise();
    }
    if (Is<Cell>(input_ast)) {
        if (!((As<Cell>(input_ast))->GetFirst())) {
            throw RuntimeError("");
        }
        std::shared_ptr<Object> output_ast;
        if (Is<Symbol>((As<Cell>(input_ast))->GetFirst())) {
            auto name = As<Symbol>((As<Cell>(input_ast))->GetFirst())->GetName();
            if (name == "quote") {
                auto temp = (As<Cell>(input_ast))->GetSecond();
                if (!temp) {
                    throw SyntaxError("");
                }
                if (!Is<Cell>(temp)) {
                    return temp->Serialise();
                }
                if (!As<Cell>(temp)->GetFirst()) {
                    throw RuntimeError("");
                }
                return "(" + (As<Cell>(temp)->GetFirst())->Serialise() + ")";
            } else {
                std::unordered_set<std::string> functions{
                    "quote",   "+",        "*",   "-",   "/",    "-",         "=",
                    "<",       ">",        ">=",  "<=",  "min",  "max",       "abs",
                    "number?", "boolean?", "not", "and", "or",   "pair?",     "null?",
                    "list?",   "cons",     "car", "cdr", "list", "list-tail", "list-ref"};
                if (functions.contains(name)) {
                    output_ast = input_ast->Eval();
                } else {
                    throw RuntimeError("");
                }
            }
            // output_ast = input_ast->Eval();
            std::string t;
            if (Is<Cell>(output_ast)) {
                if (!As<Cell>(output_ast)->GetFirst() && !As<Cell>(output_ast)->GetSecond()) {
                    return "()";
                }
                t = "(" + output_ast->Serialise() + ")";
            } else {
                t = output_ast->Serialise();
            }
            return t;
        }
        throw RuntimeError("");
        output_ast = input_ast->Eval();
        auto t = output_ast->Serialise();
        return t;
    }
    throw RuntimeError("");
}
