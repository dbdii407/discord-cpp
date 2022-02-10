#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include "../funcs.hpp"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <optional>
#include <variant>
#include <vector>

namespace p$web::ssl {
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

  enum class status {
    FAIL = EOF,
    OK
  };

  static bool initialized = !1;

  void init() {
    if (initialized)
      return;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    initialized = !0;
  }
  
  std::optional<SSL*> open(uint id) {
    init();

    auto method = TLS_client_method();
    auto context = SSL_CTX_new(method);
    auto ssl = SSL_new(context);

    if (!ssl)
      return {};

    auto i = ::SSL_set_fd(ssl, id);

    if (i == 0)
      return {};

    return ssl;
  }

  status connect(SSL* id) {
    auto i = ::SSL_connect(id);
    auto err = ::SSL_get_error(id, i);

    if (err == SSL_ERROR_NONE)
      return status::OK;

    return status::FAIL;
  }

  enum class event {
    ERROR = EOF,
    WAITING,
    DISCONNECTED
  };

  std::variant<event, std::string> recv(SSL* id, uint size = 1024) {
    std::vector<char> buffer(size);
    std::string recvd;
    int len = 0;

    while (!0) {
      auto i = ::SSL_read(id, &buffer[0], size);
      auto err = ::SSL_get_error(id, i);

      if (i == EOF) {
        if (err == SSL_ERROR_WANT_READ)
          return event::WAITING;

        if (err == SSL_ERROR_SYSCALL)
          return event::DISCONNECTED;

        return event::ERROR;
      }

      recvd += p$funcs::to<std::basic_string, char>(buffer);
      buffer.clear();
      len += i;

      if (i < size)
        return recvd.substr(0, len);
    }
  }

  status send(SSL* id, std::string data) {
    while (data.size()) {
      auto len = ::SSL_write(id, &data[0], data.size());
      auto err = ::SSL_get_error(id, len);

      if (err)
        return status::FAIL;

      auto begin = std::begin(data);

      data.erase(begin, begin + len);
    }

    return status::OK;
  }
}