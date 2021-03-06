// Copyright 2015-2017 Hans Dembinski
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_HISTOGRAM_SERIALIZATION_HPP
#define BOOST_HISTOGRAM_SERIALIZATION_HPP

#include <boost/container/string.hpp>
#include <boost/container/vector.hpp>
#include <boost/histogram/accumulators/mean.hpp>
#include <boost/histogram/accumulators/sum.hpp>
#include <boost/histogram/accumulators/weighted_mean.hpp>
#include <boost/histogram/accumulators/weighted_sum.hpp>
#include <boost/histogram/adaptive_storage.hpp>
#include <boost/histogram/axis/base.hpp>
#include <boost/histogram/axis/category.hpp>
#include <boost/histogram/axis/circular.hpp>
#include <boost/histogram/axis/integer.hpp>
#include <boost/histogram/axis/regular.hpp>
#include <boost/histogram/axis/variable.hpp>
#include <boost/histogram/axis/variant.hpp>
#include <boost/histogram/detail/buffer.hpp>
#include <boost/histogram/histogram.hpp>
#include <boost/histogram/storage_adaptor.hpp>
#include <boost/mp11/tuple.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <tuple>
#include <type_traits>

/** \file boost/histogram/serialization.hpp
 *  \brief Defines the serialization functions, to use with boost.serialize.
 *
 */

namespace std {
template <class Archive, typename... Ts>
void serialize(Archive& ar, tuple<Ts...>& t, unsigned /* version */) {
  ::boost::mp11::tuple_for_each(t, [&ar](auto& x) { ar& x; });
}
} // namespace std

namespace boost {
namespace container {
template <class Archive, class T, class A>
void serialize(Archive& ar, vector<T, A>& v, unsigned) {
  std::size_t size = v.size();
  ar& size;
  if (Archive::is_loading::value) { v.resize(size); }
  if (std::is_trivially_copyable<T>::value) {
    ar& ::boost::serialization::make_array(v.data(), size);
  } else {
    for (auto&& x : v) ar& x;
  }
}

template <class Archive, class C, class T, class A>
void serialize(Archive& ar, basic_string<C, T, A>& v, unsigned) {
  std::size_t size = v.size();
  ar& size;
  if (Archive::is_loading::value) v.resize(size);
  if (std::is_trivially_copyable<T>::value) {
    ar& ::boost::serialization::make_array(v.data(), size);
  } else {
    for (auto&& x : v) ar& x;
  }
}
} // namespace container

namespace histogram {

namespace accumulators {
template <typename RealType>
template <class Archive>
void sum<RealType>::serialize(Archive& ar, unsigned /* version */) {
  ar& sum_;
  ar& cor_;
}

template <typename RealType>
template <class Archive>
void weighted_sum<RealType>::serialize(Archive& ar, unsigned /* version */) {
  ar& sum_;
  ar& sum2_;
}

template <typename RealType>
template <class Archive>
void mean<RealType>::serialize(Archive& ar, unsigned /* version */) {
  ar& sum_;
  ar& mean_;
  ar& dsum2_;
}

template <typename RealType>
template <class Archive>
void weighted_mean<RealType>::serialize(Archive& ar, unsigned /* version */) {
  ar& sum_;
  ar& sum2_;
  ar& mean_;
  ar& dsum2_;
}
} // namespace accumulators

template <class Archive, typename T>
void serialize(Archive& ar, storage_adaptor<T>& s, unsigned /* version */) {
  auto size = s.size();
  ar& size;
  if (Archive::is_loading::value) { s.reset(size); }
  ar& static_cast<T&>(s);
}

namespace detail {
struct serializer {
  template <typename T, typename Buffer, typename Archive>
  void operator()(T*, Buffer& b, Archive& ar) {
    if (Archive::is_loading::value) {
      // precondition: buffer is destroyed
      b.set(b.template create<T>());
    }
    ar& boost::serialization::make_array(reinterpret_cast<T*>(b.ptr), b.size);
  }

  template <typename Buffer, typename Archive>
  void operator()(void*, Buffer& b, Archive&) {
    if (Archive::is_loading::value) { b.ptr = nullptr; }
  }
};
} // namespace detail

template <class Archive, typename A>
void serialize(Archive& ar, adaptive_storage<A>& s, unsigned /* version */) {
  using S = adaptive_storage<A>;
  if (Archive::is_loading::value) { S::apply(typename S::destroyer(), s.buffer); }
  ar& s.buffer.type;
  ar& s.buffer.size;
  S::apply(detail::serializer(), s.buffer, ar);
}

template <typename A, typename S>
template <class Archive>
void histogram<A, S>::serialize(Archive& ar, unsigned /* version */) {
  ar& axes_;
  ar& storage_;
}

namespace axis {
template <class Archive>
void serialize(Archive&, null_type&, unsigned /* version */) {} // noop

template <typename M>
template <class Archive>
void base<M>::serialize(Archive& ar, unsigned /* version */) {
  ar& size_meta_.first();
  ar& size_meta_.second();
  ar& opt_;
}

template <typename T>
template <class Archive>
void transform::pow<T>::serialize(Archive& ar, unsigned /* version */) {
  ar& power;
}

template <typename T, typename M>
template <class Archive>
void regular<T, M>::serialize(Archive& ar, unsigned /* version */) {
  ar& static_cast<base_type&>(*this);
  ar& static_cast<transform_type&>(*this);
  ar& min_;
  ar& delta_;
}

template <typename R, typename M>
template <class Archive>
void circular<R, M>::serialize(Archive& ar, unsigned /* version */) {
  ar& static_cast<base_type&>(*this);
  ar& phase_;
  ar& delta_;
}

template <typename R, typename A, typename M>
template <class Archive>
void variable<R, A, M>::serialize(Archive& ar, unsigned /* version */) {
  // destroy must happen before base serialization with old size
  if (Archive::is_loading::value) detail::destroy_buffer(x_.second(), x_.first(), nx());
  ar& static_cast<base_type&>(*this);
  if (Archive::is_loading::value)
    x_.first() = boost::histogram::detail::create_buffer(x_.second(), nx());
  ar& boost::serialization::make_array(x_.first(), nx());
}

template <typename I, typename M>
template <class Archive>
void integer<I, M>::serialize(Archive& ar, unsigned /* version */) {
  ar& static_cast<base_type&>(*this);
  ar& min_;
}

template <typename V, typename A, typename M>
template <class Archive>
void category<V, A, M>::serialize(Archive& ar, unsigned /* version */) {
  // destroy must happen before base serialization with old size
  if (Archive::is_loading::value)
    detail::destroy_buffer(x_.second(), x_.first(), base_type::size());
  ar& static_cast<base_type&>(*this);
  if (Archive::is_loading::value)
    x_.first() = boost::histogram::detail::create_buffer(x_.second(), base_type::size());
  ar& boost::serialization::make_array(x_.first(), base_type::size());
}

template <typename... Ts>
template <class Archive>
void variant<Ts...>::serialize(Archive& ar, unsigned /* version */) {
  ar& static_cast<base_type&>(*this);
}
} // namespace axis

} // namespace histogram
} // namespace boost

#endif
