#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include "../error.hpp"
#include "../funcs.hpp"

#include <arpa/inet.h>
#include <exception>
#include <unistd.h>
#include <variant>
#include <netdb.h>
#include <string>
#include <vector>
#include <list>

namespace p$web::net {
  using exception = p$err::exception;

  enum class status {
    FAIL = EOF,
    OK
  };

  enum class family {
    Unknown = EOF,
    Unspecified,
    IPv6,
    IPv4
  };

  enum class proto {
    Unknown = EOF,
    IP,
    TCP,
    UDP
  };

  enum class type {
    Unknown = EOF,
    STREAM,
    RAW
  };

  // ----

  // determines whether or not an IP address is valid IPv6
  bool isIPv6(std::string_view ip) {
    auto sa = sockaddr_in6();
    auto i = ::inet_pton(AF_INET6, &ip[0], &(sa.sin6_addr));
    return (i == 1) ? !0 : !1;
  }

  // determines whether or not an IP address is valid IPv4
  bool isIPv4(std::string_view ip) {
    auto sa = sockaddr_in();
    auto i = ::inet_pton(AF_INET, &ip[0], &(sa.sin_addr));
    return (i == 1) ? !0 : !1;
  }

  // ----

  // sets family, used by set_addr.
  void set_family(addrinfo* &ai, family f) {
    switch (f) {
      case family::IPv6:
        ai->ai_family = AF_INET6;
        break;

      case family::IPv4:
        ai->ai_family = AF_INET;
        break;

      case family::Unknown:
        ai->ai_family = AF_UNSPEC;
        break;

      default:
        throw exception("unable to set family");
    }
  }

  family get_family(addrinfo* ai) {
    if (ai->ai_family == AF_INET6)
      return family::IPv6;

    else if (ai->ai_family == AF_INET)
      return family::IPv4;

    else 
      return family::Unknown;
  }

  family get_family(std::string_view addr) {
    if (isIPv6(addr))
      return family::IPv6;

    if (isIPv4(addr))
      return family::IPv4;

    return family::Unknown;
  }

  // ----
  
  void set_proto(addrinfo* &ai, proto p) { 
    switch (p) {
      case proto::IP:
        ai->ai_protocol = IPPROTO_IP;
        break;

      case proto::TCP:
        ai->ai_protocol = IPPROTO_TCP;
        break;

      case proto::UDP:
        ai->ai_protocol = IPPROTO_UDP;
        break;

      default:
        throw exception("unable to set protocol");
    }
  }
  
  proto get_proto(addrinfo* ai) {
    if (ai->ai_protocol == IPPROTO_IP)
      return proto::IP;

    if (ai->ai_protocol == IPPROTO_TCP)
      return proto::TCP;

    if (ai->ai_protocol == IPPROTO_UDP)
      return proto::UDP;

    return proto::Unknown;
  }

  // ----
  
  void set_type(addrinfo* &ai, type t) {
    switch (t) {
      case type::STREAM:
        ai->ai_socktype = SOCK_STREAM;
        break;

      case type::RAW:
        ai->ai_socktype = SOCK_RAW;
        break;

      default:
        throw exception("unable to set type");
    }
  }

  type get_type(addrinfo* ai) {
    if (ai->ai_socktype == SOCK_STREAM)
      return type::STREAM;

    if (ai->ai_socktype == SOCK_RAW)
      return type::RAW;

    return type::Unknown;
  }

  // ----

  void set_types(addrinfo* &ai, std::initializer_list<type> list) {
    uint types;

    for (auto t : list) {
      switch (t) {
        case type::STREAM:
          types |= SOCK_STREAM;
          break;

        case type::RAW:
          types |= SOCK_RAW;
          break;

        default:
          throw exception("unable to set type");
      }
    }

    ai->ai_socktype = types;
  }

  std::list<type> get_types(addrinfo* ai) {
    std::list<type> out;
    
    if (ai->ai_socktype && SOCK_STREAM)
      out.push_back(type::STREAM);

    if (ai->ai_socktype && SOCK_RAW)
      out.push_back(type::RAW);

    return out;
  }

  // ----

  void set_addr(addrinfo* &ai, std::string_view addr) {
    set_family(ai, get_family(addr));

    switch (ai->ai_family) {
      case AF_INET6: {
        auto sa = new sockaddr_in6();
        
        auto i = inet_pton(AF_INET6, &addr[0], &sa->sin6_addr);

        if (i == EOF)
          throw exception("unable to set ipv6 addr");

        ai->ai_addrlen = sizeof(sockaddr_in6);
        ai->ai_addr = (sockaddr *) sa;

        break;
      }

      case AF_INET: {
        auto sa = new sockaddr_in();
        
        auto i = inet_pton(AF_INET, &addr[0], &sa->sin_addr);

        if (i == EOF)
          throw exception("unable to set ipv4 addr");

        ai->ai_addrlen = sizeof(sockaddr_in);
        ai->ai_addr = (sockaddr *) sa;

        break;
      }

      default:
        throw exception("unable to set addr");
    }
  }

