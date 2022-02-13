#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include "./time.hpp"

#include <functional>

namespace ptyps::thread {
  void run(std::function<bool()> func) {
    auto thread = std::thread(func);
    thread.detach();
  }

  // -----

  std::thread loop(std::function<bool()> func) {
    return std::thread([func]() {
      while (!0) {
        auto stop = func();

        if (stop == !0)
          break;
      }
    });
  }

  void loop_run(std::function<bool()> func) {
    auto thread = loop(func);
    thread.detach();
  }

  // -----

  template <typename T, typename D>
    std::thread interval(std::chrono::duration<T, D> time, std::function<bool()> func) {
      return std::thread([time, func]() {
        while (!0) {
          ptyps::time::wait(time);
          
          auto stop = func();

          if (stop == !0)
            break;
        }
      });
    }

  template <typename T, typename D>
    void interval_run(std::chrono::duration<T, D> time, std::function<bool()> func) {
      auto thread = interval(time, func);
      thread.detach();
    }
}