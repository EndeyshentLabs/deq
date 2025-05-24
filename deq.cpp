/*
 * Copyright (c) 2024 EndeyshentLabs <Themikfound@gmail.com>
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <array>
#include <deque>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstring>

using s64 = std::int64_t;
using s32 = std::int32_t;
using s16 = std::int16_t;
using s8 = std::int8_t;

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8 = std::uint8_t;

using usz = std::uintptr_t;
using ssz = std::intptr_t;

using f32 = float;
using f64 = double;

#define UNREACHABLE()                                                          \
    do {                                                                       \
        std::cerr << __FILE__ << ":" << __LINE__ << ": Unreachable in "        \
                  << __PRETTY_FUNCTION__ << std::endl;                         \
        abort();                                                               \
    } while (0)

struct Location {
    std::string filename;
    u64 col;
    u64 row;
};

std::ostream& operator<<(std::ostream& os, const Location& loc)
{
    os << loc.filename << ':' << (loc.row + 1) << ':' << (loc.col + 1);
    return os;
}

struct Token {
    const Location loc;
    std::string text;
};

struct Value {
    using As = std::variant<s64, f64, std::string>;
    enum class Type {
        Integer = 0,
        Real,
        String,
    };

    Value(const Token& tok, s64 num)
        : tok(tok)
    {
        type = Type::Integer;
        as = num;
    }

    Value(const Token& tok, f64 real)
        : tok(tok)
    {
        type = Type::Real;
        as = real;
    }

    Value(const Token& tok, std::string str)
        : tok(tok)
    {
        type = Type::String;
        as = str;
    }

    Type type;
    As as;
    const Token& tok;
};

static std::string human(Value::Type t, bool plural = false)
{
    switch (t) {
    case Value::Type::Integer:
        return plural ? "integers" : "an integer";
    case Value::Type::Real:
        return plural ? "reals" : "a real";
    case Value::Type::String:
        return plural ? "strings" : "a string";
    default:
        UNREACHABLE();
    }
}

std::ostream& operator<<(std::ostream& os, const Value& v)
{
    if (auto* num = std::get_if<s64>(&v.as)) {
        os << *num;
    } else if (auto* real = std::get_if<f64>(&v.as)) {
        os << *real;
    } else if (auto* str = std::get_if<std::string>(&v.as)) {
        os << *str;
    }

    return os;
}

using deq_t = Value;

class Lexer {
public:
    Lexer(std::string filename)
    {
        loc = Location { filename, 0, 0 };

        std::ifstream infile { filename };
        if (!infile.is_open()) {
            std::cerr << "[ERR] Failed to open file '" << filename
                      << "': " << std::strerror(errno) << '\n';
            std::exit(1);
        }

        std::stringstream buffer;
        buffer << infile.rdbuf();
        source = buffer.str();
        if (source.size() == 0) {
            std::cerr << "[WRN] File '" << filename << "' is empty\n";
        }
        c = source[0];
    }
    std::vector<Token> lex();

private:
    Location loc;
    char c {};
    usz cursor {};
    std::string source;
    bool advance();
    char peek();
    bool isend();
};

bool Lexer::isend()
{
    if ((cursor > source.size() - 1) || (cursor + 1 > source.size() - 1)
        || c == '\0') {
        c = 0;
        cursor = -1;
        return true;
    }

    return false;
}

bool Lexer::advance()
{
    cursor++;
    c = source.at(cursor);
    loc.col++;

    isend();

    return true;
}

char Lexer::peek()
{
    if (cursor + 1 > source.size() - 1) {
        return 0;
    }
    return source.at(cursor + 1);
}

std::vector<Token> Lexer::lex()
{
    std::vector<Token> tox;

    while (c != '\0') {
        if (c == '\n') {
            advance();
            loc.row++;
            loc.col = 0;
        } else if (std::isspace(c)) {
            advance();
        } else if (c == '#') {
            do {
                advance();
            } while (c != '\n' && c != '\0');
        } else if (c == '"' || (c == '!' && peek() == '"')) {
            std::stringstream ss;
            auto sloc = loc;

            if (c == '!') {
                ss << c;
                advance();
            }

            do {
                if (c == '\0') {
                    std::cerr << sloc << ": [ERR] Unclosed string!\n";
                }
                ss << c;
                advance();
            } while (c != '"');

            // Eat "
            ss << c;
            advance();

            if (c == '!') {
                ss << c;
                advance();
            }

            tox.push_back({ sloc, ss.str() });
        } else {
            std::stringstream ss;
            auto sloc = loc;
            do {
                ss << c;
                advance();
            } while (!std::isspace(c) && c != '\0');
            tox.push_back({ sloc, ss.str() });
        }
    }

    return tox;
}

#define NOTE(msg) NOTET(token, msg)

#define NOTET(token, msg)                                                      \
    std::cout << std::endl << token.loc << ": [NOTE] " << msg << '\n'

#define ERR(msg) ERRT(token, msg)

#define ERRT(token, msg)                                                       \
    std::cerr << std::endl << token.loc << ": [ERR] " << msg << '\n'

static void trace(const std::deque<deq_t>& deq)
{
    for (const auto& v : deq) {
        std::cout << v << "(" << human(v.type) << ")"
                  << " ";
    }
    std::cout << '\n';
}

struct TypecheckResult {
    usz i;
    Value val;
    Value::Type expected;
};

template <usz Nm>
static std::optional<TypecheckResult> typecheck(
    const std::array<Value, Nm>& vals, const std::array<Value::Type, Nm>& types)
{
    for (usz i = 0; i < Nm; i++) {
        if (vals.at(i).type != types.at(i)) {
            return TypecheckResult { i, vals.at(i), types.at(i) };
        }
    }

    return {};
}

static bool diag(std::optional<TypecheckResult> r, Token token)
{
    if (!r)
        return true;

    ERRT(r->val.tok,
        "expected to be " << human(r->expected) << " but got "
                          << human(r->val.type));
    NOTE("for this operation");

    return false;
}

#define DIAG(v)                                                                \
    do {                                                                       \
        if (!diag(v, token)) {                                                 \
            std::exit(1);                                                      \
        }                                                                      \
    } while (0)

static void interpret(const std::vector<Token>& tox, bool debug = false)
{
    std::deque<deq_t> deq;
    std::vector<std::tuple<usz, bool>> callstack;
    std::unordered_map<std::string, usz> labels;
    bool inverted = false;

    {
        usz i = 0;
        for (const auto& token : tox) {
            const auto& tok = token.text;

            if (tok.back() == ':') {
                const auto& word = tok.substr(0, tok.size() - 1);

                if (labels.contains(word)) {
                    ERR("label '" << word << "' is already defined!");
                }

                labels.insert({ word, i });
            }

            i++;
        }
    }

    for (usz i = 0; i < tox.size();) {
        const auto& token = tox.at(i);
        const auto& tok = token.text;
        bool left = false;

        if (tok == "trace") {
            trace(deq);

            i++;
            continue;
        } else if (tok == "ret") {
            if (callstack.size() < 1) {
                ERR("cannot return: call stack is empty!");
                std::exit(1);
            }

            i = std::get<0>(callstack.back()) + 1;
            callstack.pop_back();
            continue;
        } else if (tok == "exit") {
            break;
        }

        if (tok.size() < 2) {
            ERR("token of size less than 2 is impossible!");
            std::exit(1);
        }

        if (tok.front() != '!' && tok.back() != '!' && tok.back() != ':') {
            ERR("not a label and no direction specified!");
            std::exit(1);
        }
        if (tok.back() == ':' && tok.front() == '!') {
            ERR("label cannot contain direction specifier! Consider removing "
                "'!', if "
                "it is a label.");
            std::exit(1);
        }

        auto word = tok;
        if (tok.back() == ':') {
            i++;
            continue;
        } else if (tok.front() == '!') {
            left = true;
            word = word.substr(1);
        } else if (tok.back() == '!') {
            left = false;
            word = word.substr(0, word.size() - 1);
        }

        auto push = [left, inverted, &deq](deq_t v) {
            bool dir = inverted ? !left : left;
            if (dir) {
                deq.push_front(v);
            } else {
                deq.push_back(v);
            }
        };

        auto pop = [left, inverted, &deq]() -> deq_t {
            bool dir = inverted ? !left : left;
            if (dir) {
                deq_t ret = deq.front();
                deq.pop_front();
                return ret;
            } else {
                deq_t ret = deq.back();
                deq.pop_back();
                return ret;
            }
        };

        auto expect = [&deq, &token](usz n) {
            if (deq.size() < n) {
                ERR("expected to have at least " << n
                                                 << " elements on the deq");
                std::exit(1);
            }
        };

        using enum Value::Type;
        if ((word.front() == '-' && word.back() == 'f')
            || (std::isdigit(word.front()) && word.back() == 'f')) {
            push({ token, static_cast<f64>(std::stod(word)) });

            i++;
        } else if (word.front() == '-' || std::isdigit(word.front())) {
            push({ token, static_cast<s64>(std::stoll(word)) });

            i++;
        } else if (word.front() == '"' && word.back() == '"') {
            push({ token, word.substr(1, word.size() - 2) });

            i++;
        } else if (word == "drop") {
            expect(1);
            (void)pop();

            i++;
        } else if (word == "dup") {
            expect(1);
            deq_t v = pop();
            push(v);
            push(v);

            i++;
        } else if (word == "swap") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            push(top);
            push(below);

            i++;
        } else if (word == "move") {
            expect(1);
            deq_t v = pop();
            left = !left;
            push(v);
            left = !left;

            i++;
        } else if (word == "rot") {
            expect(3);
            deq_t top = pop(), below = pop(), under = pop();
            push(under);
            push(top);
            push(below);

            i++;
        } else if (word == "over") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            push(below);
            push(top);
            push(below);

            i++;
        } else if (word == "add") {
            expect(2);
            deq_t v2 = pop();
            deq_t v1 = pop();
            if (v1.type == Integer || v2.type == Integer) {
                DIAG(typecheck<2>({ v1, v2 }, { Integer, Integer }));
                push({ token, std::get<s64>(v1.as) + std::get<s64>(v2.as) });
            } else if (v1.type == Real || v2.type == Real) {
                DIAG(typecheck<2>({ v1, v2 }, { Real, Real }));
                push({ token, std::get<f64>(v1.as) + std::get<f64>(v2.as) });
            } else {
                ERR("expected two " << human(Integer, true) << " or two "
                                    << human(Real, true));
                std::exit(1);
            }

            i++;
        } else if (word == "mul") {
            expect(2);
            deq_t v2 = pop();
            deq_t v1 = pop();
            if (v1.type == Integer || v2.type == Integer) {
                DIAG(typecheck<2>({ v1, v2 }, { Integer, Integer }));
                push({ token, std::get<s64>(v1.as) * std::get<s64>(v2.as) });
            } else if (v1.type == Real || v2.type == Real) {
                DIAG(typecheck<2>({ v1, v2 }, { Real, Real }));
                push({ token, std::get<f64>(v1.as) * std::get<f64>(v2.as) });
            } else {
                ERR("expected two " << human(Integer, true) << " or two "
                                    << human(Real, true));
                std::exit(1);
            }

            i++;
        } else if (word == "sub") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            if (below.type == Integer || top.type == Integer) {
                DIAG(typecheck<2>({ below, top }, { Integer, Integer }));
                push(
                    { token, std::get<s64>(below.as) - std::get<s64>(top.as) });
            } else if (below.type == Real || top.type == Real) {
                DIAG(typecheck<2>({ below, top }, { Real, Real }));
                push(
                    { token, std::get<f64>(below.as) - std::get<f64>(top.as) });
            } else {
                ERR("expected two " << human(Integer, true) << " or two "
                                    << human(Real, true));
                std::exit(1);
            }

            i++;
        } else if (word == "div") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            if (below.type == Integer || top.type == Integer) {
                DIAG(typecheck<2>({ below, top }, { Integer, Integer }));
                push(
                    { token, std::get<s64>(below.as) / std::get<s64>(top.as) });
            } else if (below.type == Real || top.type == Real) {
                DIAG(typecheck<2>({ below, top }, { Real, Real }));
                push(
                    { token, std::get<f64>(below.as) / std::get<f64>(top.as) });
            } else {
                ERR("expected two " << human(Integer, true) << " or two "
                                    << human(Real, true));
                std::exit(1);
            }

            i++;
        } else if (word == "mod") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            DIAG(typecheck<2>({ below, top }, { Integer, Integer }));
            push({ token, std::get<s64>(below.as) % std::get<s64>(top.as) });

            i++;
        } else if (word == "shr") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            DIAG(typecheck<2>({ below, top }, { Integer, Integer }));
            push({ token, std::get<s64>(below.as) - std::get<s64>(top.as) });

            i++;
        } else if (word == "shl") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            DIAG(typecheck<2>({ below, top }, { Integer, Integer }));
            push({ token, std::get<s64>(below.as) - std::get<s64>(top.as) });

            i++;
        } else if (word == "band") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            DIAG(typecheck<2>({ below, top }, { Integer, Integer }));
            push({ token, std::get<s64>(below.as) - std::get<s64>(top.as) });

            i++;
        } else if (word == "bor") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            DIAG(typecheck<2>({ below, top }, { Integer, Integer }));
            push({ token, std::get<s64>(below.as) - std::get<s64>(top.as) });

            i++;
        } else if (word == "bnot") {
            expect(1);
            deq_t v = pop();
            DIAG(typecheck<1>({ v }, { Integer }));
            push({ token, ~std::get<s64>(v.as) });

            i++;
        } else if (word == "eq") {
            expect(2);
            deq_t v1 = pop();
            deq_t v2 = pop();
            if (v1.type == Integer || v2.type == Integer) {
                DIAG(typecheck<2>({ v1, v2 }, { Integer, Integer }));
                push({ token,
                    static_cast<s64>(
                        std::get<s64>(v1.as) == std::get<s64>(v2.as)) });
            } else if (v1.type == Real || v2.type == Real) {
                DIAG(typecheck<2>({ v1, v2 }, { Real, Real }));
                push({ token,
                    static_cast<s64>(
                        std::get<f64>(v1.as) == std::get<f64>(v2.as)) });
            } else if (v1.type == String || v2.type == String) {
                DIAG(typecheck<2>({ v1, v2 }, { String, String }));
                push({ token,
                    static_cast<s64>(std::get<std::string>(v1.as)
                        == std::get<std::string>(v2.as)) });
            }

            i++;
        } else if (word == "neq") {
            expect(2);
            deq_t v1 = pop();
            deq_t v2 = pop();
            if (v1.type == Integer || v2.type == Integer) {
                DIAG(typecheck<2>({ v1, v2 }, { Integer, Integer }));
                push({ token,
                    static_cast<s64>(
                        std::get<s64>(v1.as) != std::get<s64>(v2.as)) });
            } else if (v1.type == Real || v2.type == Real) {
                DIAG(typecheck<2>({ v1, v2 }, { Real, Real }));
                push({ token,
                    static_cast<s64>(
                        std::get<f64>(v1.as) != std::get<f64>(v2.as)) });
            } else if (v1.type == String || v2.type == String) {
                DIAG(typecheck<2>({ v1, v2 }, { String, String }));
                push({ token,
                    static_cast<s64>(std::get<std::string>(v1.as)
                        != std::get<std::string>(v2.as)) });
            }

            i++;
        } else if (word == "lt") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            DIAG(typecheck<2>({ below, top }, { Integer, Integer }));
            push({ token,
                static_cast<s64>(
                    std::get<s64>(below.as) < std::get<s64>(top.as)) });

            i++;
        } else if (word == "lteq") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            DIAG(typecheck<2>({ below, top }, { Integer, Integer }));
            push({ token,
                static_cast<s64>(
                    std::get<s64>(below.as) <= std::get<s64>(top.as)) });

            i++;
        } else if (word == "gt") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            DIAG(typecheck<2>({ below, top }, { Integer, Integer }));
            push({ token,
                static_cast<s64>(
                    std::get<s64>(below.as) > std::get<s64>(top.as)) });

            i++;
        } else if (word == "gteq") {
            expect(2);
            deq_t top = pop();
            deq_t below = pop();
            DIAG(typecheck<2>({ below, top }, { Integer, Integer }));
            push({ token,
                static_cast<s64>(
                    std::get<s64>(below.as) >= std::get<s64>(top.as)) });

            i++;
        } else if (word == "and") {
            expect(2);
            deq_t v2 = pop();
            deq_t v1 = pop();
            DIAG(typecheck<2>({ v1, v2 }, { Integer, Integer }));
            push({ token,
                static_cast<s64>(
                    std::get<s64>(v1.as) && std::get<s64>(v2.as)) });

            i++;
        } else if (word == "or") {
            expect(2);
            deq_t v2 = pop();
            deq_t v1 = pop();
            DIAG(typecheck<2>({ v1, v2 }, { Integer, Integer }));
            push({ token,
                static_cast<s64>(
                    std::get<s64>(v1.as) || std::get<s64>(v2.as)) });

            i++;
        } else if (word == "not") {
            expect(1);
            deq_t v = pop();
            DIAG(typecheck<1>({ v }, { Integer }));
            push({ token, static_cast<s64>(!std::get<s64>(v.as)) });

            i++;
        } else if (word == "jmp") {
            expect(1);
            deq_t v = pop();
            DIAG(typecheck<1>({ v }, { Integer }));

            i = std::get<s64>(v.as);
        } else if (word == "call") {
            expect(1);
            deq_t v = pop();
            DIAG(typecheck<1>({ v }, { Integer }));

            callstack.push_back({ i, left });
            i = std::get<s64>(v.as);
        } else if (word == "jz") {
            expect(2);
            deq_t addr = pop();
            deq_t v = pop();
            DIAG(typecheck<2>({ v, addr }, { Integer, Integer }));

            if (std::get<s64>(v.as) == 0) {
                i = std::get<s64>(addr.as);
            } else {
                i++;
            }
        } else if (word == "jnz") {
            expect(2);
            deq_t addr = pop();
            deq_t v = pop();

            DIAG(typecheck<2>({ v, addr }, { Integer, Integer }));

            if (std::get<s64>(v.as) != 0) {
                i = std::get<s64>(addr.as);
            } else {
                i++;
            }
        } else if (word == "print") {
            expect(1);
            deq_t v = pop();
            std::cout << v;

            i++;
        } else if (word == "println") {
            expect(1);
            deq_t v = pop();
            std::cout << v << '\n';

            i++;
        } else if (word == "putc") {
            expect(1);
            deq_t v = pop();
            DIAG(typecheck<1>({ v }, { Integer }));
            std::cout << static_cast<char>(std::get<s64>(v.as));

            i++;
        } else if (word == "calldir") {
            push({ token, static_cast<s64>(std::get<1>(callstack.back())) });

            i++;
        } else if (word == "invertdir") {
            inverted = !inverted;
            push({ token, static_cast<s64>(left) });

            i++;
        } else if (word == "setinverted") {
            expect(1);
            deq_t v = pop();
            DIAG(typecheck<1>({ v }, { Integer }));
            inverted = static_cast<bool>(std::get<s64>(v.as));

            i++;
        } else if (word == ">real") {
            expect(1);
            deq_t v = pop();

            switch (v.type) {
            case Integer:
                DIAG(typecheck<1>({ v }, { Integer }));
                push({ token, static_cast<f64>(std::get<s64>(v.as)) });
                break;
            case String:
                DIAG(typecheck<1>({ v }, { String }));
                push({ token,
                    static_cast<f64>(std::stod(std::get<std::string>(v.as))) });
                break;
            case Real:
                ERR("expected " << human(Integer) << " or " << human(String));
                std::exit(1);
                UNREACHABLE();
            }

            i++;
        } else if (word == ">integer") {
            expect(1);
            deq_t v = pop();

            switch (v.type) {
            case Real:
                DIAG(typecheck<1>({ v }, { Real }));
                push({ token, static_cast<s64>(std::get<f64>(v.as)) });
                break;
            case String:
                DIAG(typecheck<1>({ v }, { String }));
                push({ token,
                    static_cast<s64>(
                        std::stoll(std::get<std::string>(v.as))) });
                break;
            case Integer:
                ERR("expected " << human(Real) << " or " << human(String));
                std::exit(1);
                UNREACHABLE();
            }

            i++;
        } else if (word == ">string") {
            expect(1);
            deq_t v = pop();
            switch (v.type) {
            case Integer:
                DIAG(typecheck<1>({ v },
                    { Integer })); // NOTE: Is this really needed? <2025-05-24>
                push({ token, std::to_string(std::get<s64>(v.as)) });
                break;
            case Real:
                DIAG(typecheck<1>({ v },
                    { Real })); // NOTE: Is this really needed? <2025-05-24>
                push({ token, std::to_string(std::get<f64>(v.as)) });
                break;
            case String:
                ERR("expected " << human(Integer) << " or " << human(Real));
                std::exit(1);
                UNREACHABLE();
            }

            i++;
        } else {
            if (labels.contains(word)) {
                push({ token, static_cast<s64>(labels.at(word)) });
                i++;
            } else {
                ERR("unexpected token");
                std::exit(1);
            }
        }

        if (debug) {
            std::cout << "\nCALLSTACK: ";
            for (const auto& [i, isleft] : callstack) {
                std::cout << i;
            }
            std::cout << '\n';

            std::cout << "DEQUE STATE(inverted: " << inverted << "): ";
            trace(deq);
        }
    }
}

static void usage(const char* program)
{
    std::cout << "Usage: " << program << " [-d] file.deq\n";
}

int main(int argc, char** argv)
{
    const char* program = argv[0];
    if (argc < 2) {
        std::cerr << "No input file was provided!\n";
        usage(program);
        return 1;
    }

    bool debug = false;
    const char* source = nullptr;

    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];
        if (std::strcmp(arg, "-d") == 0) {
            debug = true;
        } else {
            if (source != nullptr) {
                std::cerr << "unexpected CLI argument '" << arg << "'\n";
                usage(program);
                return 1;
            }
            source = arg;
        }
    }

    Lexer l(source);
    auto tox = l.lex();

    interpret(tox, debug);
}
