#pragma once

#include "../base/assert.hpp"
#include "../base/base.hpp"
#include "../base/math.hpp"
#include "../base/stl_add.hpp"

#include "../std/iterator.hpp"


template <typename IsVisibleF>
bool FindSingleStripForIndex(size_t i, size_t n, IsVisibleF isVisible)
{
  // Searching for a strip only in a single direction, because the opposite direction
  // is traversed from the last vertex of the possible strip.
  size_t a = my::PrevModN(i, n);
  size_t b = my::NextModN(i, n);
  for (size_t j = 2; j < n; ++j)
  {
    ASSERT_NOT_EQUAL ( a, b, () );
    if (!isVisible(a, b))
      return false;
    if (j & 1)
      a = my::PrevModN(a, n);
    else
      b = my::NextModN(b, n);
  }

  ASSERT_EQUAL ( a, b, () );
  return true;
}

// If polygon with n vertices is a single strip, return the start index of the strip or n otherwise.
template <typename IsVisibleF>
size_t FindSingleStrip(size_t n, IsVisibleF isVisible)
{
  for (size_t i = 0; i < n; ++i)
  {
    if (FindSingleStripForIndex(i, n, isVisible))
      return i;
  }

  return n;
}

#ifdef DEBUG
template <typename IterT> bool TestPolygonPreconditions(IterT beg, IterT end)
{
  ASSERT_GREATER ( distance(beg, end), 2, () );
  ASSERT ( !AlmostEqual(*beg, *(--end)), () );
  return true;
}
#endif

/// Is polygon [beg, end) has CCW orientation.
template <typename IterT> bool IsPolygonCCW(IterT beg, IterT end)
{
  ASSERT ( TestPolygonPreconditions(beg, end), () );

  double minY = numeric_limits<double>::max();
  IterT iRes;
  for (IterT i = beg; i != end; ++i)
  {
    if ((*i).y < minY || ((*i).y == minY && (*i).x < (*iRes).x))
    {
      iRes = i;
      minY = (*i).y;
    }
  }

  return CrossProduct(*iRes - *PrevIterInCycle(iRes, beg, end),
                      *NextIterInCycle(iRes, beg, end) - *iRes) > 0;
}

/// Is segment (v, v1) in cone (vPrev, v, vNext)?
/// @precondition Orientation CCW!!!
template <typename PointT> bool IsSegmentInCone(PointT v, PointT v1, PointT vPrev, PointT vNext)
{
  PointT const diff = v1 - v;
  PointT const edgeL = vPrev - v;
  PointT const edgeR = vNext - v;
  double const cpLR = CrossProduct(edgeR, edgeL);
  //ASSERT(!my::AlmostEqual(cpLR, 0.0),
  ASSERT_NOT_EQUAL(cpLR, 0.0,
         ("vPrev, v, vNext shouldn't be collinear!", edgeL, edgeR, v, v1, vPrev, vNext));
  if (cpLR > 0)
  {
    // vertex is convex
    return CrossProduct(diff, edgeR) < 0 && CrossProduct(diff, edgeL) > 0;
  }
  else
  {
    // vertex is reflex
    return CrossProduct(diff, edgeR) < 0 || CrossProduct(diff, edgeL) > 0;
  }
}

/// Is diagonal (i0, i1) visible in polygon [beg, end).
/// @precondition Orientation CCW!!
template <typename IterT>
bool IsDiagonalVisible(IterT beg, IterT end, IterT i0, IterT i1)
{
  ASSERT ( IsPolygonCCW(beg, end), () );
  ASSERT ( TestPolygonPreconditions(beg, end), () );
  ASSERT ( i0 != i1, () );

  IterT const prev = PrevIterInCycle(i0, beg, end);
  IterT const next = NextIterInCycle(i0, beg, end);
  if (prev == i1 || next == i1)
    return true;

  if (!IsSegmentInCone(*i0, *i1, *prev, *next))
    return false;

  for (IterT j0 = beg, j1 = PrevIterInCycle(beg, beg, end); j0 != end; j1 = j0++)
    if (j0 != i0 && j0 != i1 && j1 != i0 && j1 != i1 && SegmentsIntersect(*i0, *i1, *j0, *j1))
      return false;

  return true;
}

template <typename IterT> class IsDiagonalVisibleFunctor
{
  IterT m_Beg, m_End;
public:
  IsDiagonalVisibleFunctor(IterT beg, IterT end) : m_Beg(beg), m_End(end) {}

  bool operator () (size_t a, size_t b) const
  {
    return IsDiagonalVisible(m_Beg, m_End, m_Beg + a, m_Beg + b);
  }
};

namespace detail
{
  template <typename F> class StripEmitter
  {
    F & m_f;
    int m_order;

  public:
    StripEmitter(F & f) : m_f(f), m_order(0) {}

    bool operator () (size_t a, size_t b)
    {
      if (m_order == 0)
      {
        m_f(b);
        m_f(a);
        m_order = 1;
      }
      else
      {
        m_f(m_order == 1 ? b : a);
        m_order = -m_order;
      }
      return true;
    }
  };
}

/// Make single strip for the range of points [beg, end), started with index = i.
template <typename F> 
void MakeSingleStripFromIndex(size_t i, size_t n, F f)
{
  ASSERT_LESS ( i, n, () );
  f(i);
  FindSingleStripForIndex(i, n, detail::StripEmitter<F>(f));
}
