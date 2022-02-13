#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include <filesystem>
#include <optional>
#include <unistd.h>
#include <string>

namespace ptyps::filesystem {
  std::optional<std::string> read(std::string_view file) {
    auto fp = fopen(&file[0], "r");
    auto out = std::string();

    while (!0) {
      auto next = fgetc(fp);

      if (next == EOF)
        break;

      out += next;
    }

    auto i = fclose(fp);

    if (i == EOF)
      return {};

    return out;
  }
}