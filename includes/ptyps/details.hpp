#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include <functional>

namespace ptyps::details {
  template <typename T>
    struct func : public func<decltype(&T::operator())> {

    };
    
  template <typename R, typename C, typename ...A>
    struct func<R(C::*)(A...) const> {
      enum { arity = sizeof...(A) };

      using type = std::function<R(A...)>;
      using returns = R;

      template <uint i>
        struct arg {
          typedef typename std::tuple_element<i, std::tuple<A ...>>::type type;
        };
    };

  template <typename T>
    struct iter : public iter<decltype(&T::operator())> {

    };
    
  template <template <typename ...> typename C, typename ...A>
    struct iter<C<A...>> {
      using type = typename std::iterator_traits<typename C<A...>::iterator>::value_type;

      template <uint i>
        struct arg {
          typedef typename std::tuple_element<i, std::tuple<A ...>>::type type;
        };
    };
}