#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <variant>
#include <vector>

enum TokenType { NUMBER, ADD, SUM, RANGE, END_OF_FILE };
struct Token {
  TokenType type;
  std::string value;
};

struct Scanner {
  std::string src;
  size_t pos = 0;
  char current() { return pos < src.size() ? src[pos] : '\0'; }
  char peek() { return (pos + 1) < src.size() ? src[pos + 1] : '\0'; }
};

Token next_token(Scanner &s) {
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

enum Err {ERR_FAIL};

struct Value { std::vector<int> data; };
struct BinOp;
struct UnaryOp;
using Node = std::variant<Value, std::unique_ptr<BinOp>, std::unique_ptr<UnaryOp>>;

struct BinOp { TokenType op; Node left, right; };
struct UnaryOp { TokenType op; Node operand; };

Node parse_expr(Scanner &s, Token &cur);
Node parse_unary(Scanner &s, Token &cur);
Node parse_range(Scanner &s, Token &cur);
Node parse_term(Scanner &s, Token &cur);

Node parse_term(Scanner &s, Token &cur) {
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

Node parse_range(Scanner &s, Token &cur) {
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

Node parse_unary(Scanner &s, Token &cur) {
  if (cur.type == SUM) {
    auto node = std::make_unique<UnaryOp>();
    node->op = SUM;
    cur = next_token(s);
    node->operand = parse_unary(s, cur);
    return node;
  }
  return parse_range(s, cur);
}

Node parse_expr(Scanner &s, Token &cur) {
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

std::string to_str(Value v) {
  if (v.data.empty()) {
    return "";
  }
  if (v.data.size() == 1) {
    return std::to_string(v.data[0]);
  }
  std::string res;
  for (int i : v.data) {
    res += std::to_string(i) + " ";
  }

  return res;
}

Value eval(const Node &node) {
  if (const auto *v = std::get_if<Value>(&node)) return *v;

  if (const auto *un = std::get_if<std::unique_ptr<UnaryOp>>(&node)) {
    Value res = eval((*un)->operand);
    return Value{{std::accumulate(res.data.begin(), res.data.end(), 0)}};
  }

  const auto &bin = std::get<std::unique_ptr<BinOp>>(node);
  Value L = eval(bin->left);
  Value R = eval(bin->right);

  if (bin->op == RANGE) {
    Value res;
    if (L.data.empty() || R.data.empty()) return res;
    int start = L.data[0], end = R.data.back();
    if (start <= end) 
      for (int i = start; i <= end; ++i) res.data.push_back(i);
    else 
      for (int i = start; i >= end; --i) res.data.push_back(i);
    return res;
  }

  if (bin->op == ADD) {
    if (L.data.size() == 1) {
      for (int &x : R.data) x += L.data[0];
      return R;
    }
    if (R.data.size() == 1) {
      for (int &x : L.data) x += R.data[0];
      return L;
    }
    size_t n = std::min(L.data.size(), R.data.size());
    Value res;
    for (size_t i = 0; i < n; ++i) res.data.push_back(L.data[i] + R.data[i]);
    return res;
  }
  return Value{{0}};
}

Value run(std::string src) {
  Scanner s{src};
  Token cur = next_token(s);
  return eval(parse_expr(s, cur));
}

int main() {
  for (;;) {
    std::string in;
    std::getline(std::cin, in);
    std::cout << to_str(run(in)) << std::endl;
  }
  return 0;
}
