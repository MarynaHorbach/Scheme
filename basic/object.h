#pragma once
#include "tokenizer.h"
#include "error.h"
#include <memory>
#include <vector>
#include <unordered_set>
#include <functional>
class Object : public std::enable_shared_from_this<Object> {
public:
    virtual std::string Serialise() = 0;
    virtual std::shared_ptr<Object> Eval() = 0;
    virtual ~Object() = default;
};

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj);

template <class T>
bool Is(const std::shared_ptr<Object>& obj);

std::vector<std::shared_ptr<Object>> ArgsToVector(std::shared_ptr<Object> curr);

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

    std::string Serialise() {
        if (name_ == "=" || name_ == ">" || name_ == "<" || name_ == ">=" || name_ == "<=" ||
            name_ == "and") {
            return "#t";
        }

        if (name_ == "or") {
            return "#f";
        }
        if (name_ == "+") {
            return "0";
        }
        if (name_ == "*") {
            return "1";
        }
        if (name_ == "/" || name_ == "-" || name_ == "min" || name_ == "max" || name_ == "abs") {
            throw RuntimeError("");
        }
        return name_;
    }

    std::shared_ptr<Object> Eval() {
        return std::shared_ptr<Object>(new Symbol(GetName()));
    }

private:
    std::string name_;
};

class Bool : public Object {
public:
    Bool(std::string s) : name_(s) {
    }

    const std::string& GetName() const {
        return name_;
    }

    bool GetVal() {
        if (name_ == "#t") {
            return true;
        }
        return false;
    }

    std::string Serialise() {
        return name_;
    }

    std::shared_ptr<Object> Eval() {
        return std::shared_ptr<Object>(new Bool(GetName()));
    }

private:
    std::string name_;
};

class Number : public Object {
public:
    Number(ConstantToken* token) : value_(token->value) {
    }

    Number(int64_t v) : value_(v) {
    }

    std::string Serialise() {
        return std::to_string(value_);
    }

    std::shared_ptr<Object> Eval() {
        return std::shared_ptr<Object>(new Number(GetValue()));
    }

    int GetValue() const {
        return value_;
    }

private:
    int64_t value_;
};

class Function : public Object {
public:
    Function(std::string name) : name_(name) {
    }
    std::string Serialise() {
        return name_;
    }

    std::shared_ptr<Object> Eval() {
        return std::shared_ptr<Object>(new Symbol(name_));
    }

    virtual std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> args) = 0;

private:
    std::string name_;
};

class IsNumFunc : public Function {
public:
    IsNumFunc(std::string name) : Function(name) {
    }
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> args) {
        if (args.size() > 1) {
            return std::shared_ptr<Object>(new Bool("#f"));
        }
        if (Is<Number>(args[0])) {
            return std::shared_ptr<Object>(new Bool("#t"));
        }
        return std::shared_ptr<Object>(new Bool("#f"));
    }
};

class IsBoolFunc : public Function {
public:
    IsBoolFunc(std::string name) : Function(name) {
    }
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> args) {
        if (args.size() > 1) {
            return std::shared_ptr<Object>(new Bool("#f"));
        }
        if (Is<Bool>(args[0])) {
            return std::shared_ptr<Object>(new Bool("#t"));
        }
        return std::shared_ptr<Object>(new Bool("#f"));
    }
};

class CompareFunc : public Function {
public:
    CompareFunc(std::string name, const std::function<bool(int64_t, int64_t)> f)
        : Function(name), f_(f) {
    }
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> args) {
        if (args.empty()) {
            throw RuntimeError("");
        }
        if (!Is<Number>(args[0])) {
            throw RuntimeError("");
        }
        if (args.size() == 1) {
            return std::shared_ptr<Object>(new Bool("#t"));
        }
        for (int64_t i = 0; i < args.size() - 1; ++i) {
            if (!Is<Number>(args[i]) || !Is<Number>(args[i + 1])) {
                throw RuntimeError("");
            }
            if (!f_(As<Number>(args[i])->GetValue(), As<Number>(args[i + 1])->GetValue())) {
                return std::shared_ptr<Object>(new Bool("#f"));
            }
        }
        return std::shared_ptr<Object>(new Bool("#t"));
    }

private:
    const std::function<bool(int64_t, int64_t)> f_;
};

