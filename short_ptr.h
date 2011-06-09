#include <stdint.h>
#include <iostream>

class bad_segment
{
public:
  bad_segment(const void *ptr) : ptr_(ptr) {}
private:
  const void *ptr_;
};

class sptr_base
{
protected:
  static const uint32_t ALIGNMENT_BITS = 3;
  static const uint32_t ALIGNMENT = (1<<ALIGNMENT_BITS);
  static const uintptr_t ALIGNMENT_MASK = ALIGNMENT - 1;

protected:
  static uint64_t _seg_map[ALIGNMENT];
  static uint32_t _segs;
  
  inline static uint32_t encode(const void *in)
  {
    uint64_t ptr = (uint64_t)(in);
    uint64_t p = ptr & ~0xFFFFFFFFULL; // Keep only high part
    if (!p)              // If no high part, no need to worry the pointer is effectively 32 bit.
      return static_cast<uint32_t>(ptr);
    uint32_t s = _segs; // Find the range for the high part.
    uint32_t i = 0;      
    for (; i < s; ++i)
    {
      if (_seg_map[i] == p) // If we found a matching segment, we encode it's index in the LSB.
        return 0xFFFFFFFFULL & ptr | (i+1);
    } 
    // If we did not find it, we need to atomically insert it in the map.
    // FIXME: not multithread-safe
    if (_segs >= ALIGNMENT_MASK)
      throw bad_segment(in);
    _seg_map[_segs++] = p;
    return 0xFFFFFFFFULL & ptr | _segs;
  }
  
  inline static const void *decode(uint32_t id)
  {
    uint32_t segId = id & ALIGNMENT_MASK;
    if (!segId)
      return reinterpret_cast<const void *>(id);
    return reinterpret_cast<const void *>(_seg_map[segId-1] | (id & ~ALIGNMENT_MASK));
  }
};

uint64_t sptr_base::_seg_map[sptr_base::ALIGNMENT];
uint32_t sptr_base::_segs;

template <class T>
class sptr : public sptr_base
{
public:
  sptr(T const*ptr)
  : id_(encode(ptr))
  {}
  
  T &operator*()
  {
    return *reinterpret_cast<const T *>(decode(id_));
  }
  T const*operator->()
  {
    return reinterpret_cast<T const*>(decode(id_));
  }
private:
  uint32_t id_;
};
