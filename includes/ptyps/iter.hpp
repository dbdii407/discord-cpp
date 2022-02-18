#pragma once

#include <sys/types.h>
#include <utility>

namespace ptyps::iter {
  template <template <typename ...> typename C, typename ...A, typename I = typename C<A ...>::iterator>
    I begin(C<A ...> &cont, uint offset = 0) {
      return std::begin(cont) + offset;
    }

  template <template <typename ...> typename C, typename ...A, typename I = typename C<A ...>::iterator>
    I end(C<A ...> &cont, uint offset = 0) {
      return std::end(cont) - offset;
    }

  template <template <typename ...> typename C, typename ...A, typename I = typename C<A ...>::iterator>
    std::pair<I, I> ranges(C<A ...> &cont, uint start = 0, uint stop = 0) {
      return {
        begin(cont, start),
        end(cont, stop)
      };
    }
}