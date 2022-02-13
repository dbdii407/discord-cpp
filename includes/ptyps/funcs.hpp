#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include "./details.hpp"

#include <optional>
#include <iterator>

namespace ptyps::funcs {
  template <template <typename ...> typename C, typename ...A, typename I = typename C<A ...>::iterator>
    std::pair<I, I> ranges(C<A ...> &cont, uint start = 0, uint stop = 0) {
      auto begin = std::begin(cont) + start;
      auto end = std::end(cont) - stop;

      return {
        begin,
        end
      };
    }

  template <template <typename ...> typename C, typename ...A, typename I = typename C<A ...>::iterator>
    I iterator(C<A ...> &cont, uint start = 0) {
      return std::begin(cont) + start;
    }

  // -----

  template <template <typename ...> typename C, typename ...A, typename T>
    void append(C<A ...> &cont, T &it, uint start = 0, uint stop = 0) {
      auto [begin, end] = ranges(it, start, stop);
      auto bi = std::back_inserter(cont);

      std::copy(begin, end, bi);
    }

  // ---- find

  template <template <typename ...> typename C, typename ...A, typename R = typename ptyps::details::iter<C<A ...>>::type>
    std::optional<R> find(C<A...> &cont, std::function<bool(typename ptyps::details::iter<C<A...>>::type)> func) {
      auto [begin, end] = ranges(cont);

      for (auto iter = begin; iter != end; ++iter) {
        if (!func(*iter))
          continue;

        return *iter;
      }

      return {};
    }

  template <template <typename ...> typename C, typename ...A, typename R = typename ptyps::details::iter<C<A ...>>::type>
    bool find(C<A...> &cont, typename ptyps::details::iter<C<A...>>::type it) {
      auto [begin, end] = ranges(cont);

      for (auto iter = begin; iter != end; ++iter) {
        if (*iter == it)
          return !0;
      }

      return !1;
    }

  // ---- pop

  template <template <typename ...> typename C, typename ...A, typename R = typename ptyps::details::iter<C<A ...>>::type>
    std::optional<R> pop(C<A...> &cont) {
      if (cont.size() == 0)
        return {};

      auto out = cont.front();

      cont.pop_front();
      
      return out;
    }

  // ---- each

  template <template <typename ...> typename C, typename ...A>
    void each(C<A...> cont, std::function<void(typename ptyps::details::iter<C<A ...>>::type)> func) {
      auto [begin, end] = ranges(cont);

      for (auto iter = begin; iter != end; ++iter)
        func(*iter);
    }

  template <template <typename ...> typename C, typename ...A>
    void each(C<A...> cont, std::function<void(typename ptyps::details::iter<C<A ...>>::type, int)> func) {
      auto [begin, end] = ranges(cont);

      for (auto iter = begin; iter != end; ++iter) {
        auto i = std::distance(begin, iter);
        func(*iter, i);
      }
    }

  // ---- includes

  template <template <typename ...> typename C, typename ...A, typename R = typename ptyps::details::iter<C<A ...>>::type>
    bool includes(C<A...> &cont, std::function<bool(typename ptyps::details::iter<C<A...>>::type)> func) {
      auto [begin, end] = ranges(cont);

      for (auto iter = begin; iter != end; ++iter) {
        if (!func(*iter))
          continue;

        return !0;
      }

      return !1;
    }

  template <template <typename ...> typename C, typename ...A, typename R = typename ptyps::details::iter<C<A ...>>::type>
    bool includes(C<A...> &cont, typename ptyps::details::iter<C<A...>>::type to) {
      auto [begin, end] = ranges(cont);

      for (auto iter = begin; iter != end; ++iter) {
        if (*iter != to)
          continue;

        return !0;
      }

      return !1;
    }

  // ---- remove

  template <template <typename ...> typename C, typename ...A>
    void remove(C<A...> &cont, typename ptyps::details::iter<C<A...>>::type find) {
      auto [begin, end] = ranges(cont);

      for (auto iter = begin; iter != end; ++iter) {
        if (*iter != find)
          continue;

        iter = --cont.erase(iter);
      }
    }

  template <template <typename ...> typename C, typename ...A>
    void remove(C<A...> &cont, std::function<bool(typename ptyps::details::iter<C<A...>>::type)> func) {
      auto [begin, end] = ranges(cont);

      for (auto iter = begin; iter != end; ++iter) {
        if (!func(*iter))
          continue;

        iter = --cont.erase(iter);
      }
    }

  // ---- erase (take away everything inside of range)

  template <template <typename ...> typename C, typename ...A>
    void erase(C<A ...> &cont, uint start = 0, uint stop = 0) {
      auto [begin, end] = ranges(cont, start, stop);

      cont.erase(begin, end);
    }

  // ---- chop (take away everything outside of range)

  template <template <typename ...> typename C, typename ...A>
    void chop(C<A ...> &cont, uint start = 0, uint stop = 0) {
      auto [begin, end] = ranges(cont, start, stop);

      cont = C<A...>(begin, end);
    }

  // ---- to (convert one thing into another)

  template <template <typename ...> typename C, typename ...A, template <typename ...> typename XC, typename ...XA>
    C<A...> to(XC<XA ...> &it, uint start = 0, uint stop = 0) {
      auto [begin, end] = ranges(it, start, stop);
      return C<A ...>(begin, end);
    }
}