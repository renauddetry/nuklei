// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_TCLAP_WRAPPERS_H
#define NUKLEI_TCLAP_WRAPPERS_H

#include "tclap/ValueArg.h"

namespace TCLAP
{

#ifdef NICE_BUT_RISKY
  template<class T>
    class ConstrainedValueArg : std::vector<T>, ValuesConstraint<T>,
    public ValueArg<T>
    {
    public:
      ConstrainedValueArg( const std::string& flag,
                           const std::string& name,
                           const std::string& desc,
                           bool req,
                           T value,
                           std::vector<T> allowed,
                           CmdLineInterface& parser,
                           Visitor* v = NULL ) :
        std::vector<T>(allowed),
        // Not 
        ValuesConstraint<T>(*dynamic_cast<std::vector<T>* >(this)),
        ValueArg<T>(flag, name, desc, req, value,
                    dynamic_cast< ValuesConstraint<T>* >(this), parser, v)
        {}
    };
#else
  template<class T>
    class ConstrainedValueArg
    {
    public:
      ConstrainedValueArg( const std::string& flag,
                           const std::string& name,
                           const std::string& desc,
                           bool req,
                           T value,
                           std::vector<T> allowed,
                           CmdLineInterface& parser,
                           Visitor* v = NULL ) :
        _allowed(allowed), _vc(_allowed),
        _arg(flag, name, desc, req, value, &_vc, parser, v)
        {}

      T& getValue() { return _arg.getValue(); }
    private:
      std::vector<T> _allowed;
      ValuesConstraint<T> _vc;
      ValueArg<T> _arg;
    };
#endif
}
#endif
