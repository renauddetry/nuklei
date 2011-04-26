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

  /**
   * @author Renaud Detry <detryr@montefiore.ulg.ac.be>
   */
  class Observation
    {
    public:
      typedef enum { COVIS3D, OSUTXT, PLY, RIF, CRD, SERIAL, BUILTINVTK, NUKLEI, EXR, UNKNOWN } Type;
      static const Type defaultType = COVIS3D;
      static const std::string TypeNames[];

      virtual ~Observation() {}
      virtual std::auto_ptr<kernel::base> getKernel() const = 0;
      virtual void setKernel(const kernel::base& k) = 0;
      virtual Type type() const = 0;
    };
}

#endif
