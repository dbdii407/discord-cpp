#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include "../thread.hpp"
#include "./net.hpp"
#include "./ssl.hpp"

#include <fcntl.h>

namespace p$web::tcps {
  using exception = p$err::exception;

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

        p$web::net::set_type(ai, p$web::net::type::STREAM);
        p$web::net::set_proto(ai, p$web::net::proto::TCP);
      }

      bool connected() {
        return linked;
      }

      void write(std::string text) {
        if (!linked)
          throw exception("cannot write to closed socket");

        p$web::ssl::send(*sid, text);
      }

      void connect(uint16_t port, std::string addr) {
        auto lookup = p$web::net::lookup(addr);

        if (!lookup.size())
          throw exception("unable to resolve address");

        p$web::net::set_addr(ai, lookup.at(0));
        p$web::net::set_port(ai, port);

        id = p$web::net::open(ai);

        if (!id)
          throw exception("unable to open socket");

        auto i = p$web::net::connect(*id, ai);

        if (i == p$web::net::status::FAIL)
          throw exception("unable to connect socket");

        sid = p$web::ssl::open(*id);

        if (!sid)
          throw exception("unable to open ssl");

        auto s = p$web::ssl::connect(*sid);

        if (s == p$web::ssl::status::FAIL)
          throw exception("unable to connect ssl socket");

        // set the socket as non-blocking
        fcntl(*id, F_SETFL, O_NONBLOCK);

        linked = !0;

        tcp_on_connect();

        using pws$e = p$web::ssl::event;

        p$thread::loop_run([&]() -> bool {
          if (!linked)
            return !1;

          auto vari = p$web::ssl::recv(*sid);

          if (std::holds_alternative<p$web::ssl::event>(vari)) {
            auto event = std::get<p$web::ssl::event>(vari);

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
          p$time::wait(timeout);
      }
  };
}
