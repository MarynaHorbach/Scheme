#include <parser.h>
#include "tokenizer.h"
#include "error.h"
std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("");
    }
    auto token = tokenizer->GetToken();
    tokenizer->Next();
    if (token == Token{BracketToken::OPEN}) {
        return ReadList(tokenizer);
    } else if (token == Token{BracketToken::CLOSE}) {
        return std::shared_ptr<Object>(new CloseBracket(&token));
    } else {
        if (QuoteToken* x = std::get_if<QuoteToken>(&token)) {
            if (tokenizer->IsEnd()) {
                throw SyntaxError("");
            }
            return std::shared_ptr<Object>(new Symbol("quote"));
        }
        if (ConstantToken* x = std::get_if<ConstantToken>(&token)) {
            return std::shared_ptr<Object>(new Number(x));
        }
        return std::shared_ptr<Object>(new Symbol(&token));
    }
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    auto first = Read(tokenizer);
    if (Is<CloseBracket>(first)) {
        return nullptr;
    }
    // мб надо кинуть какие-то ошибки
    auto obj = std::shared_ptr<Object>(new Cell{first, nullptr});
    auto answer = obj;
    auto second = Read(tokenizer);
    while (!Is<CloseBracket>(second)) {
        if (As<Symbol>(second) && As<Symbol>(second)->GetName() == ".") {
            auto second1 = Read(tokenizer);
            second = Read(tokenizer);
            if (Is<CloseBracket>(second)) {
                As<Cell>(obj)->SetSecond(second1);
                return answer;
            }
            if (!Is<Cell>(second1)) {
                throw SyntaxError("");
            }
        } else {
            auto new_obj = std::shared_ptr<Object>(new Cell{second, nullptr});
            As<Cell>(obj)->SetSecond(new_obj);
            obj = new_obj;
            second = Read(tokenizer);
        }
    }
    return answer;
}