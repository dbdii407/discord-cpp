#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include "./string.hpp"

#include <exception>

namespace ptyps::err {
  struct exception : public std::exception {
    private:
      std::string message;

    public:
      template <typename ...A>
        exception(std::string_view str) : message(str) {
        }

      const char* what() const noexcept override {
        return message.data();
      }
  };
}