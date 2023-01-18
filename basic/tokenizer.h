#pragma once

#include <variant>
#include <optional>
#include <istream>

struct SymbolToken {
    std::string name;
    SymbolToken(std::string s);
    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

enum class BoolToken { TRUE, FALSE };

struct ConstantToken {
    int value;
    ConstantToken(int n);
    bool operator==(const ConstantToken& other) const;
};

using Token =
    std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken, BoolToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    bool is_end_;
    std::istream* stream_;
    Token curr_token_;
};

bool IsTrivial(std::string str);

std::string Trivial();