class ArifmFunc : public Function {
public:
    ArifmFunc(std::string name, const std::function<int64_t(int64_t, int64_t)> f)
        : Function(name), f_(f) {
    }
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> args) {
        if (args.empty()) {
            throw RuntimeError("");
        }
        if (!Is<Number>(args[0])) {
            throw RuntimeError("");
        }
        if (args.size() == 1) {
            return args[0];
        }
        int64_t ans = As<Number>(args[0])->GetValue();
        for (int64_t i = 1; i < args.size(); ++i) {
            if (!Is<Number>(args[i])) {
                throw RuntimeError("");
            }
            ans = f_(ans, As<Number>(args[i])->GetValue());
        }
        return std::shared_ptr<Object>(new Number(ans));
    }

private:
    const std::function<int64_t(int64_t, int64_t)> f_;
};

class AndFunc : public Function {
public:
    AndFunc(std::string name1, std::string name) : Function(name1) {
    }
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> args) {
        if (args.empty()) {
            throw RuntimeError("");
        }
        if (args.size() == 1) {
            auto s = args[0];
            if (Is<Bool>(s)) {
                if (As<Bool>(s)->GetVal()) {
                    return s;
                }
                return std::shared_ptr<Object>(new Bool("#f"));
            }
            return s;
        }
        for (int64_t i = 0; i < args.size(); ++i) {
            auto s = args[i];
            if (Is<Bool>(s)) {
                if (!As<Bool>(s)->GetVal()) {
                    return std::shared_ptr<Object>(new Bool("#f"));
                }
            }
        }
        return args.back();
    }
};

class OrFunc : public Function {
public:
    OrFunc(std::string name1, std::string name) : Function(name1) {
    }
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> args) {
        if (args.empty()) {
            throw RuntimeError("");
        }
        if (args.size() == 1) {
            auto s = args[0];
            if (Is<Bool>(s)) {
                if (As<Bool>(s)->GetVal()) {
                    return std::shared_ptr<Object>(new Bool("#t"));
                }
                return s;
            }
            return s;
        }
        for (int64_t i = 0; i < args.size(); ++i) {
            auto s = args[i];
            if (Is<Bool>(s)) {
                if (As<Bool>(s)->GetVal()) {
                    return std::shared_ptr<Object>(new Bool("#t"));
                }
            }
        }
        return args.back();
    }
};

class OneArgsIntFunc : public Function {
public:
    OneArgsIntFunc(std::string name, const std::function<int64_t(int64_t)> f)
        : Function(name), f_(f) {
    }
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> args) {
        if (args.empty()) {
            throw RuntimeError("");
        }
        if (!Is<Number>(args[0])) {
            throw RuntimeError("");
        }
        if (args.size() > 1) {
            throw RuntimeError("");
        }
        int64_t ans = f_(As<Number>(args[0])->GetValue());
        return std::shared_ptr<Object>(new Number(ans));
    }

private:
    const std::function<int64_t(int64_t)> f_;
};

class CloseBracket : public Object {
public:
    CloseBracket(Token* token) {
    }
    CloseBracket() {
    }

    std::string Serialise() {
        return ")";
    }

    std::shared_ptr<Object> Eval() {
        return std::shared_ptr<Object>(new CloseBracket());
    }
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

    std::string Serialise() {
        if (!first_ && !second_) {
            return "()";
        }
        if (!first_) {
            throw RuntimeError("");
        }
        if (second_) {
            if (!Is<Cell>(first_) && !Is<Cell>(second_)) {
                return first_->Serialise() + " . " + second_->Serialise();
            }
        }
        std::string ans;
        auto curr = std::shared_ptr<Object>(new Cell{first_, second_});
        while (curr) {
            if (Is<Cell>(curr)) {
                auto f = As<Cell>(curr)->GetFirst();
                auto s = As<Cell>(curr)->GetSecond();
                if (s) {
                    if (!Is<Cell>(f) && !Is<Cell>(s)) {
                        ans += (f->Serialise() + " . " + s->Serialise());
                        curr = nullptr;
                    } else {
                        curr = As<Cell>(curr)->GetSecond();
                        ans += f->Serialise();
                    }
                } else {
                    ans += f->Serialise();
                    curr = nullptr;
                }
                if (curr) {
                    ans += " ";
                }
            } else {
                ans += curr->Serialise();
                curr = nullptr;
            }
        }
        return ans;
    }

