#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <variant>
#include <vector>
#include "frontend.hpp"

enum Err {ERR_FAIL};

// converts a value to string
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
