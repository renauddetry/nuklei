// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_OBSERVATION_H
#define NUKLEI_OBSERVATION_H


#include <nuklei/Definitions.h>
#include <nuklei/Types.h>
#include <nuklei/Kernel.h>

namespace nuklei {

  class Observation
    {
    public:
      typedef enum { SERIAL, NUKLEI, COVIS3D, OSUTXT, PCD, PLY, RIF, CRD, BUILTINVTK, TXT, UNKNOWN } Type;
      static const Type defaultType = SERIAL;
      static const std::string TypeNames[];

      virtual ~Observation() {}
      virtual std::auto_ptr<kernel::base> getKernel() const = 0;
      virtual void setKernel(const kernel::base& k) = 0;
      virtual Type type() const = 0;
    };
}

#endif