    std::shared_ptr<Object> Eval() {
        if (!first_) {
            throw RuntimeError("");
        }
        if (Is<Cell>(first_)) {
            return first_;
        }
        auto f = first_->Eval();
        if (Is<Cell>(second_)) {
            if (Is<Symbol>(As<Cell>(second_)->GetFirst())) {
                second_ = second_->Eval();
            }
        }
        if (Is<Number>(f)) {
            throw RuntimeError("");
            if (second_) {
                throw RuntimeError("");
                //                auto s = second_->Eval();
                //                return std::shared_ptr<Object>(new Cell{f, s});
            } else {
                return f;
            }
        }
        if (Is<Bool>(f)) {
            if (second_) {
                throw RuntimeError("");
                //                auto s = second_->Eval();
                //                return std::shared_ptr<Object>(new Cell{f, s});
            } else {
                return f;
            }
        }
        if (Is<Symbol>(f)) {
            auto name = As<Symbol>(f)->GetName();
            if (name == "quote") {
                if (!second_) {
                    throw RuntimeError("");
                }
                //                if (Is<Number>(second_)) {
                //                    return std::shared_ptr<Object>(new
                //                    Number(As<Number>(second_)->GetValue()));
                //                }
                //                if (Is<Symbol>(second_)) {
                //                    return std::shared_ptr<Object>(new
                //                    Symbol(As<Symbol>(second_)->GetName()));
                //                }
                //                if (Is<Bool>(second_)) {
                //                    return std::shared_ptr<Object>(new
                //                    Bool(As<Bool>(second_)->GetName()));
                //                }
                //
                //                throw RuntimeError("");
                if (Is<Cell>(second_)) {
                    if (!As<Cell>(second_)->GetSecond() &&
                        Is<Cell>(As<Cell>(second_)->GetFirst())) {
                        return As<Cell>(second_)->GetFirst();
                    }
                }
                return second_;

                //            auto ans = As<Symbol>(f)->GetName();
                //            ans += " ";
                //            ans += second_->Serialise();
                //            return std::shared_ptr<Object>(new Symbol(ans));
            }
            if (name == "number?") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto args = ArgsToVector(second_);
                auto func = IsNumFunc("number?");
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == "boolean?") {
                if (!second_) {
                    // return std::shared_ptr<Bool>(new Bool("#f"));
                    throw RuntimeError("");
                }
                auto s = ArgsToVector(second_);
                if (s.size() > 1) {
                    throw RuntimeError("");
                }
                if (s.empty()) {
                    return std::shared_ptr<Bool>(new Bool("#f"));
                }
                if (!Is<Bool>(s[0])) {
                    return std::shared_ptr<Bool>(new Bool("#f"));
                }
                return std::shared_ptr<Bool>(new Bool("#t"));
            }
            if (name == "pair?") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto s = ArgsToVector(second_);
                if (s.size() == 2) {
                    return std::shared_ptr<Bool>(new Bool("#t"));
                }

                return std::shared_ptr<Bool>(new Bool("#f"));
            }
            if (name == "null?") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto s = ArgsToVector(second_);
                if (!s.empty()) {
                    return std::shared_ptr<Bool>(new Bool("#f"));
                }