  std::string get_addr(addrinfo* ai) {
    std::string out;

    switch (ai->ai_family) {
      case AF_INET6: {
        auto ia = ((sockaddr_in6 *) ai->ai_addr)->sin6_addr;
        inet_ntop(AF_INET6, &ia, &out[0], INET6_ADDRSTRLEN);
        break;
      }

      case AF_INET: {
        auto ia = ((sockaddr_in *) ai->ai_addr)->sin_addr;
        out = (std::string) inet_ntoa(ia);
        break;
      }

      default:
        throw exception("addrinfo* has invalid family");
    }

    return out.data();
  }

  // ----

  uint16_t get_port(addrinfo* ai) {
    uint out = 0;

    switch (ai->ai_family) {
      case AF_INET6:
        out = ((sockaddr_in6 *) ai->ai_addr)->sin6_port;
        break;

      case AF_INET:
        out = ((sockaddr_in *) ai->ai_addr)->sin_port;
        break;

      default:
        throw exception("addrinfo* has invalid family");
    }

    return ntohs(out);
  }

  // assign port
  void set_port(addrinfo* &ai, uint16_t p) {
    switch (ai->ai_family) {
      case AF_INET6:
        ((sockaddr_in6 *) ai->ai_addr)->sin6_port = htons(p);
        break;

      case AF_INET:
        ((sockaddr_in *) ai->ai_addr)->sin_port = htons(p);
        break;

      default:
        throw exception("unable to set port, no recognized family");
    }
  }

  // ----

  // return a list of IPv4 and/or IPv6 addresses
  std::vector<std::string> lookup(std::string_view host, family f = family::Unspecified) {
    std::vector<std::string> out;
    addrinfo* ai;

    getaddrinfo(&host[0], NULL, NULL, &ai);

    for (auto next = ai; next != NULL; next = next->ai_next) {
      auto fam = get_family(next);
      auto it = get_addr(next);

      if (f == family::Unspecified || fam == f) {
        auto exists = p$funcs::find(out, it);

        if (!exists)
          out.push_back(it);
      }
    }

    ::freeaddrinfo(ai);

    return out;
  }

  std::vector<std::string> lookupIPv6(std::string_view host) {
    return lookup(host, family::IPv6);
  }

  std::vector<std::string> lookupIPv4(std::string_view host) {
    return lookup(host, family::IPv4);
  }

  std::optional<uint> open(addrinfo* ai) {
    auto i = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

    if (i == EOF)
      return {};

    return i;
  }

  status connect(uint id, addrinfo* &ai) {
    switch(ai->ai_family) {
      case AF_INET6:
        ((sockaddr_in6 *) ai->ai_addr)->sin6_family = AF_INET6;
        break;

      case AF_INET:
        ((sockaddr_in *) ai->ai_addr)->sin_family = AF_INET;
        break;
    }

    auto i = ::connect(id, ai->ai_addr, ai->ai_addrlen);

    return (i != EOF) ?
      status::OK : status::FAIL;
  }

  status close(uint id) {
    return ::close(id) != EOF ?
      status::FAIL : status::OK;
  }

  enum class event {
    ERROR = EOF,
    WAITING,
    DISCONNECTED
  };

  std::variant<event, std::string> recv(uint id, size_t size = 1024) {
    std::vector<char> buffer(size);
    std::string recvd;
    int len = 0;

    while (!0) {
      auto i = (int) ::read(id, &buffer[0], size);

      if (i == EOF) {
        if (errno == EAGAIN)
          return event::WAITING;

        return event::ERROR;
      }

      if (i == 0)
        return event::DISCONNECTED;

      auto begin = p$funcs::iterator(buffer, 0);
      auto end = p$funcs::iterator(buffer, i);

      recvd += std::string(begin, end);
      buffer.clear();
      len += i;

      if (i < size)
        return recvd;
    }
  }

  template <typename ...A>
    status send(uint id, std::string text) {
      while(text.size()) {
        auto len = ::send(id, &text[0], text.size());

        if (err == EOF)
          return status::FAIL;

        auto begin = std::begin(text);

        text.erase(begin, begin + len);
      }

      return status::OK;
    }
}