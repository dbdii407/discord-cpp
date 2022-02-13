#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include "../thread.hpp"
#include "./net.hpp"
#include "./ssl.hpp"

#include <fcntl.h>

namespace ptyps::web::tcps {
  using exception = ptyps::err::exception;

  class Socket {
    private:
      std::optional<SSL*> sid;
      std::optional<int> id;
      addrinfo* ai;
      bool linked;

    public:
      virtual void tcp_on_recvd(std::string recvd) { }
      virtual void tcp_on_connect() { }
      virtual void tcp_on_disconnect() { }

      // -----

      Socket() {
        ai = new addrinfo();
        linked = !1;

        ptyps::web::net::set_type(ai, ptyps::web::net::type::STREAM);
        ptyps::web::net::set_proto(ai, ptyps::web::net::proto::TCP);
      }

      bool connected() {
        return linked;
      }

      void write(std::string text) {
        if (!linked)
          throw exception("cannot write to closed socket");

        ptyps::web::ssl::send(*sid, text);
      }

      void connect(uint16_t port, std::string addr) {
        auto lookup = ptyps::web::net::lookup(addr);

        if (!lookup.size())
          throw exception("unable to resolve address");

        ptyps::web::net::set_addr(ai, lookup.at(0));
        ptyps::web::net::set_port(ai, port);

        id = ptyps::web::net::open(ai);

        if (!id)
          throw exception("unable to open socket");

        auto i = ptyps::web::net::connect(*id, ai);

        if (i == ptyps::web::net::status::FAIL)
          throw exception("unable to connect socket");

        sid = ptyps::web::ssl::open(*id);

        if (!sid)
          throw exception("unable to open ssl");

        auto s = ptyps::web::ssl::connect(*sid);

        if (s == ptyps::web::ssl::status::FAIL)
          throw exception("unable to connect ssl socket");

        // set the socket as non-blocking
        fcntl(*id, F_SETFL, O_NONBLOCK);

        linked = !0;

        tcp_on_connect();

        using pws$e = ptyps::web::ssl::event;

        ptyps::thread::loop_run([&]() -> bool {
          if (!linked)
            return !1;

          auto vari = ptyps::web::ssl::recv(*sid);

          if (std::holds_alternative<ptyps::web::ssl::event>(vari)) {
            auto event = std::get<ptyps::web::ssl::event>(vari);

            if (event == pws$e::DISCONNECTED || event == pws$e::ERROR) {
              linked = !1;
              tcp_on_disconnect();
              return !0;
            }

            if (event == pws$e::WAITING)
              return !1;
          }

          auto recvd = std::get<std::string>(vari);

          tcp_on_recvd(recvd);

          return !1;
        });
      }
  
      // waits until the connection is disconnected
      void loop(uint seconds = 1) {
        auto timeout = std::chrono::seconds(seconds);

        while (linked)
          ptyps::time::wait(timeout);
      }
  };
}
