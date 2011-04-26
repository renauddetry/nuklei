// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_LINEAR_ALGEBRA_DEF_H
#define NUKLEI_LINEAR_ALGEBRA_DEF_H

#include <nuklei/Wm5/Wm5Vector2.h>
#include <nuklei/Wm5/Wm5Vector3.h>
#include <nuklei/Wm5/Wm5GVector.h>
#include <nuklei/Wm5/Wm5Matrix3.h>
#include <nuklei/Wm5/Wm5GMatrix.h>
#include <nuklei/Wm5/Wm5Plane3.h>
#include <nuklei/Wm5/Wm5Quaternion.h>

namespace nuklei {

  typedef nuklei_wmf::Vector2<coord_t> Vector2;
  typedef nuklei_wmf::Vector3<coord_t> Vector3;
  typedef nuklei_wmf::GVector<coord_t> GVector;
  typedef nuklei_wmf::Quaternion<coord_t> Quaternion;
  typedef nuklei_wmf::Matrix3<coord_t> Matrix3;
  typedef nuklei_wmf::GMatrix<coord_t> GMatrix;
  typedef nuklei_wmf::Plane3<coord_t> Plane3;

  typedef Vector3 Direction;
  typedef Vector3 Location;
  typedef Vector3 Translation;

  typedef Quaternion Rotation;
  typedef Rotation Orientation;

}

#endif
