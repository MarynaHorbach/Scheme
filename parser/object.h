#pragma once
#include "tokenizer.h"
#include <memory>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
};

class Number : public Object {
public:
    Number(ConstantToken* token) : value_(token->value) {
    }

    int GetValue() const {
        return value_;
    }

private:
    int64_t value_;
};

class CloseBracket : public Object {
public:
    CloseBracket(Token* token) {
    }
};

class Symbol : public Object {
public:
    Symbol(Token* token) : name_() {
        if (SymbolToken* x = std::get_if<SymbolToken>(token)) {
            name_ = x->name;
        } else if (DotToken* x = std::get_if<DotToken>(token)) {
            name_ = ".";
        }
    }

    Symbol(std::string s) : name_(s) {
    }

    const std::string& GetName() const {
        return name_;
    }

private:
    std::string name_;
};

class Cell : public Object {
public:
    Cell(std::shared_ptr<Object> a, std::shared_ptr<Object> b) : first_(a), second_(b) {
    }

    std::shared_ptr<Object> GetFirst() const {
        return first_;
    }
    std::shared_ptr<Object> GetSecond() const {
        return second_;
    }

    void GetFirst(std::shared_ptr<Object> f) {
        first_ = f;
    }
    void SetSecond(std::shared_ptr<Object> s) {
        second_ = s;
    }

private:
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    if (As<T>(obj)) {
        return true;
    }
    return false;
}
