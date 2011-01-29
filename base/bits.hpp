#pragma once
#include "base.hpp"
#include "../std/type_traits.hpp"

namespace bits
{
  // Count the number of 1 bits. Implementation: see Hacker's delight book.
  inline uint32_t popcount(uint32_t x)
  {
    x -= ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x += (x >> 8);
    x += (x >> 16);
    return x & 0x3F;
  }

  // Count the number of 1 bits in array p, length n bits.
  // There is a better implementation at hackersdelight.org
  inline uint32_t popcount(uint32_t const * p, uint32_t n)
  {
    uint32_t s = 0;
    for (uint32_t i = 0; i < n; i += 31)
    {
      uint32_t lim = (n < i + 31 ? n : i + 31);
      uint32_t s8 = 0;
      uint32_t x = 0;
      for (uint32_t j = i; j < lim; ++j)
      {
        x = p[j];
        x -= ((x >> 1) & 0x55555555);
        x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
        x = (x + (x >> 4)) & 0x0F0F0F0F;
        s8 += x;
      }
      x = (s8 & 0x00FF00FF) + ((s8 >> 8) & 0x00FF00FF);
      x = (x & 0x0000ffff) + (x >> 16);
      s += x;
    }
    return s;
  }

  static const int SELECT1_ERROR = -1;

  template <typename T> unsigned int select1(T x, unsigned int i)
  {
    // TODO: Fast implementation of select1.
    ASSERT(i > 0 && i <= sizeof(T) * 8, (i));
    for (unsigned int j = 0; j < sizeof(T) * 8; x >>= 1, ++j)
      if (x & 1)
        if (--i == 0)
          return j;
    return SELECT1_ERROR;
  }

  // Will be implemented when needed.
  uint64_t popcount(uint64_t x);
  // Will be implemented when needed.
  uint64_t popcount(uint64_t const * p, uint64_t n);

  template <typename T> T RoundLastBitsUpAndShiftRight(T x, T bits)
  {
    return (x & ((1 << bits) - 1)) ? (x >> bits) + 1 : (x >> bits);
  }

  template <typename T> struct LogBitSizeOfType;
  template <> struct LogBitSizeOfType<uint8_t>  { enum { value = 3 }; };
  template <> struct LogBitSizeOfType<uint16_t> { enum { value = 4 }; };
  template <> struct LogBitSizeOfType<uint32_t> { enum { value = 5 }; };
  template <> struct LogBitSizeOfType<uint64_t> { enum { value = 6 }; };

  template <typename T> T ROL(T x)
  {
    return (x << 1) | (x >> (sizeof(T) * 8 - 1));
  }

  template <typename T> inline typename make_unsigned<T>::type ZigZagEncode(T x)
  {
    STATIC_ASSERT(is_signed<T>::value);
    return (x << 1) ^ (x >> (sizeof(x) * 8 - 1));
  }

  template <typename T> inline typename make_signed<T>::type ZigZagDecode(T x)
  {
    STATIC_ASSERT(is_unsigned<T>::value);
    return (x >> 1) ^ -static_cast<typename make_signed<T>::type>(x & 1);
  }

  inline uint32_t PerfectShuffle(uint32_t x)
  {
    x = ((x & 0x0000FF00) << 8) | ((x >> 8) & 0x0000FF00) | (x & 0xFF0000FF);
    x = ((x & 0x00F000F0) << 4) | ((x >> 4) & 0x00F000F0) | (x & 0xF00FF00F);
    x = ((x & 0x0C0C0C0C) << 2) | ((x >> 2) & 0x0C0C0C0C) | (x & 0xC3C3C3C3);
    x = ((x & 0x22222222) << 1) | ((x >> 1) & 0x22222222) | (x & 0x99999999);
    return x;
  }

  inline uint32_t PerfectUnshuffle(uint32_t x)
  {
    x = ((x & 0x22222222) << 1) | ((x >> 1) & 0x22222222) | (x & 0x99999999);
    x = ((x & 0x0C0C0C0C) << 2) | ((x >> 2) & 0x0C0C0C0C) | (x & 0xC3C3C3C3);
    x = ((x & 0x00F000F0) << 4) | ((x >> 4) & 0x00F000F0) | (x & 0xF00FF00F);
    x = ((x & 0x0000FF00) << 8) | ((x >> 8) & 0x0000FF00) | (x & 0xFF0000FF);
    return x;
  }

  inline uint64_t BitwiseMerge(uint32_t x, uint32_t y)
  {
    uint32_t const hi = PerfectShuffle((y & 0xFFFF0000) | (x >> 16));
    uint32_t const lo = PerfectShuffle(((y & 0xFFFF) << 16 ) | (x & 0xFFFF));
    return (static_cast<uint64_t>(hi) << 32) + lo;
  }

  inline void BitwiseSplit(uint64_t v, uint32_t & x, uint32_t & y)
  {
    uint32_t const hi = bits::PerfectUnshuffle(static_cast<uint32_t>(v >> 32));
    uint32_t const lo = bits::PerfectUnshuffle(static_cast<uint32_t>(v & 0xFFFFFFFFULL));
    x = ((hi & 0xFFFF) << 16) | (lo & 0xFFFF);
    y =     (hi & 0xFFFF0000) | (lo >> 16);
  }
}
