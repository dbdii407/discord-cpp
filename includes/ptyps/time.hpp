#pragma once

#include <chrono>
#include <thread>

namespace ptyps::time {
  template <typename T, typename D>
    void wait(std::chrono::duration<T, D> time) {
      std::this_thread::sleep_for(time);
    }

  uint64_t milliseconds() {
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                .count();
  }
}