                return std::shared_ptr<Bool>(new Bool("#t"));
            }
            if (name == "list?") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto s = second_;
                while (Is<Cell>(s)) {
                    if (Is<Cell>(As<Cell>(s)->GetFirst())) {
                        return std::shared_ptr<Bool>(new Bool("#f"));
                    }
                    s = As<Cell>(s)->GetSecond();
                }
                if (!s) {
                    return std::shared_ptr<Bool>(new Bool("#t"));
                }
                return std::shared_ptr<Bool>(new Bool("#f"));
            }
            if (name == "cons") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto s = ArgsToVector(second_);
                if (s.size() != 2) {
                    throw RuntimeError("");
                }

                return std::shared_ptr<Object>(new Cell{s[0], s[1]});
            }
            if (name == "car") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto s = ArgsToVector(second_);
                if (s.empty()) {
                    throw RuntimeError("");
                }

                return s[0];
            }
            if (name == "cdr") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto s = second_;
                if (!Is<Cell>(s)) {
                    throw RuntimeError("");
                }
                if (!As<Cell>(s)->GetFirst() && !As<Cell>(s)->GetSecond()) {
                    throw RuntimeError("");
                }
                if (!Is<Cell>(As<Cell>(s)->GetSecond())) {
                    if (!As<Cell>(s)->GetSecond()) {
                        return std::shared_ptr<Object>(new Cell{nullptr, nullptr});
                    }
                    if (!Is<Cell>(s)) {
                        throw RuntimeError("");
                    }
                    return As<Cell>(s)->GetSecond();
                }
                if (Is<Cell>(As<Cell>(s)->GetSecond())) {
                    if (!Is<Cell>(s)) {
                        throw RuntimeError("");
                    }
                    s = As<Cell>(s)->GetSecond();
                }

                return s;
            }
            if (name == "list-tail") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto s = second_;
                if (!Is<Cell>(s)) {
                    throw RuntimeError("");
                }
                if (!As<Cell>(s)->GetFirst() && !As<Cell>(s)->GetSecond()) {
                    throw RuntimeError("");
                }
                while (Is<Cell>(As<Cell>(s)->GetSecond())) {
                    s = As<Cell>(s)->GetSecond();
                }
                if (!Is<Cell>(s)) {
                    throw RuntimeError("");
                }
                int64_t index = As<Number>((As<Cell>(s)->GetFirst()))->GetValue();
                s = As<Cell>(second_)->GetFirst();
                for (int64_t i = 0; i < index; ++i) {
                    if (!Is<Cell>(s)) {
                        throw RuntimeError("");
                    }
                    s = As<Cell>(s)->GetSecond();
                }
                if (!s) {
                    return std::shared_ptr<Object>(new Cell{nullptr, nullptr});
                }
                return s;
            }
            if (name == "list-ref") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto s = second_;
                if (!Is<Cell>(s)) {
                    throw RuntimeError("");
                }
                if (!As<Cell>(s)->GetFirst() && !As<Cell>(s)->GetSecond()) {
                    throw RuntimeError("");
                }
                while (Is<Cell>(As<Cell>(s)->GetSecond())) {
                    s = As<Cell>(s)->GetSecond();
                }
                int64_t index = As<Number>((As<Cell>(s)->GetFirst()))->GetValue();
                if (!Is<Cell>(s)) {
                    throw RuntimeError("");
                }
                s = As<Cell>(second_)->GetFirst();
                for (int64_t i = 0; i < index; ++i) {
                    if (!Is<Cell>(s)) {
                        throw RuntimeError("");
                    }
                    s = As<Cell>(s)->GetSecond();
                }
                if (!s) {
                    throw RuntimeError("");
                }
                if (!Is<Cell>(s)) {
                    throw RuntimeError("");
                }
                return As<Cell>(s)->GetFirst();
            }
            if (name == "list") {
                if (!second_) {
                    return std::shared_ptr<Object>(new Cell{nullptr, nullptr});
                }
                auto s = second_;
                if (!Is<Cell>(s)) {
                    return std::shared_ptr<Object>(new Cell{s, nullptr});
                }

                return s;
            }
            if (name == "=") {
                if (!second_) {
                    return f;
                }
                auto args = ArgsToVector(second_);
                auto f = [](int64_t a, int64_t b) { return a == b; };
                auto func = CompareFunc("=", f);
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == ">") {
                if (!second_) {
                    return f;
                }
                auto args = ArgsToVector(second_);
                auto f = [](int64_t a, int64_t b) { return a > b; };
                auto func = CompareFunc("=", f);
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == "<") {
                if (!second_) {
                    return f;
                }
                auto args = ArgsToVector(second_);
                auto f = [](int64_t a, int64_t b) { return a < b; };
                auto func = CompareFunc("=", f);
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == ">=") {
                if (!second_) {
                    return f;
                }
                auto args = ArgsToVector(second_);
                auto f = [](int64_t a, int64_t b) { return a >= b; };
                auto func = CompareFunc("=", f);
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == "<=") {
                if (!second_) {
                    return f;
                }
                auto args = ArgsToVector(second_);
                auto f = [](int64_t a, int64_t b) { return a <= b; };
                auto func = CompareFunc("=", f);
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == "+") {
                if (!second_) {
                    return f;
                }
                auto args = ArgsToVector(second_);
                auto f = [](int64_t a, int64_t b) { return a + b; };
                auto func = ArifmFunc("+", f);
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == "-") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto args = ArgsToVector(second_);
                auto f = [](int64_t a, int64_t b) { return a - b; };
                auto func = ArifmFunc("-", f);
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == "*") {
                if (!second_) {
                    return f;
                }
                auto args = ArgsToVector(second_);
                auto f = [](int64_t a, int64_t b) { return a * b; };
                auto func = ArifmFunc("*", f);
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == "/") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto args = ArgsToVector(second_);
                auto f = [](int64_t a, int64_t b) { return a / b; };
                auto func = ArifmFunc("/", f);
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == "min") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto args = ArgsToVector(second_);
                auto f = [](int64_t a, int64_t b) { return std::min(a, b); };
                auto func = ArifmFunc("min", f);
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == "max") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto args = ArgsToVector(second_);
                auto f = [](int64_t a, int64_t b) { return std::max(a, b); };
                auto func = ArifmFunc("max", f);
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == "abs") {
                if (!second_) {
                    throw RuntimeError("");
                }
                auto args = ArgsToVector(second_);
                auto f = [](int64_t a) { return std::abs(a); };
                auto func = OneArgsIntFunc("abs", f);
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == "not") {
                if (!second_) {
                    throw RuntimeError("");
                }
                if (Is<Cell>(second_)) {
                    if (!As<Cell>(second_)->GetFirst() && !As<Cell>(second_)->GetSecond()) {
                        return std::shared_ptr<Object>(new Bool("#f"));
                    }
                }
                auto s = ArgsToVector(second_);
                if (s.size() > 1) {
                    throw RuntimeError("");
                }
                if (s.empty()) {
                    throw RuntimeError("");
                }
                if (Is<Bool>(s[0])) {
                    if (As<Bool>(s[0])->GetVal()) {
                        return std::shared_ptr<Object>(new Bool("#f"));
                    }
                    return std::shared_ptr<Object>(new Bool("#t"));
                }
                return std::shared_ptr<Object>(new Bool("#f"));
            }
            if (name == "and") {
                if (!second_) {
                    return f;
                }
                auto args = ArgsToVector(second_);
                auto func = AndFunc(std::string(), "and");
                auto ans = func.Apply(args);
                return ans;
            }
            if (name == "or") {
                if (!second_) {
                    return f;
                }
                auto args = ArgsToVector(second_);
                auto func = OrFunc(std::string(), "or");
                auto ans = func.Apply(args);
                return ans;
            }
            if (!second_) {
                return std::shared_ptr<Object>(new Cell{first_, nullptr});
            }
            throw RuntimeError("");
        } else {
            throw RuntimeError("");
            //            if (!second_) {
            //                return f;
            //            }
            //            auto s = second_->Eval();
            //            return std::shared_ptr<Object>(new Cell{f, s});
        }
        return nullptr;
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

