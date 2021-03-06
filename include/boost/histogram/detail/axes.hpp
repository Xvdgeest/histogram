// Copyright 2015-2017 Hans Dembinski
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_HISTOGRAM_DETAIL_AXES_HPP
#define BOOST_HISTOGRAM_DETAIL_AXES_HPP

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/histogram/axis/traits.hpp>
#include <boost/histogram/axis/variant.hpp>
#include <boost/histogram/detail/meta.hpp>
#include <boost/histogram/histogram_fwd.hpp>
#include <boost/histogram/weight.hpp>
#include <boost/mp11.hpp>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <vector>

namespace boost {
namespace histogram {
namespace detail {

template <int N, typename... Ts>
decltype(auto) axis_get(std::tuple<Ts...>& axes) {
  return std::get<N>(axes);
}

template <int N, typename... Ts>
decltype(auto) axis_get(const std::tuple<Ts...>& axes) {
  return std::get<N>(axes);
}

template <int N, typename T>
decltype(auto) axis_get(T& axes) {
  return axes[N];
}

template <int N, typename T>
decltype(auto) axis_get(const T& axes) {
  return axes[N];
}

template <typename... Ts>
decltype(auto) axis_get(std::tuple<Ts...>& axes, std::size_t i) {
  return mp11::mp_with_index<sizeof...(Ts)>(
      i, [&](auto I) { return axis::variant<Ts&...>(std::get<I>(axes)); });
}

template <typename... Ts>
decltype(auto) axis_get(const std::tuple<Ts...>& axes, std::size_t i) {
  return mp11::mp_with_index<sizeof...(Ts)>(
      i, [&](auto I) { return axis::variant<const Ts&...>(std::get<I>(axes)); });
}

template <typename T>
decltype(auto) axis_get(T& axes, std::size_t i) {
  return axes.at(i);
}

template <typename T>
decltype(auto) axis_get(const T& axes, std::size_t i) {
  return axes.at(i);
}

template <typename... Ts, typename... Us>
bool axes_equal(const std::tuple<Ts...>& t, const std::tuple<Us...>& u) {
  return static_if<std::is_same<mp11::mp_list<Ts...>, mp11::mp_list<Us...>>>(
      [](const auto& a, const auto& b) { return a == b; },
      [](const auto&, const auto&) { return false; }, t, u);
}

template <typename... Ts, typename U>
bool axes_equal(const std::tuple<Ts...>& t, const U& u) {
  if (sizeof...(Ts) != u.size()) return false;
  bool equal = true;
  mp11::mp_for_each<mp11::mp_iota_c<sizeof...(Ts)>>([&](auto I) {
    using T = mp11::mp_at<std::tuple<Ts...>, decltype(I)>;
    auto up = axis::get<T>(&u[I]);
    equal &= (up && std::get<I>(t) == *up);
  });
  return equal;
}

template <typename T, typename... Us>
bool axes_equal(const T& t, const std::tuple<Us...>& u) {
  return axes_equal(u, t);
}

template <typename T, typename U>
bool axes_equal(const T& t, const U& u) {
  if (t.size() != u.size()) return false;
  return std::equal(t.begin(), t.end(), u.begin());
}

template <typename... Ts, typename... Us>
void axes_assign(std::tuple<Ts...>& t, const std::tuple<Us...>& u) {
  static_if<std::is_same<mp11::mp_list<Ts...>, mp11::mp_list<Us...>>>(
      [](auto& a, const auto& b) { a = b; },
      [](auto&, const auto&) {
        throw std::invalid_argument("cannot assign axes, types do not match");
      },
      t, u);
}

template <typename... Ts, typename U>
void axes_assign(std::tuple<Ts...>& t, const U& u) {
  mp11::mp_for_each<mp11::mp_iota_c<sizeof...(Ts)>>([&](auto I) {
    using T = mp11::mp_at_c<std::tuple<Ts...>, I>;
    std::get<I>(t) = axis::get<T>(u[I]);
  });
}

template <typename T, typename... Us>
void axes_assign(T& t, const std::tuple<Us...>& u) {
  t.resize(sizeof...(Us));
  mp11::mp_for_each<mp11::mp_iota_c<sizeof...(Us)>>(
      [&](auto I) { t[I] = std::get<I>(u); });
}

template <typename T, typename U>
void axes_assign(T& t, const U& u) {
  t.assign(u.begin(), u.end());
}

template <typename T>
constexpr std::size_t axes_size(const T& axes) noexcept {
  return static_if<has_fixed_size<unqual<T>>>(
      [](const auto& a) {
        using U = unqual<decltype(a)>;
        return std::tuple_size<U>::value;
      },
      [&](const auto& a) { return a.size(); }, axes);
}

template <typename T>
void rank_check(const T& axes, const unsigned N) {
  BOOST_ASSERT_MSG(N < axes_size(axes), "index out of range");
}

template <typename F, typename... Ts>
void for_each_axis(const std::tuple<Ts...>& axes, F&& f) {
  mp11::tuple_for_each(axes, std::forward<F>(f));
}

template <typename F, typename T>
void for_each_axis(const T& axes, F&& f) {
  for (const auto& x : axes) { axis::visit(std::forward<F>(f), x); }
}

template <typename T>
auto make_empty_axes(const T& t) {
  auto r = T();
  static_if<is_vector_like<T>>([&](auto) { r.reserve(t.size()); }, [](auto) {}, 0);
  for_each_axis(t, [&r](const auto& a) {
    using U = unqual<decltype(a)>;
    r.emplace_back(U());
  });
  return r;
}

template <typename... Ts>
auto make_empty_axes(const std::tuple<Ts...>&) {
  return std::tuple<Ts...>();
}

template <typename T>
std::size_t bincount(const T& axes) {
  std::size_t n = 1;
  for_each_axis(axes, [&n](const auto& a) { n *= axis::traits::extend(a); });
  return n;
}

template <typename... Ns, typename... Ts>
auto make_sub_axes(const std::tuple<Ts...>& t, Ns... ns) {
  return std::make_tuple(std::get<ns>(t)...);
}

template <typename... Ns, typename... Ts>
auto make_sub_axes(const std::vector<Ts...>& t, Ns... ns) {
  return std::vector<Ts...>({t[ns]...}, t.get_allocator());
}

/// Index with an invalid state
struct optional_index {
  std::size_t idx = 0;
  std::size_t stride = 1;
  operator bool() const { return stride > 0; }
  std::size_t operator*() const { return idx; }
};

inline void linearize(optional_index& out, const int axis_size, const int axis_shape,
                      int j) noexcept {
  BOOST_ASSERT_MSG(out.stride == 0 || (-1 <= j && j <= axis_size),
                   "index must be in bounds for this algorithm");
  if (j < 0) j += (axis_size + 2); // wrap around if j < 0
  out.idx += j * out.stride;
  // set stride to 0, if j is invalid
  out.stride *= (j < axis_shape) * axis_shape;
}

template <typename... Ts, typename U>
void linearize1(optional_index& out, const axis::variant<Ts...>& axis, const U& u) {
  axis::visit([&](const auto& a) { linearize1(out, a, u); }, axis);
}

template <typename A, typename U>
void linearize1(optional_index& out, const A& axis, const U& u) {
  // protect against instantiation with wrong template argument
  using arg_t = arg_type<A>;
  static_if<std::is_convertible<U, arg_t>>(
      [&](const auto& u) {
        const auto a_size = axis.size();
        const auto a_shape = axis::traits::extend(axis);
        const auto j = axis(u);
        linearize(out, a_size, a_shape, j);
      },
      [&](const U&) {
        throw std::invalid_argument(
            detail::cat(boost::core::demangled_name(BOOST_CORE_TYPEID(A)),
                        ": cannot convert argument of type ",
                        boost::core::demangled_name(BOOST_CORE_TYPEID(U)), " to ",
                        boost::core::demangled_name(BOOST_CORE_TYPEID(arg_t))));
      },
      u);
}

template <typename T>
void linearize2(optional_index& out, const T& axis, const int j) {
  const auto a_size = static_cast<int>(axis.size());
  const auto a_shape = axis::traits::extend(axis);
  out.stride *= (-1 <= j && j <= a_size); // set stride to 0, if j is invalid
  linearize(out, a_size, a_shape, j);
}

// special case: histogram::operator(tuple(1, 2)) is called on 1d histogram with axis
// that accepts 2d tuple, this should work and not fail
// - solution is to forward tuples of size > 1 directly to axis for 1d histograms
// - has nice side-effect of making histogram::operator(1, 2) work as well
// - cannot detect call signature of axis at compile-time in all configurations
//   (axis::variant provides generic call interface and hides concrete interface),
//   so we throw at runtime if incompatible argument is passed (e.g. 3d tuple)
template <unsigned Offset, unsigned N, typename T, typename U>
optional_index args_to_index(const std::tuple<T>& axes, const U& args) {
  optional_index idx;
  if (N > 1) {
    linearize1(idx, std::get<0>(axes), sub_tuple<Offset, N>(args));
  } else {
    linearize1(idx, std::get<0>(axes), std::get<Offset>(args));
  }
  return idx;
}

template <unsigned Offset, unsigned N, typename T0, typename T1, typename... Ts,
          typename U>
optional_index args_to_index(const std::tuple<T0, T1, Ts...>& axes, const U& args) {
  static_assert(sizeof...(Ts) + 2 == N, "number of arguments != histogram rank");
  optional_index idx;
  mp11::mp_for_each<mp11::mp_iota_c<N>>(
      [&](auto I) { linearize1(idx, std::get<I>(axes), std::get<(Offset + I)>(args)); });
  return idx;
}

// overload for dynamic axes
template <unsigned Offset, unsigned N, typename T, typename U>
optional_index args_to_index(const T& axes, const U& args) {
  const unsigned m = axes.size();
  optional_index idx;
  if (m == 1 && N > 1)
    linearize1(idx, axes[0], sub_tuple<Offset, N>(args));
  else {
    if (m != N) throw std::invalid_argument("number of arguments != histogram rank");
    mp11::mp_for_each<mp11::mp_iota_c<N>>(
        [&](auto I) { linearize1(idx, axes[I], std::get<(Offset + I)>(args)); });
  }
  return idx;
}

template <typename U>
constexpr std::pair<int, int> weight_sample_indices() {
  if (is_weight<U>::value) return std::make_pair(0, -1);
  if (is_sample<U>::value) return std::make_pair(-1, 0);
  return std::make_pair(-1, -1);
}

template <typename U0, typename U1, typename... Us>
constexpr std::pair<int, int> weight_sample_indices() {
  using L = mp11::mp_list<U0, U1, Us...>;
  const int n = sizeof...(Us) + 1;
  if (is_weight<mp11::mp_at_c<L, 0>>::value) {
    if (is_sample<mp11::mp_at_c<L, 1>>::value) return std::make_pair(0, 1);
    if (is_sample<mp11::mp_at_c<L, n>>::value) return std::make_pair(0, n);
    return std::make_pair(0, -1);
  }
  if (is_sample<mp11::mp_at_c<L, 0>>::value) {
    if (is_weight<mp11::mp_at_c<L, 1>>::value) return std::make_pair(1, 0);
    if (is_weight<mp11::mp_at_c<L, n>>::value) return std::make_pair(n, 0);
    return std::make_pair(-1, 0);
  }
  if (is_weight<mp11::mp_at_c<L, n>>::value) {
    // 0, n already covered
    if (is_sample<mp11::mp_at_c<L, (n - 1)>>::value) return std::make_pair(n, n - 1);
    return std::make_pair(n, -1);
  }
  if (is_sample<mp11::mp_at_c<L, n>>::value) {
    // n, 0 already covered
    if (is_weight<mp11::mp_at_c<L, (n - 1)>>::value) return std::make_pair(n - 1, n);
    return std::make_pair(-1, n);
  }
  return std::make_pair(-1, -1);
}

template <typename S, typename T>
void fill_storage_impl(mp11::mp_int<-1>, mp11::mp_int<-1>, S& storage, std::size_t i,
                       const T&) {
  storage(i);
}

template <int Iw, typename S, typename T>
void fill_storage_impl(mp11::mp_int<Iw>, mp11::mp_int<-1>, S& storage, std::size_t i,
                       const T& args) {
  storage(i, std::get<Iw>(args));
}

template <int Is, typename S, typename T>
void fill_storage_impl(mp11::mp_int<-1>, mp11::mp_int<Is>, S& storage, std::size_t i,
                       const T& args) {
  mp11::tuple_apply([&](auto&&... sargs) { storage(i, sargs...); },
                    std::get<Is>(args).value);
}

template <int Iw, int Is, typename S, typename T>
void fill_storage_impl(mp11::mp_int<Iw>, mp11::mp_int<Is>, S& storage, std::size_t i,
                       const T& args) {
  mp11::tuple_apply([&](auto&&... sargs) { storage(i, std::get<Iw>(args), sargs...); },
                    std::get<Is>(args).value);
}

template <typename S, typename T, typename... Us>
void fill_impl(S& storage, const T& axes, const std::tuple<Us...>& args) {
  constexpr std::pair<int, int> iws = weight_sample_indices<Us...>();
  constexpr unsigned n = sizeof...(Us) - (iws.first > -1) - (iws.second > -1);
  constexpr unsigned offset = (iws.first == 0 || iws.second == 0)
                                  ? (iws.first == 1 || iws.second == 1 ? 2 : 1)
                                  : 0;
  optional_index idx = args_to_index<offset, n>(axes, args);
  if (idx) {
    fill_storage_impl(mp11::mp_int<iws.first>(), mp11::mp_int<iws.second>(), storage,
                      *idx, args);
  }
}

template <typename A, typename... Us>
optional_index at_impl(const A& axes, const std::tuple<Us...>& args) {
  if (axes_size(axes) != sizeof...(Us))
    throw std::invalid_argument("number of arguments != histogram rank");
  optional_index idx;
  mp11::mp_for_each<mp11::mp_iota_c<sizeof...(Us)>>([&](auto I) {
    linearize2(idx, axis_get<I>(axes), static_cast<int>(std::get<I>(args)));
  });
  return idx;
}

} // namespace detail
} // namespace histogram
} // namespace boost

#endif
