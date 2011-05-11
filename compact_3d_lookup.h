#include <cassert>
#include <stdint.h>
#include <algorithm>
#include <vector>
#include <iostream>

// If the template argument does not match
// any specialized version, recursively fall back to the 
// previous one.
template <unsigned int NUM_BITS>
struct storage_traits
{
  typedef typename storage_traits<NUM_BITS - 1>::type type;
};

template <>
struct storage_traits<0>
{
  typedef unsigned char type;
};

template <>
struct storage_traits<9>
{
  typedef unsigned short type;
};

template <>
struct storage_traits<17>
{
  typedef unsigned int type;
};

template <>
struct storage_traits<33>
{
  typedef uint64_t type;
};

template <>
struct storage_traits<65>
{
};

template <class T, unsigned int W_BITS, unsigned int H_BITS, unsigned int D_BITS, unsigned int MAX_ELEMENTS_BITS>
class compact_3d_lookup
{
public:
  typedef T value_type;
  typedef typename storage_traits<W_BITS>::type w_type;
  typedef typename storage_traits<H_BITS>::type h_type;
  typedef typename storage_traits<D_BITS>::type d_type;
private:
  typedef typename storage_traits<W_BITS + H_BITS + D_BITS + MAX_ELEMENTS_BITS>::type index_key_type;

  /** We store both the spatial position and the index in the storage vector
      in the same item. The spatial position gets the high bits, so that we 
      can order them, while the index gets the lower bits, so that we only 
      need one and operation to reference items in the array.
    */
  struct index_item
  {
    index_item(w_type x, h_type y, d_type z, size_t i)
      : key_((x << H_BITS+D_BITS + MAX_ELEMENTS_BITS) 
             + (y << D_BITS + MAX_ELEMENTS_BITS) 
             + (z << MAX_ELEMENTS_BITS) + i)
    {}

    // spatial position is in the top part of the key, so we can simply order by it.
    bool operator<(const index_item &b) const
    {
      return this->key_ < b.key_;
    }
    
    // Mask out the spatial position to get the index in the storage vector.
    size_t index()
    {
      return key_ & (1 << MAX_ELEMENTS_BITS) - 1;
    }
    
    // Shift out the index to get the compact spatial position.
    index_key_type compact_position()
    {
      return key_ >> MAX_ELEMENTS_BITS;
    }

    index_key_type key_;
  };
  typedef std::vector<index_item> index_type;
  typedef std::vector<value_type> storage_type;
public:
  compact_3d_lookup()
    : sorted_(false)
  {}
  // Insert v at position x,y,z.
  void push_back(const value_type &v, w_type x, h_type y, d_type z)
  {
    assert(!sorted_);
    index_.push_back(index_item(x, y, z, data_.size()));
    data_.push_back(v);
  }

  void do_index()
  {
    assert(!sorted_);
    std::sort(index_.begin(), index_.end());
    sorted_ = true;
  }

  typename storage_type::const_iterator find(w_type x, h_type y, d_type z)
  {
    assert(sorted_);
    index_item tmp(x,y,z, 0);
    typename index_type::iterator i = std::lower_bound(index_.begin(), index_.end(), tmp);
    // If the keys are not the same it means that the point is not in the 
    // lookup.
    if (i == index_.end() || i->compact_position() != tmp.compact_position())
      return data_.end();
    else
      return data_.begin() + i->index();
  }
  
  const value_type &operator()(w_type x, h_type y, d_type z)
  {
    return *(this->find(x,y,z));
  }

  typename storage_type::const_iterator end()
  {
    return data_.end();
  } 

  typename storage_type::const_iterator begin()
  {
    return data_.begin();
  }
private:
  index_type    index_;
  storage_type  data_;
  bool          sorted_;
};
