#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include <cstdint>
#include <random>
#include <string>

namespace ptyps::random {
  static auto engine = std::default_random_engine(std::random_device{}()); 

  inline uint number(uint start, uint end) {
    auto distro = std::uniform_int_distribution<uint>(start, end);
    return distro(engine);
  }

  inline std::string string(uint length, std::vector<char> charlist) {
    auto out = std::string();

    for (auto i = 0; i < length; i++) {
      auto position = number(0, charlist.size() - 1);
      auto next = charlist.at(position);

      out.push_back(next);
    }

    return out;
  }
}