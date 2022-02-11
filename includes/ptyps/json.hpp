#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include <boost/json/src.hpp>
#include <boost/json.hpp>
#include <list>

#include "./filesystem.hpp"
#include "./string.hpp"
#include "./funcs.hpp"

namespace p$json {
  using obj = boost::json::value;

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

  obj object(std::initializer_list<boost::json::value_ref> init) {
    return boost::json::value(init); 
  }

  obj parse(std::string_view str) {
    return boost::json::parse(&str[0]);
  }

  obj open(std::string_view file) {
    auto contents = p$filesystem::read(&file[0]);

    if (!contents)
      throw exception("unable to read json file");
      
    return parse(*contents);
  }

  std::string stringify(obj o) {
    return boost::json::serialize(o);
  }

  std::optional<obj> get(obj o, std::string_view prop) {
    try {
      return o.at(&prop[0]);
    }

    catch (const std::exception &err) {
      return {};
    }
  }

  template <typename T>
    std::optional<T> value(obj o, std::string_view prop) {
      auto list = std::list<std::string>();

      p$string::split(&prop[0], ".", [&](auto next) {
        list.push_back(next);
      });

      while (!0) {
        auto next = p$funcs::pop(list);

        if (!next)
          break;

        try {
          o = *get(o, &(*next)[0]);
        }

        catch (const std::exception &err) {
          return {};
        }
      }

      try {
        return boost::json::value_to<T>(o);
      }

      catch (const std::exception &err) {
        return {};
      }
    }

  template <typename T>
    void put(obj &o, std::string_view prop, T result) {
      o.as_object().emplace(&prop[0], result);
    }
}