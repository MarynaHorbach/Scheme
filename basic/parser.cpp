#pragma once
#include <parser.h>
#include "tokenizer.h"
#include "error.h"

std::shared_ptr<Object> Read2(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("");
    }
    auto token = tokenizer->GetToken();
    tokenizer->Next();
    if (token == Token{BracketToken::OPEN}) {
        return ReadList(tokenizer);
    } else if (token == Token{BracketToken::CLOSE}) {
        return std::shared_ptr<Object>(new CloseBracket(&token));
    } else if (token == Token{BoolToken::FALSE}) {
        return std::shared_ptr<Object>(new Bool("#f"));
    } else if (token == Token{BoolToken::TRUE}) {
        return std::shared_ptr<Object>(new Bool("#t"));
    } else {
        if (QuoteToken* x = std::get_if<QuoteToken>(&token)) {
            if (tokenizer->IsEnd()) {
                throw SyntaxError("");
            }
            //            auto f = std::shared_ptr<Object>(new Symbol("quote"));
            //            auto s = Read2(tokenizer);
            //            return std::shared_ptr<Object>(new Cell{f, s});
            return std::shared_ptr<Object>(new Symbol("quote"));
        }
        if (ConstantToken* x = std::get_if<ConstantToken>(&token)) {
            return std::shared_ptr<Object>(new Number(x));
        }
        return std::shared_ptr<Object>(new Symbol(&token));
    }
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    auto first = Read2(tokenizer);
    if (Is<CloseBracket>(first)) {
        return nullptr;
    }
    if (Is<Symbol>(first) && As<Symbol>(first)->GetName() == ".") {
        throw SyntaxError("");
    }
    // мб надо кинуть какие-то ошибки
    auto obj = std::shared_ptr<Object>(new Cell{first, nullptr});
    auto answer = obj;
    auto second = Read2(tokenizer);
    while (!Is<CloseBracket>(second)) {
        if (Is<Symbol>(second) && As<Symbol>(second)->GetName() == ".") {
            auto second1 = Read2(tokenizer);
            if (Is<CloseBracket>(second1)) {
                throw SyntaxError("");
            }
            second = Read2(tokenizer);
            if (Is<CloseBracket>(second)) {
                As<Cell>(obj)->SetSecond(second1);
                return answer;
            }
            if (!Is<Cell>(second1)) {
                throw SyntaxError("");
            }
        } else {
            auto second2 = Read2(tokenizer);
            auto new_obj = std::shared_ptr<Object>(new Cell{second, nullptr});
            As<Cell>(obj)->SetSecond(new_obj);
            obj = new_obj;
            second = second2;
        }
    }
    return answer;
}

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("");
    }
    auto out = Read2(tokenizer);

    if (!(tokenizer->IsEnd())) {
        throw SyntaxError("");
    }
    return out;
}