inline std::vector<std::shared_ptr<Object>> ArgsToVector(std::shared_ptr<Object> curr) {
    std::unordered_set<std::string> functions{
        "quote", "+",     "-",     "/",    "-",       "=",        "<",    ">",         ">=",
        "<=",    "min",   "max",   "abs",  "number?", "boolean?", "not",  "and",       "or",
        "pair?", "null?", "list?", "cons", "car",     "cdr",      "list", "list-tail", "list-ref"};

    if (!Is<Cell>(curr)) {
        throw RuntimeError("");
    }
    std::vector<std::shared_ptr<Object>> ans(0);
    if (As<Cell>(curr)->GetFirst()) {
        if (Is<Symbol>(As<Cell>(curr)->GetFirst())) {
            if (functions.contains(As<Symbol>(As<Cell>(curr)->GetFirst())->GetName())) {
                curr = curr->Eval();
            }
        }
    }
    if (!Is<Cell>(curr)) {
        throw RuntimeError("");
    }
    if (As<Cell>(curr)->GetFirst()) {
        auto val = As<Cell>(curr)->GetFirst();
        //        if (Is<Symbol>(As<Cell>(curr)->GetFirst())) {
        //            auto temp = curr->Eval();
        //            val = (As<Cell>(curr)->GetFirst())->Eval();
        //        }
        ans.push_back(val);
        if (Is<Cell>(curr)) {
            curr = As<Cell>(curr)->GetSecond();
        } else {
            curr = nullptr;
        }
        while (curr) {
            val = curr;
            // val = curr->Eval();
            if (Is<Cell>(val)) {
                if (As<Cell>(val)->GetFirst()) {
                    if (!Is<Cell>(As<Cell>(val)->GetFirst())) {
                        ans.push_back(As<Cell>(val)->GetFirst());
                        curr = As<Cell>(curr)->GetSecond();
                    } else {
                        while (Is<Cell>(As<Cell>(val)->GetFirst())) {
                            val = As<Cell>(val)->GetFirst();
                        }
                        if (Is<Symbol>(As<Cell>(val)->GetFirst())) {
                            if (functions.contains(
                                    As<Symbol>(As<Cell>(val)->GetFirst())->GetName())) {
                                val = val->Eval();
                            }
                        }
                        ans.push_back(val);
                        break;
                    }
                } else {
                    curr = nullptr;
                }
            } else {
                ans.push_back(val);
                break;
            }
        }
    }
    return ans;
}