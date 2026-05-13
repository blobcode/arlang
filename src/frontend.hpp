#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <cctype>

enum TokenType { 
    NUMBER, 
    ADD, 
    SUM, 
    RANGE, 
    END_OF_FILE 
};

struct Token {
    TokenType type;
    std::string value;
};

struct Scanner {
    std::string src;
    size_t pos = 0;

    char current() const { return pos < src.size() ? src[pos] : '\0'; }
    char peek() const { return (pos + 1) < src.size() ? src[pos + 1] : '\0'; }
};


struct Value { 
    std::vector<int> data; 
};

struct BinOp;
struct UnaryOp;

using Node = std::variant<Value, std::unique_ptr<BinOp>, std::unique_ptr<UnaryOp>>;

struct BinOp { 
    TokenType op; 
    Node left; 
    Node right; 
};

struct UnaryOp { 
    TokenType op; 
    Node operand; 
};


inline Token next_token(Scanner &s);
inline Node parse_expr(Scanner &s, Token &cur);
inline Node parse_unary(Scanner &s, Token &cur);
inline Node parse_range(Scanner &s, Token &cur);
inline Node parse_term(Scanner &s, Token &cur);

inline Token next_token(Scanner &s) {
    while (isspace(s.current())) s.pos++;
    if (s.pos >= s.src.size()) return {END_OF_FILE, ""};

    if (isdigit(s.current())) {
        std::string val;
        while (isdigit(s.current())) val += s.src[s.pos++];
        return {NUMBER, val};
    }
    if (s.current() == '+') {
        if (s.peek() == '/') {
            s.pos += 2;
            return {SUM, "+/"};
        }
        s.pos++;
        return {ADD, "+"};
    }
    if (s.current() == '.' && s.peek() == '.') {
        s.pos += 2;
        return {RANGE, ".."};
    }
    return {END_OF_FILE, ""};
}

inline Node parse_term(Scanner &s, Token &cur) {
    if (cur.type == NUMBER) {
        Value v;
        while (cur.type == NUMBER) {
            v.data.push_back(std::stoi(cur.value));
            cur = next_token(s);
        }
        return v;
    }
    return Value{{0}};
}

inline Node parse_range(Scanner &s, Token &cur) {
    Node node = parse_term(s, cur);
    while (cur.type == RANGE) {
        auto bin = std::make_unique<BinOp>();
        bin->op = RANGE;
        cur = next_token(s);
        bin->left = std::move(node);
        bin->right = parse_term(s, cur);
        node = std::move(bin);
    }
    return node;
}

inline Node parse_unary(Scanner &s, Token &cur) {
    if (cur.type == SUM) {
        auto node = std::make_unique<UnaryOp>();
        node->op = SUM;
        cur = next_token(s);
        node->operand = parse_unary(s, cur);
        return node;
    }
    return parse_range(s, cur);
}

inline Node parse_expr(Scanner &s, Token &cur) {
    Node node = parse_unary(s, cur);
    while (cur.type == ADD) {
        auto bin = std::make_unique<BinOp>();
        bin->op = ADD;
        cur = next_token(s);
        bin->left = std::move(node);
        bin->right = parse_unary(s, cur);
        node = std::move(bin);
    }
    return node;
}