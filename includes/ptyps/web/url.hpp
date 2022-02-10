#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include <curl/curl.h>
#include <exception>
#include <optional>
#include <list>
#include <map>

#include "../string.hpp"
#include "../error.hpp"

namespace p$web::url {
  using exception = p$err::exception;

  std::string escape(std::string url) {
    auto curl = curl_easy_init();

    if (!curl)
      throw exception("escape not able to init curl");

    auto output = curl_easy_escape(curl, &url[0], url.length());
    curl_easy_cleanup(curl);
    
    return output;
  }

  using query_map = std::map<std::string, std::string>;

  std::string queries(query_map map) {
    auto list = std::list<std::string>();

    for (auto [key, value] : map) {
      auto next = escape(key) + "=" + escape(value);
      list.push_back(next);
    }

    return "?" + p$string::join(list, "&");
  } 

  struct formation {
    public:
      std::string protocol;
      std::string host;
      query_map queries;
  };

  std::string format(std::string path, formation args) {
    auto href = args.protocol + "://";

    if (args.host.ends_with("/"))
      args.host = args.host.substr(0, args.host.length() - 1);

    href += args.host;

    if (path.starts_with("/"))
      path = path.substr(1);

    href += "/" + path;

    if (args.queries.size())
      href += queries(args.queries);

    return href;
  }

  struct parsed {
    public:
      std::string protocol;
      std::string host;
      uint16_t port;
      std::string path;
      query_map queries;
      std::string query;
  };

  parsed parse(std::string addr) {
    auto map = std::map<std::string, std::string>();

    auto protocol = addr.substr(0, addr.find_first_of("://"));

    addr = addr.substr(protocol.length() + 3);

    auto host = addr.substr(0, addr.find_first_of("/"));

    // at this point, path
    addr = addr.substr(host.length());

    auto i = host.find_first_of(":");

    auto port = uint16_t();

    if (i != EOF) {
      port = std::stoi(host.substr(0, i++));
      host = host.substr(i);
    }

    else {
      if (protocol == "wss" || protocol == "https")
        port = 443;

      if (protocol == "ws" || protocol == "http")
        port = 80;

      if (protocol == "ftps")
        port = 990;

      if (protocol == "ftp")
        port = 22;
    }

    auto pos = addr.find_first_of("?");

    if (pos == EOF)
      return {
        protocol: protocol,
        host: host,
        port: port,
        path: addr,
        queries: map,
        query: ""
      };
      
    auto ques = addr.substr(pos + 1);

    // now it's the path
    addr = addr.substr(0, pos);

    p$string::split(ques, "&", [&](auto que) {
      auto i = que.find_first_of("=");

      auto property = que.substr(0, i++);
      auto value = que.substr(i);

      auto pair = std::pair(property, value);

      map.insert(pair);
    });

    return {
      protocol: protocol,
      host: host,
      port: port,
      path: addr,
      queries: map,
      query: "?" + ques
    };
  }
}