#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include <functional>
#include <string>

#include "./iter.hpp"

namespace ptyps::string {
  template <typename ...A>
    std::string format(std::string_view form, A ...args) {
      auto length = std::snprintf(nullptr, 0, &form[0], args...);
      auto out = std::string();
      out.resize(length);
      std::sprintf(&out[0], &form[0], args...);
      return out;
    }

  template <typename C>
    inline std::string join(C cont, std::string delm = "") {
      auto out = std::string();

      for (auto next : cont) {
        if (!out.empty())
          out += delm;

        out += next;
      }

      return out;
    }

  inline void split(std::string text, std::string_view d, std::function<void(std::string)> func) {
    for (int i = text.find(d); i != EOF; i = text.find(d)) {
      func(text.substr(0, i));
      text.erase(0, i + d.size());
    }

    if (!text.empty())
      func(text);
  }

  template <template <typename> typename C>
    inline C<std::string> split(std::string text, std::string_view d) {
      auto out = C<std::string>();

      split(text, d, [&](std::string next) {
        out.push_back(next);
      });

      return out;
    }

  void rtrim(std::string &str) {
    if (str.empty())
      return;

    auto rbegin = std::rbegin(str);
    auto rend = std::rend(str);

    auto iter = std::find_if(rbegin, rend, [](char ch) {
      return !std::isspace(ch);
    });

   str.erase(iter.base(), str.end());
  }

  inline void lower(std::string &text) {
    auto [begin, end] = ptyps::iter::ranges(text);

    std::transform(begin, end, begin, [] (char next) {
      return std::tolower(next);
    });
  }

  inline void upper(std::string &text) {
    auto [begin, end] = ptyps::iter::ranges(text);

    std::transform(begin, end, begin, [] (char next) {
      return std::toupper(next);
    });
  }
}