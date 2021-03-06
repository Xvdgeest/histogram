// Copyright 2015-2016 Hans Dembinski
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_HISTOGRAM_VALUE_ITERATOR_HPP
#define BOOST_HISTOGRAM_VALUE_ITERATOR_HPP

#include <boost/histogram/detail/index_cache.hpp>
#include <boost/histogram/unsafe_access.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/mp11.hpp>

namespace boost {
namespace histogram {

/// Fancy iterator over histogram bins with access multi-dimensional index.
template <typename Histogram>
class iterator
    : public iterator_facade<iterator<Histogram>, typename Histogram::value_type,
                             random_access_traversal_tag,
                             typename Histogram::const_reference> {
public:
  iterator(const Histogram& h, std::size_t idx) : histogram_(h), idx_(idx) {}

  iterator(const iterator& o) : histogram_(o.histogram_), idx_(o.idx_) {}

  iterator& operator=(const iterator& o) {
    histogram_ = o.histogram_;
    idx_ = o.idx_;
    cache_.reset();
  }

  std::size_t rank() const noexcept { return histogram_.rank(); }

  int idx(std::size_t dim = 0) const noexcept {
    if (!cache_) { cache_.set(histogram_); }
    cache_.set_idx(idx_);
    return cache_.get(dim);
  }

  decltype(auto) bin() const { return histogram_.axis()[idx()]; }

  template <std::size_t I>
  decltype(auto) bin(mp11::mp_size_t<I>) const {
    return histogram_.axis(mp11::mp_size_t<I>())[idx(I)];
  }

  decltype(auto) bin(std::size_t dim) const { return histogram_.axis(dim)[idx(dim)]; }

private:
  bool equal(const iterator& rhs) const noexcept {
    return &histogram_ == &rhs.histogram_ && idx_ == rhs.idx_;
  }

  void increment() noexcept { ++idx_; }
  void decrement() noexcept { --idx_; }

  void advance(int n) noexcept { idx_ += n; }

  decltype(auto) dereference() const noexcept {
    return unsafe_access::storage(histogram_)[idx_];
  }

  const Histogram& histogram_;
  std::size_t idx_;
  mutable detail::index_cache cache_;
  friend class ::boost::iterator_core_access;
};

} // namespace histogram
} // namespace boost

#endif
