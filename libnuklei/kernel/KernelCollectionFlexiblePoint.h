// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_KERNEL_COLLECTION_FLEXIBLE_POINT_H
#define NUKLEI_KERNEL_COLLECTION_FLEXIBLE_POINT_H


#include <nuklei/Definitions.h>

namespace nuklei
{

  struct FlexiblePoint {
    // libkdtree++:
    typedef coord_t value_type;

    FlexiblePoint() : idx_(-1) { vec_[0]= vec_[1] = vec_[2] = 0; }
    FlexiblePoint (value_type x, value_type y, value_type z) : idx_(-1)
    { vec_[0]=x; vec_[1]=y; vec_[2]=z;  }
    FlexiblePoint (value_type x, value_type y, value_type z, int idx) : idx_(idx)
    { vec_[0]=x; vec_[1]=y; vec_[2]=z;  }

    // cgal:
    inline value_type x() const { return vec_[ 0 ]; }
    inline value_type y() const { return vec_[ 1 ]; }
    inline value_type z() const { return vec_[ 2 ]; }

    // cgal:
    inline value_type& x() { return vec_[ 0 ]; }
    inline value_type& y() { return vec_[ 1 ]; }
    inline value_type& z() { return vec_[ 2 ]; }
    
    // cgal:
    inline const value_type* vec() const { return vec_; }
    inline value_type* vec() { return vec_; }

    // libkdtree++:
    inline value_type operator[](const size_t i) const { return vec_[i]; }

    // cgal&libkdtree++:
    inline bool operator==(const FlexiblePoint& p) const
    {
      return (x() == p.x()) && (y() == p.y()) && (z() == p.z()) && (idx() == p.idx());
    }

    // cgal:
    inline bool operator!=(const FlexiblePoint& p) const
    { return ! (*this == p); }
    

    inline int idx() const { return idx_; }
    inline int& idx() { return idx_; }

    struct Construct_coord_iterator {
      const value_type* operator()(const FlexiblePoint& p) const
      { return static_cast<const value_type*>(p.vec()); }

      const value_type* operator()(const FlexiblePoint& p, int)  const
      { return static_cast<const value_type*>(p.vec()+3); }
    };
    
    struct Accessor
    {
      typedef value_type result_type;

      inline result_type
      operator()(const FlexiblePoint& p, const size_t i) const throw ()
      { return p[i]; }
    };

    struct Distance {
      typedef FlexiblePoint Query_item;

      value_type transformed_distance(const FlexiblePoint& p1,
                                      const FlexiblePoint& p2) const
      {
        value_type distx= p1.x()-p2.x();
        value_type disty= p1.y()-p2.y();
        value_type distz= p1.z()-p2.z();
        return distx*distx+disty*disty+distz*distz;
      }

#ifdef NUKLEI_USE_CGAL
      template <class TreeTraits>
      value_type min_distance_to_rectangle(const FlexiblePoint& p,
               const CGAL::Kd_tree_rectangle<TreeTraits>& b) const
      {
        value_type distance(0.0), h = p.x();
        if (h < b.min_coord(0)) distance += (b.min_coord(0)-h)*(b.min_coord(0)-h);
        if (h > b.max_coord(0)) distance += (h-b.max_coord(0))*(h-b.max_coord(0));
        h=p.y();
        if (h < b.min_coord(1)) distance += (b.min_coord(1)-h)*(b.min_coord(1)-h);
        if (h > b.max_coord(1)) distance += (h-b.max_coord(1))*(h-b.min_coord(1));
        h=p.z();
        if (h < b.min_coord(2)) distance += (b.min_coord(2)-h)*(b.min_coord(2)-h);
        if (h > b.max_coord(2)) distance += (h-b.max_coord(2))*(h-b.max_coord(2));
        return distance;
      }

      template <class TreeTraits>
      value_type max_distance_to_rectangle(const FlexiblePoint& p,
               const CGAL::Kd_tree_rectangle<TreeTraits>& b) const
      {
        value_type h = p.x();

        value_type d0 = (h >= (b.min_coord(0)+b.max_coord(0))/2.0) ?
          (h-b.min_coord(0))*(h-b.min_coord(0)) : (b.max_coord(0)-h)*(b.max_coord(0)-h);

        h=p.y();
        value_type d1 = (h >= (b.min_coord(1)+b.max_coord(1))/2.0) ?
          (h-b.min_coord(1))*(h-b.min_coord(1)) : (b.max_coord(1)-h)*(b.max_coord(1)-h);
        h=p.z();
        value_type d2 = (h >= (b.min_coord(2)+b.max_coord(2))/2.0) ?
          (h-b.min_coord(2))*(h-b.min_coord(2)) : (b.max_coord(2)-h)*(b.max_coord(2)-h);
        return d0 + d1 + d2;
      }
#endif
      
      value_type new_distance(value_type& dist, value_type old_off, value_type new_off,
                              int /* cutting_dimension */)  const
      {
        return dist + new_off*new_off - old_off*old_off;
      }

      value_type transformed_distance(value_type d) const { return d*d; }

      value_type inverse_of_transformed_distance(value_type d) { return std::sqrt(d); }

    }; // end of struct Distance

  private:
    value_type vec_[3];
    int idx_;
  };

  inline std::ostream& operator<<(std::ostream& out, const FlexiblePoint& p)
  {
    out << p.x() << " " << p.y() << " " << p.z() << " " << p.idx();
    return out;
  }

}


#endif
