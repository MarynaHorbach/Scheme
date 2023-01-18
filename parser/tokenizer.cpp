#include <tokenizer.h>
#include "error.h"
#include <vector>

SymbolToken::SymbolToken(std::string s) : name(s){};

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
};

bool QuoteToken ::operator==(const QuoteToken&) const {
    return true;
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

ConstantToken::ConstantToken(int n) : value(n) {
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

Tokenizer::Tokenizer(std::istream* in) : is_end_(false), stream_(in), curr_token_(0) {
    Next();
    curr_token_ = GetToken();
}

bool Tokenizer::IsEnd() {
    return is_end_;
}

bool IsStartingSymbol(char c) {
    if (c == '<' || c == '=' || c == '>') {
        return true;
    }
    if ('A' <= c && c <= 'z') {
        return true;
    }
    if (c == '*' || c == '/' || c == '#') {
        return true;
    }
    return false;
}

bool IsNumber(char c) {
    if ('0' <= c && c <= '9') {
        return true;
    }
    return false;
}

bool IsInsideSymbol(char c) {
    if (IsStartingSymbol(c)) {
        return true;
    }
    if (c == '!' || c == '-' || c == '?') {
        return true;
    }
    if ('0' <= c && c <= '9') {
        return true;
    }
    return false;
}

void Tokenizer::Next() {
    auto c = stream_->peek();
    if (c == EOF) {
        stream_->get();
        is_end_ = true;
    } else {
        while (stream_->peek() <= 32) {
            if (stream_->peek() == EOF) {
                stream_->get();
                is_end_ = true;
                return;
            }
            stream_->get();
        }
        char c1 = stream_->get();
        if (c1 == '(') {
            curr_token_ = BracketToken::OPEN;
        } else if (c1 == ')') {
            curr_token_ = BracketToken::CLOSE;
        } else if (c1 == '.') {
            curr_token_ = DotToken();
        } else if (c1 == 39) {
            curr_token_ = QuoteToken();
        } else if (c1 == '+') {
            std::string stack;
            while (IsNumber(stream_->peek())) {
                stack += stream_->get();
            }
            if (!stack.empty()) {
                curr_token_ = ConstantToken(std::stoi(stack));
            } else {
                curr_token_ = SymbolToken("+");
            }
        } else if (c1 == '-') {
            std::string stack = "-";
            while (IsNumber(stream_->peek())) {
                stack += stream_->get();
            }
            if (stack != "-") {
                curr_token_ = ConstantToken(std::stoi(stack));
            } else {
                curr_token_ = SymbolToken("-");
            }
        } else if (IsNumber(c1)) {
            std::string stack;
            stack += c1;
            while (IsNumber(stream_->peek())) {
                stack += stream_->get();
            }
            curr_token_ = ConstantToken(std::stoi(stack));
        } else {
            if (c1 == '#') {
                if (stream_->peek() == EOF) {
                    curr_token_ = SymbolToken("#");
                    return;
                } else {
                    char c2 = stream_->get();
                    std::string stack;
                    stack += '#';
                    stack += c2;
                    if (stream_->peek() == ' ') {
                        if (c2 == 't') {
                            curr_token_ = BoolToken::TRUE;
                        } else if (c2 == 'f') {
                            curr_token_ = BoolToken::FALSE;
                        } else {
                            curr_token_ = SymbolToken(stack);
                        }
                    } else if (IsInsideSymbol(stream_->peek())) {
                        while (IsInsideSymbol(stream_->peek())) {
                            stack += stream_->get();
                        }
                        curr_token_ = SymbolToken(stack);
                    } else {
                        curr_token_ = SymbolToken(stack);
                    }
                }
            } else if (IsStartingSymbol(c1)) {
                std::string stack;
                stack += c1;
                while (IsInsideSymbol(stream_->peek())) {
                    stack += stream_->get();
                }
                curr_token_ = SymbolToken(stack);
            } else {
                throw SyntaxError({""});
            }
        }
    }
}

Token Tokenizer::GetToken() {
    return curr_token_;
}