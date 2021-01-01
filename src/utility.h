#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <string_view>

[[noreturn]] void unreachable();
[[noreturn]] void error(int line, const std::string_view &msg);

std::ostream &repeat(std::ostream &os, char input, size_t num);

extern bool do_constant_propagation;

template <typename T> class save_and_restore {
  T &place;
  T original;

public:
  save_and_restore(T &x, T new_value) : place(x), original(x) {
    place = new_value;
  }
  ~save_and_restore() { place = original; }
};

#endif // UTILITY_H
